// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "InteractObjective.generated.h"

class USphereComponent;

/**
 *  Objective completed by holding the interact key (E) next to it for a while
 *  Progress drains slowly if the player lets go or walks away
 *  Can make noise while it is being used, which attracts nearby enemies
 *  The sphere shows how close the player has to be. Move the actor and resize the sphere in the level
 */
UCLASS(ClassGroup=(Mission))
class MEUFPS_API AInteractObjective : public AMissionObjective
{
	GENERATED_BODY()

	friend class AMission01Builder;

	/** How close the player has to be to interact */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* Range;

protected:

	/** Text shown next to the progress bar while the player is in range */
	UPROPERTY(EditAnywhere, Category="Interact")
	FText Prompt = INVTEXT("Segure E para interagir");

	/** How long the key has to be held */
	UPROPERTY(EditAnywhere, Category="Interact", meta = (ClampMin = 0.1, Units = "s"))
	float HoldTime = 6.0f;

	/** If true, using this objective makes noise that enemies can hear */
	UPROPERTY(EditAnywhere, Category="Interact|Noise")
	bool bMakeNoise = false;

	/** Time between noises while the objective is being used */
	UPROPERTY(EditAnywhere, Category="Interact|Noise", meta = (ClampMin = 0.1, Units = "s"))
	float NoiseInterval = 2.0f;

	/** How far the noise can be heard */
	UPROPERTY(EditAnywhere, Category="Interact|Noise", meta = (ClampMin = 0, Units = "cm"))
	float NoiseRange = 4000.0f;

	/** Loudness of each noise */
	UPROPERTY(EditAnywhere, Category="Interact|Noise", meta = (ClampMin = 0))
	float NoiseLoudness = 1.0f;

	/** Seconds of holding the key so far */
	float Progress = 0.0f;

	/** Time left until the next noise */
	float NoiseTimer = 0.0f;

	/** True while the player is close enough to interact */
	bool bPlayerInRange = false;

public:

	AInteractObjective();

	/** Updates the progress while the objective is active */
	virtual void Tick(float DeltaSeconds) override;

	/** Returns true if the player is close enough to interact */
	bool IsPlayerInRange() const { return bPlayerInRange; }

	/** Returns the progress from 0 to 1 */
	float GetProgress01() const { return FMath::Clamp(Progress / HoldTime, 0.0f, 1.0f); }

	/** Returns the prompt text */
	FString GetPrompt() const { return Prompt.ToString(); }

protected:

	/** Starts tracking the player */
	virtual void OnObjectiveActivated() override;
};
