// Copyright Epic Games, Inc. All Rights Reserved.


#include "MissionObjective.h"
#include "Components/SceneComponent.h"

AMissionObjective::AMissionObjective()
{
	PrimaryActorTick.bCanEverTick = false;

	// create the root
	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AMissionObjective::ActivateObjective()
{
	// ignore if we're already tracking or already done
	if (bIsActive || bIsCompleted)
	{
		return;
	}

	bIsActive = true;

	// let the subclass start tracking
	OnObjectiveActivated();

	// let Blueprints react
	BP_OnObjectiveActivated();
}

void AMissionObjective::CompleteObjective()
{
	if (bIsCompleted)
	{
		return;
	}

	bIsActive = false;
	bIsCompleted = true;

	// let Blueprints react
	BP_OnObjectiveCompleted();

	// tell the Mission Manager
	OnObjectiveCompleted.Broadcast(this);
}

FString AMissionObjective::GetDisplayText() const
{
	FString Text = Description.ToString();

	const FString Progress = GetProgressText();
	if (!Progress.IsEmpty())
	{
		Text += TEXT(" ") + Progress;
	}

	return Text;
}
