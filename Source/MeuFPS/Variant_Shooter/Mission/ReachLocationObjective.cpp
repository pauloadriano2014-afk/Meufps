// Copyright Epic Games, Inc. All Rights Reserved.


#include "ReachLocationObjective.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

AReachLocationObjective::AReachLocationObjective()
{
	Description = INVTEXT("Chegue ao local marcado");

	// create the trigger box
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(RootComponent);

	Trigger->SetBoxExtent(FVector(200.0f, 200.0f, 150.0f));
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionObjectType(ECC_WorldStatic);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// subscribe to the overlap event
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AReachLocationObjective::OnTriggerOverlap);
}

void AReachLocationObjective::OnObjectiveActivated()
{
	// the player may already be standing inside the area
	TArray<AActor*> Overlapping;
	Trigger->GetOverlappingActors(Overlapping, APawn::StaticClass());

	for (const AActor* Actor : Overlapping)
	{
		if (IsPlayer(Actor))
		{
			CompleteObjective();
			return;
		}
	}
}

void AReachLocationObjective::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// only the active objective reacts, and only to the player
	if (bIsActive && IsPlayer(OtherActor))
	{
		CompleteObjective();
	}
}

bool AReachLocationObjective::IsPlayer(const AActor* Actor) const
{
	const APawn* Pawn = Cast<APawn>(Actor);
	return Pawn && Pawn->IsPlayerControlled();
}
