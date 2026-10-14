// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Checkpoint.generated.h"

class UBoxComponent;
class UArrowComponent;

/**
 *  Saves the player's progress when walked through
 *  After dying, the player respawns at the last checkpoint reached, facing the way the arrow points
 *  Needs a Mission Manager in the level
 */
UCLASS(ClassGroup=(Mission))
class MEUFPS_API ACheckpoint : public AActor
{
	GENERATED_BODY()

	/** Area that activates the checkpoint */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Trigger;

	/** Shows where the player will face after respawning */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UArrowComponent* Facing;

protected:

	/** Message shown when the checkpoint is reached */
	UPROPERTY(EditAnywhere, Category="Checkpoint")
	FText Message = INVTEXT("Checkpoint");

	/** How high above the actor the player respawns */
	UPROPERTY(EditAnywhere, Category="Checkpoint", meta = (ClampMin = 0, Units = "cm"))
	float RespawnHeight = 100.0f;

	/** True once the player has reached this checkpoint */
	bool bReached = false;

public:

	ACheckpoint();

protected:

	/** Called when something enters the trigger box */
	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
