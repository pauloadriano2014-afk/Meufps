// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "EliminateObjective.generated.h"

/**
 *  Objective completed after enough enemies die while it is active
 *  Counts every death of a character from the enemy team, whether placed in the level or made by a spawner
 */
UCLASS(ClassGroup=(Mission))
class MEUFPS_API AEliminateObjective : public AMissionObjective
{
	GENERATED_BODY()

	friend class AMission01Builder;

protected:

	/** How many enemies have to die */
	UPROPERTY(EditAnywhere, Category="Objective", meta = (ClampMin = 1))
	int32 KillsRequired = 5;

	/** Team of the characters that count as enemies */
	UPROPERTY(EditAnywhere, Category="Objective")
	uint8 EnemyTeam = 1;

	/** Enemies killed since this objective became active */
	int32 Kills = 0;

	/** Handle of the subscription to the game mode's death notification */
	FDelegateHandle DeathHandle;

public:

	AEliminateObjective();

protected:

	/** Starts counting kills */
	virtual void OnObjectiveActivated() override;

	/** Returns the kill count as (kills/required) */
	virtual FString GetProgressText() const override;

	/** Gameplay cleanup */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Called by the game mode whenever a character dies */
	void OnTeamMemberDied(uint8 TeamByte);

	/** Stops listening for deaths */
	void StopCounting();
};
