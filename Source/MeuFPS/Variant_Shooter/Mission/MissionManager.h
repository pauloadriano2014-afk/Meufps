// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MissionManager.generated.h"

class USceneComponent;
class AMissionObjective;

/** Where a mission stands */
UENUM(BlueprintType)
enum class EMissionState : uint8
{
	NotStarted,
	Active,
	Completed,
	Failed
};

/**
 *  Runs the mission of a level. Place one per level
 *  Finds the objectives in the level, activates them in order, keeps the last checkpoint and
 *  provides the text that the HUD shows
 */
UCLASS(ClassGroup=(Mission))
class MEUFPS_API AMissionManager : public AActor
{
	GENERATED_BODY()

	/** Root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

protected:

	/** Mission name, shown in the briefing and in the HUD */
	UPROPERTY(EditAnywhere, Category="Mission")
	FText MissionTitle = INVTEXT("Missao");

	/** Short briefing shown when the mission starts */
	UPROPERTY(EditAnywhere, Category="Mission", meta = (MultiLine = true))
	FText Briefing = INVTEXT("");

	/** Text shown when every objective is done */
	UPROPERTY(EditAnywhere, Category="Mission", meta = (MultiLine = true))
	FText CompletionText = INVTEXT("");

	/** How long the briefing stays on screen */
	UPROPERTY(EditAnywhere, Category="Mission", meta = (ClampMin = 1, Units = "s"))
	float BriefingDuration = 8.0f;

	/** If true, the mission starts as soon as the level begins */
	UPROPERTY(EditAnywhere, Category="Mission")
	bool bAutoStart = true;

	/** Current state of the mission */
	EMissionState State = EMissionState::NotStarted;

	/** Every objective in the level, sorted by order */
	UPROPERTY()
	TArray<TObjectPtr<AMissionObjective>> Objectives;

	/** Order of the group of objectives that is currently active */
	int32 CurrentOrder = 0;

	/** Message currently shown at the center of the screen */
	FString BannerText;

	/** World time at which the message disappears */
	float BannerEndTime = 0.0f;

	/** True once the player has reached a checkpoint */
	bool bHasCheckpoint = false;

	/** Where the player respawns after the last checkpoint */
	FTransform CheckpointTransform;

public:

	AMissionManager();

	/** Returns the mission manager of the current level, or null if the level has none */
	static AMissionManager* Get(const UObject* WorldContext);

	/** Starts the mission and activates the first objectives */
	UFUNCTION(BlueprintCallable, Category="Mission")
	void StartMission();

	/** Ends the mission as a failure */
	UFUNCTION(BlueprintCallable, Category="Mission")
	void FailMission(const FText& Reason);

	/** Remembers where the player should respawn */
	void SetCheckpoint(const FTransform& Transform);

	/** Returns true and the respawn transform if a checkpoint was reached */
	bool GetCheckpoint(FTransform& OutTransform) const;

	/** Shows a message at the center of the screen */
	void ShowBanner(const FString& Text, float Duration);

	/** Returns the state of the mission */
	EMissionState GetState() const { return State; }

	/** Returns the mission name */
	FString GetMissionTitle() const { return MissionTitle.ToString(); }

	/** Returns the HUD text of each active objective */
	void GetObjectiveLines(TArray<FString>& OutLines) const;

	/** Returns true and the message text if a message is on screen at the given world time */
	bool GetBanner(float WorldTime, FString& OutText) const;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Activates the next group of objectives, or completes the mission if none are left */
	void ActivateNextGroup();

	/** Called when any objective is completed */
	void OnObjectiveCompleted(AMissionObjective* Objective);

	/** Ends the mission as a success */
	void CompleteMission();

	/** Called when the mission starts. Implement in Blueprint for custom effects */
	UFUNCTION(BlueprintImplementableEvent, Category="Mission", meta = (DisplayName = "On Mission Started"))
	void BP_OnMissionStarted();

	/** Called when the mission is completed. Implement in Blueprint, for example to load the next level */
	UFUNCTION(BlueprintImplementableEvent, Category="Mission", meta = (DisplayName = "On Mission Completed"))
	void BP_OnMissionCompleted();

	/** Called when the mission fails. Implement in Blueprint for custom effects */
	UFUNCTION(BlueprintImplementableEvent, Category="Mission", meta = (DisplayName = "On Mission Failed"))
	void BP_OnMissionFailed();
};
