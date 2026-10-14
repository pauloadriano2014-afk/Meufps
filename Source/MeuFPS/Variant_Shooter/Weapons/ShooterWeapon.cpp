// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "ShooterProjectile.h"
#include "ShooterWeaponHolder.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

AShooterWeapon::AShooterWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	// create the root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// create the first person mesh
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(RootComponent);

	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
	FirstPersonMesh->bOnlyOwnerSee = true;

	// create the third person mesh
	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third Person Mesh"));
	ThirdPersonMesh->SetupAttachment(RootComponent);

	ThirdPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	ThirdPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);
	ThirdPersonMesh->bOwnerNoSee = true;
}

void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();

	// subscribe to the owner's destroyed delegate
	GetOwner()->OnDestroyed.AddDynamic(this, &AShooterWeapon::OnOwnerDestroyed);

	// cast the weapon owner
	WeaponOwner = Cast<IShooterWeaponHolder>(GetOwner());
	PawnOwner = Cast<APawn>(GetOwner());

	// fill the first ammo clip
	CurrentBullets = MagazineSize;

	// attach the meshes to the owner
	WeaponOwner->AttachWeaponMeshes(this);
}

void AShooterWeapon::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the refire and reload timers
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
}

void AShooterWeapon::OnOwnerDestroyed(AActor* DestroyedActor)
{
	// ensure this weapon is destroyed when the owner is destroyed
	Destroy();
}

void AShooterWeapon::ActivateWeapon(const FName& OwnerTag)
{
	// save the owner tag for perception noise detection
	NoiseOwnerTag = OwnerTag;

	// unhide this weapon
	SetActorHiddenInGame(false);

	// notify the owner
	WeaponOwner->OnWeaponActivated(this);

	// refresh the ammo readout for the newly active weapon
	RefreshHUD();
}

void AShooterWeapon::DeactivateWeapon()
{
	// ensure we're no longer firing this weapon while deactivated
	StopFiring();

	// switching away from the weapon cancels a reload in progress
	CancelReload();

	// hide the weapon
	SetActorHiddenInGame(true);

	// notify the owner
	WeaponOwner->OnWeaponDeactivated(this);
}

void AShooterWeapon::StartFiring()
{
	// raise the firing flag
	bIsFiring = true;

	// check how much time has passed since we last shot
	// this may be under the refire rate if the weapon shoots slow enough and the player is spamming the trigger
	const float TimeSinceLastShot = GetWorld()->GetTimeSeconds() - TimeOfLastShot;

	if (TimeSinceLastShot > RefireRate)
	{
		// fire the weapon right away
		Fire();

	} else {

		// if we're full auto, schedule the next shot
		if (bFullAuto)
		{
			GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, TimeSinceLastShot, false);
		}

	}
}

void AShooterWeapon::StopFiring()
{
	// lower the firing flag
	bIsFiring = false;

	// clear the refire timer
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
}

void AShooterWeapon::Fire()
{
	// ensure the player still wants to fire. They may have let go of the trigger
	if (!bIsFiring)
	{
		return;
	}

	// can't shoot while the magazine is being reloaded
	if (bIsReloading)
	{
		return;
	}

	// an empty magazine can't shoot. Reload if that's allowed to happen by itself
	if (CurrentBullets <= 0)
	{
		if (ShouldAutoReload())
		{
			StartReload();
		}

		return;
	}

	// fire a projectile at the target
	FireProjectile(WeaponOwner->GetWeaponTargetLocation());

	// update the time of our last shot
	TimeOfLastShot = GetWorld()->GetTimeSeconds();

	// make noise so the AI perception system can hear us
	MakeNoise(ShotLoudness, PawnOwner, PawnOwner->GetActorLocation(), ShotNoiseRange, NoiseOwnerTag);

	// are we full auto?
	if (bFullAuto)
	{
		// schedule the next shot
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, RefireRate, false);
	} else {

		// for semi-auto weapons, schedule the cooldown notification
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::FireCooldownExpired, RefireRate, false);

	}
}

