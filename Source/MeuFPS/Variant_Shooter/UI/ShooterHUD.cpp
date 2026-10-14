// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterHUD.h"
#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "Variant_Shooter/Mission/MissionManager.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void AShooterHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	// mission text is drawn even while the player is dead
	DrawMission();

	// the combat HUD only exists for a living shooter character
	const AShooterCharacter* Character = Cast<AShooterCharacter>(GetOwningPawn());
	if (Character && !Character->IsDead())
	{
		DrawCombat(Character);
	}
}

void AShooterHUD::DrawMission()
{
	const float Time = GetWorld()->GetTimeSeconds();

	// look for the level's mission manager, but not every frame if there isn't one
	if (!Mission.IsValid() && Time >= NextMissionSearchTime)
	{
		Mission = AMissionManager::Get(this);
		NextMissionSearchTime = Time + 1.0f;
	}

	if (!Mission.IsValid())
	{
		return;
	}

	UFont* Font = GEngine->GetMediumFont();
	const FLinearColor Accent(1.0f, 0.6f, 0.1f);

	// title and active objectives, below the health bar
	TArray<FString> Lines;
	Mission->GetObjectiveLines(Lines);

	if (Lines.Num() > 0)
	{
		float X = 40.0f;
		float Y = 110.0f;

		DrawText(Mission->GetMissionTitle().ToUpper(), Accent, X, Y, Font, 1.0f);
		Y += 26.0f;

		for (const FString& Line : Lines)
		{
			DrawText(TEXT("- ") + Line, FLinearColor::White, X, Y, Font, 1.0f);
			Y += 24.0f;
		}
	}

	// message at the center of the screen: briefing, objective done, checkpoint, mission result
	FString BannerText;

	if (Mission->GetBanner(Time, BannerText))
	{
		const float MaxWidth = Canvas->ClipX * 0.6f;

		TArray<FString> BannerLines;
		WrapText(BannerText, MaxWidth, BannerTextScale, BannerLines);

		float Width = 0.0f;
		float LineHeight = 0.0f;
		GetTextSize(TEXT("Ag"), Width, LineHeight, Font, BannerTextScale);
		LineHeight *= 1.2f;

		const float Padding = 20.0f;
		const float TopY = Canvas->ClipY * 0.15f;
		const float BoxHeight = (LineHeight * BannerLines.Num()) + (Padding * 2.0f);

		DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f), (Canvas->ClipX * 0.5f) - (MaxWidth * 0.5f) - Padding, TopY - Padding, MaxWidth + (Padding * 2.0f), BoxHeight);

		float Y = TopY;

		for (int32 i = 0; i < BannerLines.Num(); ++i)
		{
			float LineWidth = 0.0f;
			float UnusedHeight = 0.0f;
			GetTextSize(BannerLines[i], LineWidth, UnusedHeight, Font, BannerTextScale);

			// the first line is the title
			DrawText(BannerLines[i], i == 0 ? Accent : FLinearColor::White, (Canvas->ClipX * 0.5f) - (LineWidth * 0.5f), Y, Font, BannerTextScale);
			Y += LineHeight;
		}
	}
}

void AShooterHUD::DrawCombat(const AShooterCharacter* Character)
{
	const float CenterX = Canvas->ClipX * 0.5f;
	const float CenterY = Canvas->ClipY * 0.5f;

	// hit marker: four short diagonal lines around the crosshair, red for headshots
	const float TimeSinceHit = GetWorld()->GetTimeSeconds() - Character->GetLastHitConfirmTime();

	if (TimeSinceHit >= 0.0f && TimeSinceHit < HitMarkerDuration)
	{
		const FLinearColor MarkerColor = Character->WasLastHitHeadshot() ? FLinearColor::Red : FLinearColor::White;
		const float Inner = HitMarkerInnerRadius;
		const float Outer = HitMarkerOuterRadius;

		DrawLine(CenterX - Outer, CenterY - Outer, CenterX - Inner, CenterY - Inner, MarkerColor, 2.0f);
		DrawLine(CenterX + Outer, CenterY - Outer, CenterX + Inner, CenterY - Inner, MarkerColor, 2.0f);
		DrawLine(CenterX - Outer, CenterY + Outer, CenterX - Inner, CenterY + Inner, MarkerColor, 2.0f);
		DrawLine(CenterX + Outer, CenterY + Outer, CenterX + Inner, CenterY + Inner, MarkerColor, 2.0f);
	}

	// ammo readout in the bottom right corner: magazine / reserve
	if (const AShooterWeapon* Weapon = Character->GetCurrentWeapon())
	{
		FString AmmoText = FString::Printf(TEXT("%d / %d"), Weapon->GetBulletCount(), Weapon->GetReserveAmmo());

		if (Weapon->IsReloading())
		{
			AmmoText += TEXT("  RELOADING");
		}

		// warn when the magazine is nearly empty
		const bool bLowAmmo = Weapon->GetBulletCount() <= FMath::Max(1, Weapon->GetMagazineSize() / 4);
		const FLinearColor AmmoColor = (bLowAmmo || Weapon->IsReloading()) ? FLinearColor(1.0f, 0.6f, 0.1f) : FLinearColor::White;

		DrawText(AmmoText, AmmoColor, Canvas->ClipX - 380.0f, Canvas->ClipY - 90.0f, GEngine->GetLargeFont(), AmmoTextScale);
	}
}

void AShooterHUD::WrapText(const FString& Text, float MaxWidth, float Scale, TArray<FString>& OutLines) const
{
	UFont* Font = GEngine->GetMediumFont();

	// keep the line breaks the designer typed
	TArray<FString> Paragraphs;
	Text.ParseIntoArrayLines(Paragraphs, false);

	for (const FString& Paragraph : Paragraphs)
	{
		TArray<FString> Words;
		Paragraph.ParseIntoArray(Words, TEXT(" "), true);

		FString Line;

		for (const FString& Word : Words)
		{
			const FString Candidate = Line.IsEmpty() ? Word : Line + TEXT(" ") + Word;

			float Width = 0.0f;
			float Height = 0.0f;
			GetTextSize(Candidate, Width, Height, Font, Scale);

			if (Width > MaxWidth && !Line.IsEmpty())
			{
				// the word doesn't fit, start a new line with it
				OutLines.Add(Line);
				Line = Word;

			} else {

				Line = Candidate;
			}
		}

		OutLines.Add(Line);
	}
}
