// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

class UShooterUI;

/** Broadcast with the team of every character that dies. Mission objectives use it to count kills */
DECLARE_MULTICAST_DELEGATE_OneParam(FTeamMemberDiedDelegate, uint8 /*TeamByte*/);

/**
 *  Simple GameMode for a first person shooter game
 *  Manages game UI
 *  Keeps track of team scores
 */
UCLASS(abstract)
class MEUFPS_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	/** Type of UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Shooter")
	TSubclassOf<UShooterUI> ShooterUIClass;

	/** Pointer to the UI widget */
	TObjectPtr<UShooterUI> ShooterUI;

	/** Map of scores by team ID */
	TMap<uint8, int32> TeamScores;

protected:

	/** Determines how many local players should be spawned on game start */
	UPROPERTY(EditDefaultsOnly, Category="Local Multiplayer", meta = (ClampMin = 1, ClampMax = 4))
	int32 NumberOfLocalPlayers = 1;

	/** Used to assign players to different PlayerStarts in the level */
	int32 CurrentPlayerStartAssignment = 0;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Assigns a PlayerStart to a specific player */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

public:

	/** Broadcast whenever a character dies, with the team of the character that died */
	FTeamMemberDiedDelegate OnTeamMemberDied;

public:

	/** Increases the score for the given team. Characters call it when they die, so it also announces the death */
	void IncrementTeamScore(uint8 TeamByte);

	/** Returns true if enemy NPCs should be used */
	bool ShouldSpawnEnemyNPCs() const;
};