void AShooterWeapon::FireCooldownExpired()
{
	// notify the owner
	WeaponOwner->OnSemiWeaponRefire();
}

void AShooterWeapon::FireProjectile(const FVector& TargetLocation)
{
	if (bHitscan)
	{
		// instant shot, no projectile actor
		FireHitscan(TargetLocation);

	} else {

		// get the projectile transform
		FTransform ProjectileTransform = CalculateProjectileSpawnTransform(TargetLocation);

		// spawn the projectile
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = PawnOwner;

		AShooterProjectile* Projectile = GetWorld()->SpawnActor<AShooterProjectile>(ProjectileClass, ProjectileTransform, SpawnParams);

		// set the noise tag on the projectile
		if (Projectile)
		{
			Projectile->SetNoiseTag(NoiseOwnerTag);
		}
	}

	// play the firing montage
	WeaponOwner->PlayFiringMontage(FiringMontage);

	// add recoil
	WeaponOwner->AddWeaponRecoil(FiringRecoil);

	// consume bullets
	--CurrentBullets;

	// update the weapon HUD
	RefreshHUD();

	// if the magazine is depleted, start reloading right away
	if (CurrentBullets <= 0 && ShouldAutoReload())
	{
		StartReload();
	}
}

void AShooterWeapon::FireHitscan(const FVector& TargetLocation)
{
	// shoot from the muzzle towards the aim target, with random spread inside a cone
	const FVector MuzzleLoc = FirstPersonMesh->GetSocketLocation(MuzzleSocketName);
	const FVector AimDir = (TargetLocation - MuzzleLoc).GetSafeNormal();
	const float Spread = bIsAiming ? HitscanSpread * AimSpreadMultiplier : HitscanSpread;
	const FVector ShotDir = FMath::VRandCone(AimDir, FMath::DegreesToRadians(Spread));
	const FVector TraceEnd = MuzzleLoc + (ShotDir * HitscanRange);

	// never hit the shooter or the weapon itself
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, MuzzleLoc, TraceEnd, ECC_Visibility, QueryParams);
	const FVector ImpactPoint = bHit ? Hit.ImpactPoint : TraceEnd;

#if ENABLE_DRAW_DEBUG
	if (bDebugDrawShots)
	{
		DrawDebugLine(GetWorld(), MuzzleLoc, ImpactPoint, FColor::Yellow, false, 0.1f, 0, 1.0f);

		if (bHit)
		{
			DrawDebugPoint(GetWorld(), ImpactPoint, 10.0f, FColor::Red, false, 1.0f);
		}
	}
#endif

	if (!bHit)
	{
		return;
	}

	const bool bHeadshot = IsHeadshot(Hit);

	if (AActor* HitActor = Hit.GetActor())
	{
		// damage falls off linearly between the start and end distances
		float Damage = HitscanDamage;

		if (DamageFalloffEnd > DamageFalloffStart)
		{
			const float FalloffAlpha = FMath::Clamp((Hit.Distance - DamageFalloffStart) / (DamageFalloffEnd - DamageFalloffStart), 0.0f, 1.0f);
			Damage *= FMath::Lerp(1.0f, MinDamageFraction, FalloffAlpha);
		}

		if (bHeadshot)
		{
			Damage *= HeadshotMultiplier;
		}

		UGameplayStatics::ApplyPointDamage(HitActor, Damage, ShotDir, Hit, PawnOwner->GetController(), this, UDamageType::StaticClass());

		// tell the owner it hit a character, so the HUD can show a hit marker
		if (HitActor->IsA<ACharacter>())
		{
			WeaponOwner->OnHitConfirmed(bHeadshot);
		}
	}

	// push physics objects
	if (UPrimitiveComponent* HitComp = Hit.GetComponent())
	{
		if (HitComp->IsSimulatingPhysics())
		{
			HitComp->AddImpulseAtLocation(ShotDir * HitscanPhysicsForce, Hit.ImpactPoint);
		}
	}

	// let Blueprints add impact effects
	BP_OnHitscanImpact(Hit, bHeadshot);
}

