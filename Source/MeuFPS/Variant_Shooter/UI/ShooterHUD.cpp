// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterHUD.h"
#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void AShooterHUD::DrawHUD()
{
	Super::DrawHUD();

	// only draw for a living shooter character
	const AShooterCharacter* Character = Cast<AShooterCharacter>(GetOwningPawn());
	if (!Character || Character->IsDead() || !Canvas)
	{
		return;
	}

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
