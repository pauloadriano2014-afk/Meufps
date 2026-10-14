// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "SurviveObjective.generated.h"

/**
 *  Objective completed when a countdown runs out while the objective is active
 *  Meant for holding a position or lasting through an attack
 */
UCLASS(ClassGroup=(Mission))
class MEUFPS_API ASurviveObjective : public AMissionObjective
{
	GENERATED_BODY()

protected:

	/** How long the player has to survive */
	UPROPERTY(EditAnywhere, Category="Objective", meta = (ClampMin = 1, Units = "s"))
	float Duration = 60.0f;

	/** Timer that completes the objective */
	FTimerHandle SurviveTimer;

public:

	ASurviveObjective();

protected:

	/** Starts the countdown */
	virtual void OnObjectiveActivated() override;

	/** Returns the remaining time in seconds */
	virtual FString GetProgressText() const override;

	/** Gameplay cleanup */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Called when the countdown ends */
	void OnSurvived();
};
