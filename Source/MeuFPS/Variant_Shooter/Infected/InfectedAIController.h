// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "InfectedAIController.generated.h"

class UAIPerceptionComponent;
struct FAIStimulus;

/** What an infected is doing right now */
enum class EInfectedAIState : uint8
{
	/** Standing around, or wandering near its spawn point */
	Idle,

	/** Heading to a place where it heard something */
	Investigate,

	/** Going after the player */
	Chase
};

/**
 *  Brain of an infected
 *  Sees the player and hears gunshots and other noise. Chases when it sees or is hurt, investigates when it hears
 *  Gives up and investigates the last known position if it loses the target for a while
 */
UCLASS()
class MEUFPS_API AInfectedAIController : public AAIController
{
	GENERATED_BODY()

	/** Detects the player through sight and hearing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent* AIPerception;

protected:

	/** Seconds without seeing the target before the infected gives up the chase */
	UPROPERTY(EditAnywhere, Category="Behavior", meta = (ClampMin = 0, Units = "s"))
	float LoseTargetTime = 6.0f;

	/** Seconds spent looking around after reaching a place it was investigating */
	UPROPERTY(EditAnywhere, Category="Behavior", meta = (ClampMin = 0, Units = "s"))
	float InvestigateWaitTime = 4.0f;

	/** How far from its spawn point it wanders while idle */
	UPROPERTY(EditAnywhere, Category="Behavior", meta = (ClampMin = 0, Units = "cm"))
	float WanderRadius = 600.0f;

	/** Current behavior */
	EInfectedAIState State = EInfectedAIState::Idle;

	/** The actor being chased */
	TWeakObjectPtr<AActor> Target;

	/** Where the infected spawned, the center of its wandering */
	FVector HomeLocation = FVector::ZeroVector;

	/** Where it is heading while investigating */
	FVector InvestigateLocation = FVector::ZeroVector;

	/** Seconds since the target was last in view */
	float TimeSinceTargetSeen = 0.0f;

	/** Seconds left to wait after investigating */
	float WaitTimer = 0.0f;

	/** Seconds until the path to the target is recalculated */
	float RepathTimer = 0.0f;

	/** Seconds until the next line of sight check */
	float SightCheckTimer = 0.0f;

	/** Seconds until the next wander move */
	float WanderTimer = 0.0f;

public:

	AInfectedAIController();

	/** Makes the infected go after the actor. Used when it is hurt by it */
	void AlertTo(AActor* Actor);

protected:

	/** Pawn initialization */
	virtual void OnPossess(APawn* InPawn) override;

	/** Runs the behavior */
	virtual void Tick(float DeltaSeconds) override;

	/** Called when the perception component sees or hears something */
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	/** Starts chasing the actor */
	void StartChase(AActor* Actor);

	/** Walks to a place and looks around when it gets there */
	void Investigate(const FVector& Location);

	/** Stops and goes back to idle behavior */
	void GoIdle();

	/** Returns true if the actor is a player-controlled pawn */
	bool IsPlayerPawn(const AActor* Actor) const;
};
