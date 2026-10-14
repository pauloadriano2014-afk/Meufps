// Copyright Epic Games, Inc. All Rights Reserved.


#include "EliminateObjective.h"
#include "Variant_Shooter/ShooterGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AEliminateObjective::AEliminateObjective()
{
	Description = INVTEXT("Elimine os inimigos");
}

void AEliminateObjective::OnObjectiveActivated()
{
	Kills = 0;

	// listen for character deaths
	if (AShooterGameMode* GM = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		DeathHandle = GM->OnTeamMemberDied.AddUObject(this, &AEliminateObjective::OnTeamMemberDied);
	}
}

FString AEliminateObjective::GetProgressText() const
{
	return FString::Printf(TEXT("(%d/%d)"), Kills, KillsRequired);
}

void AEliminateObjective::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	StopCounting();
}

void AEliminateObjective::OnTeamMemberDied(uint8 TeamByte)
{
	// only enemies count, and only while the objective is active
	if (!bIsActive || TeamByte != EnemyTeam)
	{
		return;
	}

	++Kills;

	if (Kills >= KillsRequired)
	{
		StopCounting();
		CompleteObjective();
	}
}

void AEliminateObjective::StopCounting()
{
	if (DeathHandle.IsValid())
	{
		if (AShooterGameMode* GM = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			GM->OnTeamMemberDied.Remove(DeathHandle);
		}

		DeathHandle.Reset();
	}
}
