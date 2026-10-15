// Copyright Epic Games, Inc. All Rights Reserved.


#include "InfectedCharacter.h"
#include "InfectedAIController.h"
#include "Variant_Shooter/ShooterGameMode.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AInfectedCharacter::AInfectedCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 96.0f);

	// use the template mannequin as a placeholder body
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimFinder(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -96.0f), FRotator(0.0f, -90.0f, 0.0f));

	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	if (AnimFinder.Succeeded())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(AnimFinder.Class);
	}

	// face the way it walks
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	// it is driven by its own AI controller, whether placed in the level or spawned
	AIControllerClass = AInfectedAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	Tags.Add(FName("Infected"));
}

void AInfectedCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AInfectedCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AInfectedCharacter::SetInfectedType(EInfectedType NewType)
{
	InfectedType = NewType;

	switch (NewType)
	{
	case EInfectedType::Walker:
		MaxHP = 80.0f;
		MoveSpeed = 130.0f;
		AttackDamage = 15.0f;
		AttackRange = 130.0f;
		AttackCooldown = 1.3f;
		bWanders = true;
		SetActorScale3D(FVector(1.0f));
		break;

	case EInfectedType::Runner:
		MaxHP = 40.0f;
		MoveSpeed = 480.0f;
		AttackDamage = 12.0f;
		AttackRange = 120.0f;
		AttackCooldown = 0.9f;
		bWanders = false;
		SetActorScale3D(FVector(0.95f));
		break;

	case EInfectedType::Brute:
		MaxHP = 450.0f;
		MoveSpeed = 270.0f;
		AttackDamage = 45.0f;
		AttackRange = 190.0f;
		AttackCooldown = 1.8f;
		AttackWindup = 0.6f;
		bWanders = false;
		SetActorScale3D(FVector(1.3f));
		break;
	}

	CurrentHP = MaxHP;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

float AInfectedCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead)
	{
		return 0.0f;
	}

	CurrentHP -= Damage;

	if (CurrentHP <= 0.0f)
	{
		Die();

	} else {

		// being shot tells the infected where the shooter is
		if (AInfectedAIController* AI = Cast<AInfectedAIController>(GetController()))
		{
			if (EventInstigator)
			{
				AI->AlertTo(EventInstigator->GetPawn());
			}
		}
	}

	return Damage;
}

bool AInfectedCharacter::TryAttack(AActor* Target)
{
	if (bIsDead || !Target)
	{
		return false;
	}

	const float Now = GetWorld()->GetTimeSeconds();

	if (Now - LastAttackTime < AttackCooldown)
	{
		return false;
	}

	LastAttackTime = Now;
	AttackTarget = Target;

	BP_OnAttack();

	// the damage lands after the windup, so the player has a moment to react
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AInfectedCharacter::DealAttackDamage, FMath::Max(AttackWindup, 0.01f), false);

	return true;
}

void AInfectedCharacter::DealAttackDamage()
{
	AActor* Target = AttackTarget.Get();

	if (bIsDead || !Target)
	{
		return;
	}

	// the hit misses if the target moved out of reach during the windup
	if (GetDistanceTo(Target) > AttackRange + 80.0f)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
}

void AInfectedCharacter::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	Tags.Add(DeathTag);

	GetWorldTimerManager().ClearTimer(AttackTimer);

	// announce the death so kill objectives and the score can count it
	if (AShooterGameMode* GM = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->IncrementTeamScore(TeamByte);
	}

	// stop moving and shut the AI down
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	if (AController* AI = GetController())
	{
		AI->UnPossess();
		AI->Destroy();
	}

	// fall as a ragdoll
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetPhysicsBlendWeight(1.0f);

	BP_OnDeath();

	SetLifeSpan(BodyLifeSpan);
}
