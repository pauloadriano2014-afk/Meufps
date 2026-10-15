// Copyright Epic Games, Inc. All Rights Reserved.


#include "MissionManager.h"
#include "MissionObjective.h"
#include "InteractObjective.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

AMissionManager::AMissionManager()
{
	PrimaryActorTick.bCanEverTick = false;

	// create the root
	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

AMissionManager* AMissionManager::Get(const UObject* WorldContext)
{
	return Cast<AMissionManager>(UGameplayStatics::GetActorOfClass(WorldContext, AMissionManager::StaticClass()));
}

void AMissionManager::BeginPlay()
{
	Super::BeginPlay();

	// find every objective placed in the level
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, AMissionObjective::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (AMissionObjective* Objective = Cast<AMissionObjective>(Actor))
		{
			Objectives.Add(Objective);

			// listen for completion
			Objective->OnObjectiveCompleted.AddUObject(this, &AMissionManager::OnObjectiveCompleted);
		}
	}

	// sort by order. Objectives with the same order keep their relative position
	Objectives.StableSort([](const AMissionObjective& A, const AMissionObjective& B)
	{
		return A.GetOrder() < B.GetOrder();
	});

	if (bAutoStart)
	{
		StartMission();
	}
}

void AMissionManager::StartMission()
{
	if (State != EMissionState::NotStarted)
	{
		return;
	}

	State = EMissionState::Active;

	// show the title and briefing
	FString Text = MissionTitle.ToString();

	if (!Briefing.IsEmpty())
	{
		Text += TEXT("\n") + Briefing.ToString();
	}

	ShowBanner(Text, BriefingDuration);

	BP_OnMissionStarted();

	ActivateNextGroup();
}

void AMissionManager::FailMission(const FText& Reason)
{
	if (State != EMissionState::Active)
	{
		return;
	}

	State = EMissionState::Failed;

	ShowBanner(TEXT("MISSÃO FALHOU\n") + Reason.ToString(), 8.0f);

	BP_OnMissionFailed();
}

void AMissionManager::SetCheckpoint(const FTransform& Transform)
{
	bHasCheckpoint = true;
	CheckpointTransform = Transform;
}

bool AMissionManager::GetCheckpoint(FTransform& OutTransform) const
{
	if (bHasCheckpoint)
	{
		OutTransform = CheckpointTransform;
	}

	return bHasCheckpoint;
}

void AMissionManager::ShowBanner(const FString& Text, float Duration)
{
	BannerText = Text;
	BannerEndTime = GetWorld()->GetTimeSeconds() + Duration;
}

void AMissionManager::GetObjectiveLines(TArray<FString>& OutLines) const
{
	if (State != EMissionState::Active)
	{
		return;
	}

	for (const AMissionObjective* Objective : Objectives)
	{
		if (Objective->IsActive())
		{
			OutLines.Add(Objective->GetDisplayText());
		}
	}
}

bool AMissionManager::GetBanner(float WorldTime, FString& OutText) const
{
	if (WorldTime < BannerEndTime)
	{
		OutText = BannerText;
		return true;
	}

	return false;
}

bool AMissionManager::GetInteractPrompt(FString& OutPrompt, float& OutProgress) const
{
	if (State != EMissionState::Active)
	{
		return false;
	}

	for (const AMissionObjective* Objective : Objectives)
	{
		if (!Objective->IsActive())
		{
			continue;
		}

		if (const AInteractObjective* Interact = Cast<AInteractObjective>(Objective))
		{
			if (Interact->IsPlayerInRange())
			{
				OutPrompt = Interact->GetPrompt();
				OutProgress = Interact->GetProgress01();
				return true;
			}
		}
	}

	return false;
}

void AMissionManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorldTimerManager().ClearTimer(NextLevelTimer);
}

void AMissionManager::OpenNextLevel()
{
	UGameplayStatics::OpenLevel(this, NextLevelName);
}

void AMissionManager::ActivateNextGroup()
{
	// find the lowest order among the objectives that are still open
	bool bFoundOpen = false;
	int32 NextOrder = MAX_int32;

	for (const AMissionObjective* Objective : Objectives)
	{
		if (!Objective->IsCompleted())
		{
			bFoundOpen = true;
			NextOrder = FMath::Min(NextOrder, Objective->GetOrder());
		}
	}

	// nothing left to do
	if (!bFoundOpen)
	{
		CompleteMission();
		return;
	}

	CurrentOrder = NextOrder;

	// collect the group first, because an objective may complete the moment it is activated
	TArray<AMissionObjective*> Group;

	for (AMissionObjective* Objective : Objectives)
	{
		if (!Objective->IsCompleted() && Objective->GetOrder() == NextOrder)
		{
			Group.Add(Objective);
		}
	}

	for (AMissionObjective* Objective : Group)
	{
		Objective->ActivateObjective();
	}
}

void AMissionManager::OnObjectiveCompleted(AMissionObjective* Objective)
{
	if (State != EMissionState::Active)
	{
		return;
	}

	ShowBanner(FString::Printf(TEXT("Objetivo concluído: %s"), *Objective->GetDescriptionText()), 3.0f);

	// wait until every objective of the current group is done
	for (const AMissionObjective* Other : Objectives)
	{
		if (!Other->IsCompleted() && Other->GetOrder() == CurrentOrder)
		{
			return;
		}
	}

	ActivateNextGroup();
}

void AMissionManager::CompleteMission()
{
	if (State != EMissionState::Active)
	{
		return;
	}

	State = EMissionState::Completed;

	FString Text = TEXT("MISSÃO CUMPRIDA");

	if (!CompletionText.IsEmpty())
	{
		Text += TEXT("\n") + CompletionText.ToString();
	}

	ShowBanner(Text, 10.0f);

	BP_OnMissionCompleted();

	// move on to the next level after a pause, if there is one
	if (!NextLevelName.IsNone())
	{
		GetWorldTimerManager().SetTimer(NextLevelTimer, this, &AMissionManager::OpenNextLevel, NextLevelDelay, false);
	}
}
