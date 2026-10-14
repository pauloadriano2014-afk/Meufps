// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShooterHUD.generated.h"

class AShooterCharacter;
class AMissionManager;

/**
 *  Draws the parts of the shooter HUD that don't live in the UMG widgets:
 *  the mission objectives and messages, the magazine and reserve ammo readout,
 *  the reloading notice and the hit marker
 */
UCLASS()
class MEUFPS_API AShooterHUD : public AHUD
{
	GENERATED_BODY()

protected:

	/** How long the hit marker stays on screen after a hit */
	UPROPERTY(EditAnywhere, Category="Hit Marker", meta = (ClampMin = 0, Units = "s"))
	float HitMarkerDuration = 0.2f;

	/** Distance from the screen center where the hit marker lines start */
	UPROPERTY(EditAnywhere, Category="Hit Marker", meta = (ClampMin = 0, Units = "px"))
	float HitMarkerInnerRadius = 8.0f;

	/** Distance from the screen center where the hit marker lines end */
	UPROPERTY(EditAnywhere, Category="Hit Marker", meta = (ClampMin = 0, Units = "px"))
	float HitMarkerOuterRadius = 18.0f;

	/** Scale of the ammo readout text */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0.1))
	float AmmoTextScale = 1.5f;

	/** Scale of the mission message text shown at the center of the screen */
	UPROPERTY(EditAnywhere, Category="Mission", meta = (ClampMin = 0.1))
	float BannerTextScale = 1.3f;

	/** The mission manager of the level, found lazily */
	TWeakObjectPtr<AMissionManager> Mission;

	/** World time of the next attempt to find a mission manager */
	float NextMissionSearchTime = 0.0f;

protected:

	/** Draws the HUD every frame */
	virtual void DrawHUD() override;

	/** Draws the mission title, the active objectives and the center message */
	void DrawMission();

	/** Draws the ammo readout and the hit marker for a living character */
	void DrawCombat(const AShooterCharacter* Character);

	/** Splits text into lines that fit the given width. Existing line breaks are kept */
	void WrapText(const FString& Text, float MaxWidth, float Scale, TArray<FString>& OutLines) const;
};
