// Copyright Epic Games, Inc. All Rights Reserved.


#include "InfectedAIController.h"
#include "InfectedCharacter.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

AInfectedAIController::AInfectedAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// create the perception component and make it the controller's
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerception);

	// sight: a cone in front of the infected
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1800.0f;
	SightConfig->LoseSightRadius = 2200.0f;
	SightConfig->PeripheralVisionAngleDegrees = 80.0f;
	SightConfig->SetMaxAge(4.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	// hearing: gunshots and other noise
	UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 4000.0f;
	HearingConfig->SetMaxAge(3.0f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->ConfigureSense(*HearingConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());

	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AInfectedAIController::OnTargetPerceptionUpdated);
}

void AInfectedAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	HomeLocation = InPawn->GetActorLocation();

	GoIdle();
}

void AInfectedAIController::AlertTo(AActor* Actor)
{
	if (Actor && IsPlayerPawn(Actor))
	{
		StartChase(Actor);
	}
}

void AInfectedAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	const AInfectedCharacter* Infected = Cast<AInfectedCharacter>(GetPawn());
	if (!Actor || !Infected || Infected->IsDead() || !Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		// seeing the player starts the chase
		if (IsPlayerPawn(Actor))
		{
			StartChase(Actor);
		}

	} else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>()) {

		// noise sends the infected to check it out, unless it is already hunting
		if (State != EInfectedAIState::Chase)
		{
			Investigate(Stimulus.StimulusLocation);
		}
	}
}

void AInfectedAIController::StartChase(AActor* Actor)
{
	State = EInfectedAIState::Chase;
	Target = Actor;
	TimeSinceTargetSeen = 0.0f;
	RepathTimer = 0.0f;
	SightCheckTimer = 0.0f;
}

void AInfectedAIController::Investigate(const FVector& Location)
{
	State = EInfectedAIState::Investigate;
	InvestigateLocation = Location;
	WaitTimer = InvestigateWaitTime;

	MoveToLocation(Location, 120.0f, true, true, true, false, nullptr, true);
}

void AInfectedAIController::GoIdle()
{
	State = EInfectedAIState::Idle;
	Target = nullptr;
	StopMovement();

	WanderTimer = FMath::FRandRange(4.0f, 10.0f);
}

bool AInfectedAIController::IsPlayerPawn(const AActor* Actor) const
{
	const APawn* Pawn = Cast<APawn>(Actor);
	return Pawn && Pawn->IsPlayerControlled();
}

void AInfectedAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AInfectedCharacter* Infected = Cast<AInfectedCharacter>(GetPawn());
	if (!Infected || Infected->IsDead())
	{
		return;
	}

	switch (State)
	{
	case EInfectedAIState::Idle:

		// wander slowly around the spawn point
		if (Infected->ShouldWander())
		{
			WanderTimer -= DeltaSeconds;

			if (WanderTimer <= 0.0f)
			{
				const FVector Offset(FMath::FRandRange(-WanderRadius, WanderRadius), FMath::FRandRange(-WanderRadius, WanderRadius), 0.0f);
				MoveToLocation(HomeLocation + Offset, 60.0f, true, true, true, false, nullptr, true);

				WanderTimer = FMath::FRandRange(6.0f, 14.0f);
			}
		}
		break;

	case EInfectedAIState::Investigate:

		// once there, look around for a while and then give up
		if (GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			WaitTimer -= DeltaSeconds;

			if (WaitTimer <= 0.0f)
			{
				GoIdle();
			}
		}
		break;

	case EInfectedAIState::Chase:
	{
		AActor* TargetActor = Target.Get();

		// the target is gone or dead
		if (!TargetActor || TargetActor->ActorHasTag(FName("Dead")))
		{
			GoIdle();
			break;
		}

		// keep track of how long the target has been out of sight
		TimeSinceTargetSeen += DeltaSeconds;
		SightCheckTimer -= DeltaSeconds;

		if (SightCheckTimer <= 0.0f)
		{
			SightCheckTimer = 0.3f;

			if (LineOfSightTo(TargetActor))
			{
				TimeSinceTargetSeen = 0.0f;
			}
		}

		if (TimeSinceTargetSeen > LoseTargetTime)
		{
			// lost it. Check the last known position
			Investigate(TargetActor->GetActorLocation());
			break;
		}

		if (Infected->GetDistanceTo(TargetActor) <= Infected->GetAttackRange())
		{
			// close enough: stop, face the target and hit it
			StopMovement();

			const FVector ToTarget = TargetActor->GetActorLocation() - Infected->GetActorLocation();
			const FRotator FacingRotation(0.0f, ToTarget.Rotation().Yaw, 0.0f);
			Infected->SetActorRotation(FMath::RInterpTo(Infected->GetActorRotation(), FacingRotation, DeltaSeconds, 12.0f));

			Infected->TryAttack(TargetActor);

		} else {

			// close the distance, recalculating the path a few times per second
			RepathTimer -= DeltaSeconds;

			if (RepathTimer <= 0.0f)
			{
				RepathTimer = 0.3f;
				MoveToActor(TargetActor, Infected->GetAttackRange() * 0.6f, true, true, false, nullptr, true);
			}
		}
		break;
	}
	}
}
