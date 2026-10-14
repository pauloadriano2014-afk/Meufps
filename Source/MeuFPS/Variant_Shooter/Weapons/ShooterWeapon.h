// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeaponHolder.h"
#include "Animation/AnimInstance.h"
#include "ShooterWeapon.generated.h"

class IShooterWeaponHolder;
class AShooterProjectile;
class USkeletalMeshComponent;
class UAnimMontage;
class UAnimInstance;

/**
 *  Base class for a simple first person shooter weapon
 *  Provides both first person and third person perspective meshes
 *  Handles ammo and firing logic
 *  Interacts with the weapon owner through the ShooterWeaponHolder interface
 */
UCLASS(abstract)
class MEUFPS_API AShooterWeapon : public AActor
{
	GENERATED_BODY()
	
	/** First person perspective mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** Third person perspective mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ThirdPersonMesh;

protected:

	/** Cast pointer to the weapon owner */
	IShooterWeaponHolder* WeaponOwner;

	/** Type of projectiles this weapon will shoot */
	UPROPERTY(EditAnywhere, Category="Ammo")
	TSubclassOf<AShooterProjectile> ProjectileClass;

	/** Number of bullets in a magazine */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0, ClampMax = 100))
	int32 MagazineSize = 10;

	/** Number of bullets in the current magazine */
	int32 CurrentBullets = 0;

	/** Bullets carried outside the magazine. Only the player runs out, AI-held weapons reload for free */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0))
	int32 ReserveAmmo = 90;

	/** Max bullets that can be carried outside the magazine */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0))
	int32 MaxReserveAmmo = 180;

	/** Time it takes to reload the magazine */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float ReloadTime = 2.0f;

	/** If true, the player's weapon starts reloading by itself when the magazine runs empty. AI weapons always do */
	UPROPERTY(EditAnywhere, Category="Ammo")
	bool bAutoReload = true;

	/** If true, the weapon is currently reloading */
	bool bIsReloading = false;

	/** Timer that completes the reload */
	FTimerHandle ReloadTimer;

	/** If true, bullets hit instantly along the aim line instead of spawning a projectile */
	UPROPERTY(EditAnywhere, Category="Hitscan")
	bool bHitscan = false;

	/** Max distance a hitscan shot can reach */
	UPROPERTY(EditAnywhere, Category="Hitscan", meta = (ClampMin = 0, Units = "cm"))
	float HitscanRange = 20000.0f;

	/** Damage of a hitscan shot at point blank range */
	UPROPERTY(EditAnywhere, Category="Hitscan", meta = (ClampMin = 0))
	float HitscanDamage = 25.0f;

	/** Damage multiplier when the shot hits the head */
	UPROPERTY(EditAnywhere, Category="Hitscan", meta = (ClampMin = 1))
	float HeadshotMultiplier = 2.0f;

	/** Distance at which damage starts to fall off */
	UPROPERTY(EditAnywhere, Category="Hitscan", meta = (ClampMin = 0, Units = "cm"))
	float DamageFalloffStart = 2000.0f;

	/** Distance at which damage reaches its minimum */
	UPROPERTY(EditAnywhere, Category="Hitscan", meta = (ClampMin = 0, Units = "cm"))
	float DamageFalloffEnd = 8000.0f;

	/** Fraction of the damage that remains at or past the falloff end distance */
	UPROPERTY(EditAnywhere, Category="Hitscan", meta = (ClampMin = 0, ClampMax = 1))
	float MinDamageFraction = 0.4f;

	/** Cone half-angle of random spread applied to each hitscan shot */
	UPROPERTY(EditAnywhere, Category="Hitscan", meta = (ClampMin = 0, ClampMax = 45, Units = "Degrees"))
	float HitscanSpread = 0.5f;

	/** Impulse applied to physics objects hit by a hitscan shot */
	UPROPERTY(EditAnywhere, Category="Hitscan", meta = (ClampMin = 0))
	float HitscanPhysicsForce = 20000.0f;

	/** If true, draws a short tracer line and an impact point for each hitscan shot. Temporary until real effects exist */
	UPROPERTY(EditAnywhere, Category="Hitscan")
	bool bDebugDrawShots = true;

	/** Animation montage to play when firing this weapon */
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* FiringMontage;

	/** AnimInstance class to set for the first person character mesh when this weapon is active */
	UPROPERTY(EditAnywhere, Category="Animation")
	TSubclassOf<UAnimInstance> FirstPersonAnimInstanceClass;

	/** AnimInstance class to set for the third person character mesh when this weapon is active */
	UPROPERTY(EditAnywhere, Category="Animation")
	TSubclassOf<UAnimInstance> ThirdPersonAnimInstanceClass;

	/** Cone half-angle for variance while aiming */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 90, Units = "Degrees"))
	float AimVariance = 0.0f;

	/** Amount of firing recoil to apply to the owner */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 100))
	float FiringRecoil = 0.0f;

	/** Name of the first person muzzle socket where projectiles will spawn */
	UPROPERTY(EditAnywhere, Category="Aim")
	FName MuzzleSocketName;

	/** Distance ahead of the muzzle that bullets will spawn at */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float MuzzleOffset = 10.0f;

	/** If true, this weapon will automatically fire at the refire rate */
	UPROPERTY(EditAnywhere, Category="Refire")
	bool bFullAuto = false;

	/** Time between shots for this weapon. Affects both full auto and semi auto modes */
	UPROPERTY(EditAnywhere, Category="Refire", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float RefireRate = 0.5f;

	/** Game time of last shot fired, used to enforce refire rate on semi auto */
	float TimeOfLastShot = 0.0f;

	/** If true, the weapon is currently firing */
	bool bIsFiring = false;

	/** Timer to handle full auto refiring */
	FTimerHandle RefireTimer;

	/** Cast pawn pointer to the owner for AI perception system interactions */
	TObjectPtr<APawn> PawnOwner;

	/** Loudness of the shot for AI perception system interactions */
	UPROPERTY(EditAnywhere, Category="Perception", meta = (ClampMin = 0, ClampMax = 100))
	float ShotLoudness = 1.0f;

	/** Max range of shot AI perception noise */
	UPROPERTY(EditAnywhere, Category="Perception", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float ShotNoiseRange = 300.0f;

	/** Tag to apply to noise generated by shooting this weapon */
	UPROPERTY(EditAnywhere, Category="Perception")
	FName NoiseOwnerTag = FName("Shot");

public:	

	/** Constructor */
	AShooterWeapon();

protected:
	
	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay Cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

protected:

	/** Called when the weapon's owner is destroyed */
	UFUNCTION()
	void OnOwnerDestroyed(AActor* DestroyedActor);

public:

	/** Activates this weapon and gets it ready to fire */
	void ActivateWeapon(const FName& OwnerTag);

	/** Deactivates this weapon */
	void DeactivateWeapon();

	/** Start firing this weapon */
	void StartFiring();

	/** Stop firing this weapon */
	void StopFiring();

	/** Starts reloading the magazine from the reserve ammo */
	void StartReload();

	/** Adds bullets to the reserve, up to the max reserve */
	void AddReserveAmmo(int32 Amount);

protected:

	/** Fire the weapon */
	virtual void Fire();

	/** Called when the refire rate time has passed while shooting semi auto weapons */
	void FireCooldownExpired();

	/** Fire a projectile towards the target location, or a hitscan shot if bHitscan is set */
	virtual void FireProjectile(const FVector& TargetLocation);

	/** Fires an instant shot towards the target location and applies damage to whatever it hits */
	void FireHitscan(const FVector& TargetLocation);

	/** Returns true if the hit landed on the head of a character */
	bool IsHeadshot(const FHitResult& Hit) const;

	/** Called when the reload timer expires */
	void FinishReload();

	/** Aborts a reload in progress */
	void CancelReload();

	/** Returns true if reloading never consumes reserve ammo, which is the case for AI-controlled owners */
	bool HasInfiniteReserve() const;

	/** Returns true if an empty magazine should start a reload without the player asking for it */
	bool ShouldAutoReload() const;

	/** Pushes the current ammo counts to the owner's HUD */
	void RefreshHUD();

	/** Called when a hitscan shot hits something. Implement in Blueprint for impact effects */
	UFUNCTION(BlueprintImplementableEvent, Category="Weapon", meta = (DisplayName = "On Hitscan Impact"))
	void BP_OnHitscanImpact(const FHitResult& Hit, bool bHeadshot);

	/** Called when a reload starts. Implement in Blueprint for sounds and animations */
	UFUNCTION(BlueprintImplementableEvent, Category="Weapon", meta = (DisplayName = "On Reload Started"))
	void BP_OnReloadStarted();

	/** Called when a reload finishes */
	UFUNCTION(BlueprintImplementableEvent, Category="Weapon", meta = (DisplayName = "On Reload Finished"))
	void BP_OnReloadFinished();

	/** Calculates the spawn transform for projectiles shot by this weapon */
	FTransform CalculateProjectileSpawnTransform(const FVector& TargetLocation) const;

public:

	/** Returns the first person mesh */
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; };

	/** Returns the third person mesh */
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; };

	/** Returns the first person anim instance class */
	const TSubclassOf<UAnimInstance>& GetFirstPersonAnimInstanceClass() const;

	/** Returns the third person anim instance class */
	const TSubclassOf<UAnimInstance>& GetThirdPersonAnimInstanceClass() const;

	/** Returns the magazine size */
	int32 GetMagazineSize() const { return MagazineSize; };

	/** Returns the current bullet count */
	int32 GetBulletCount() const { return CurrentBullets; }

	/** Returns the bullets carried outside the magazine */
	int32 GetReserveAmmo() const { return ReserveAmmo; }

	/** Returns true while a reload is in progress */
	bool IsReloading() const { return bIsReloading; }
};
