// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MissionObjective.generated.h"

class USceneComponent;
class AMissionObjective;

/** Broadcast when an objective is completed */
DECLARE_MULTICAST_DELEGATE_OneParam(FObjectiveCompletedDelegate, AMissionObjective*);

/**
 *  Base class for the objectives of a mission
 *  Objectives are placed in the level. The Mission Manager finds them, sorts them by Order and activates them in sequence
 *  Objectives that share the same Order are active at the same time and must all be completed to move on
 */
UCLASS(Abstract, ClassGroup=(Mission))
class MEUFPS_API AMissionObjective : public AActor
{
	GENERATED_BODY()

	/** Root component so subclasses have something to attach to */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

protected:

	/** Text shown to the player while this objective is active */
	UPROPERTY(EditAnywhere, Category="Objective")
	FText Description = INVTEXT("Objetivo");

	/** Position of this objective in the mission. Lower numbers come first. Equal numbers are done at the same time */
	UPROPERTY(EditAnywhere, Category="Objective")
	int32 Order = 0;

	/** If true, the objective is currently being tracked */
	bool bIsActive = false;

	/** If true, the objective has been completed */
	bool bIsCompleted = false;

public:

	/** Broadcast when this objective is completed */
	FObjectiveCompletedDelegate OnObjectiveCompleted;

public:

	AMissionObjective();

	/** Starts tracking this objective. Called by the Mission Manager */
	void ActivateObjective();

	/** Marks this objective as completed */
	UFUNCTION(BlueprintCallable, Category="Mission")
	void CompleteObjective();

	/** Returns the objective position in the mission */
	int32 GetOrder() const { return Order; }

	/** Returns true while the objective is being tracked */
	bool IsActive() const { return bIsActive; }

	/** Returns true once the objective is completed */
	bool IsCompleted() const { return bIsCompleted; }

	/** Returns the plain description text */
	FString GetDescriptionText() const { return Description.ToString(); }

	/** Returns the description followed by the progress, for the HUD */
	FString GetDisplayText() const;

protected:

	/** Called when the objective becomes active. Subclasses start tracking here */
	virtual void OnObjectiveActivated() {}

	/** Returns a short progress string such as (2/5), or an empty string */
	virtual FString GetProgressText() const { return FString(); }

	/** Called when the objective becomes active. Implement in Blueprint for custom effects */
	UFUNCTION(BlueprintImplementableEvent, Category="Mission", meta = (DisplayName = "On Objective Activated"))
	void BP_OnObjectiveActivated();

	/** Called when the objective is completed. Implement in Blueprint for custom effects */
	UFUNCTION(BlueprintImplementableEvent, Category="Mission", meta = (DisplayName = "On Objective Completed"))
	void BP_OnObjectiveCompleted();
};
