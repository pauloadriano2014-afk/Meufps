// Copyright Epic Games, Inc. All Rights Reserved.


#include "SurviveObjective.h"
#include "Engine/World.h"
#include "TimerManager.h"

ASurviveObjective::ASurviveObjective()
{
	Description = INVTEXT("Sobreviva");
}

void ASurviveObjective::OnObjectiveActivated()
{
	GetWorldTimerManager().SetTimer(SurviveTimer, this, &ASurviveObjective::OnSurvived, Duration, false);
}

FString ASurviveObjective::GetProgressText() const
{
	if (!bIsActive)
	{
		return FString();
	}

	const float Remaining = GetWorldTimerManager().GetTimerRemaining(SurviveTimer);

	return FString::Printf(TEXT("(%ds)"), FMath::Max(0, FMath::CeilToInt(Remaining)));
}

void ASurviveObjective::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorldTimerManager().ClearTimer(SurviveTimer);
}

void ASurviveObjective::OnSurvived()
{
	CompleteObjective();
}
