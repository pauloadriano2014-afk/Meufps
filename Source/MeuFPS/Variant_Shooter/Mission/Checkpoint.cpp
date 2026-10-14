// Copyright Epic Games, Inc. All Rights Reserved.


#include "Checkpoint.h"
#include "MissionManager.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Pawn.h"

ACheckpoint::ACheckpoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// create the trigger box as the root
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	RootComponent = Trigger;

	Trigger->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionObjectType(ECC_WorldStatic);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// subscribe to the overlap event
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnTriggerOverlap);

	// create the facing arrow
	Facing = CreateDefaultSubobject<UArrowComponent>(TEXT("Facing"));
	Facing->SetupAttachment(RootComponent);
}

void ACheckpoint::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// only the player triggers checkpoints, and each one only works once
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (bReached || !Pawn || !Pawn->IsPlayerControlled())
	{
		return;
	}

	AMissionManager* Mission = AMissionManager::Get(this);
	if (!Mission)
	{
		return;
	}

	bReached = true;

	// the player respawns here, facing the arrow
	Mission->SetCheckpoint(FTransform(GetActorRotation(), GetActorLocation() + FVector(0.0f, 0.0f, RespawnHeight)));

	Mission->ShowBanner(Message.ToString(), 2.5f);
}
