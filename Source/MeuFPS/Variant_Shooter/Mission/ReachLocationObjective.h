// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "ReachLocationObjective.generated.h"

class UBoxComponent;

/**
 *  Objective completed when the player walks into its trigger box
 *  Move and scale the actor in the level to place the destination
 */
UCLASS(ClassGroup=(Mission))
class MEUFPS_API AReachLocationObjective : public AMissionObjective
{
	GENERATED_BODY()

	/** Area the player has to reach */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Trigger;

public:

	AReachLocationObjective();

protected:

	/** Completes right away if the player is already inside the area */
	virtual void OnObjectiveActivated() override;

	/** Called when something enters the trigger box */
	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Returns true if the actor is a player-controlled pawn */
	bool IsPlayer(const AActor* Actor) const;
};