bool AShooterWeapon::IsHeadshot(const FHitResult& Hit) const
{
	// hits on a character mesh report the bone that was struck
	if (Hit.BoneName != NAME_None)
	{
		return Hit.BoneName.ToString().Contains(TEXT("head"));
	}

	// no bone info: fall back to the top fifth of the character's capsule
	if (const ACharacter* HitCharacter = Cast<ACharacter>(Hit.GetActor()))
	{
		const float HeadHeight = HitCharacter->GetActorLocation().Z + (HitCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.6f);
		return Hit.ImpactPoint.Z >= HeadHeight;
	}

	return false;
}

void AShooterWeapon::StartReload()
{
	// nothing to do if we're already reloading or the magazine is full
	if (bIsReloading || CurrentBullets >= MagazineSize)
	{
		return;
	}

	// we need bullets in reserve to reload
	if (ReserveAmmo <= 0 && !HasInfiniteReserve())
	{
		return;
	}

	bIsReloading = true;

	// show the reloading state in the HUD readout
	RefreshHUD();

	// cancel any shot that was scheduled
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);

	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &AShooterWeapon::FinishReload, ReloadTime, false);

	// let Blueprints play sounds and animations
	BP_OnReloadStarted();
}

void AShooterWeapon::FinishReload()
{
	bIsReloading = false;

	const int32 BulletsNeeded = MagazineSize - CurrentBullets;

	if (HasInfiniteReserve())
	{
		CurrentBullets = MagazineSize;

	} else {

		// move bullets from the reserve into the magazine
		const int32 BulletsLoaded = FMath::Min(BulletsNeeded, ReserveAmmo);
		CurrentBullets += BulletsLoaded;
		ReserveAmmo -= BulletsLoaded;
	}

	RefreshHUD();

	BP_OnReloadFinished();

	// full auto weapons resume shooting if the trigger is still held
	if (bIsFiring && bFullAuto)
	{
		StartFiring();
	}
}

void AShooterWeapon::CancelReload()
{
	bIsReloading = false;
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
}

void AShooterWeapon::AddReserveAmmo(int32 Amount)
{
	ReserveAmmo = FMath::Clamp(ReserveAmmo + Amount, 0, MaxReserveAmmo);

	RefreshHUD();
}

bool AShooterWeapon::HasInfiniteReserve() const
{
	// only player-controlled owners have limited ammo
	return PawnOwner && !PawnOwner->IsPlayerControlled();
}

bool AShooterWeapon::ShouldAutoReload() const
{
	return bAutoReload || HasInfiniteReserve();
}

void AShooterWeapon::RefreshHUD()
{
	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
}

FTransform AShooterWeapon::CalculateProjectileSpawnTransform(const FVector& TargetLocation) const
{
	// find the muzzle location
	const FVector MuzzleLoc = FirstPersonMesh->GetSocketLocation(MuzzleSocketName);

	// calculate the spawn location ahead of the muzzle
	const FVector SpawnLoc = MuzzleLoc + ((TargetLocation - MuzzleLoc).GetSafeNormal() * MuzzleOffset);

	// find the aim rotation vector while applying some variance to the target 
	const FRotator AimRot = UKismetMathLibrary::FindLookAtRotation(SpawnLoc, TargetLocation + (UKismetMathLibrary::RandomUnitVector() * AimVariance));

	// return the built transform
	return FTransform(AimRot, SpawnLoc, FVector::OneVector);
}

const TSubclassOf<UAnimInstance>& AShooterWeapon::GetFirstPersonAnimInstanceClass() const
{
	return FirstPersonAnimInstanceClass;
}

const TSubclassOf<UAnimInstance>& AShooterWeapon::GetThirdPersonAnimInstanceClass() const
{
	return ThirdPersonAnimInstanceClass;
}
