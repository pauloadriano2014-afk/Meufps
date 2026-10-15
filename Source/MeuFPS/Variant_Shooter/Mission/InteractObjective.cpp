// Copyright Epic Games, Inc. All Rights Reserved.


#include "InteractObjective.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"

AInteractObjective::AInteractObjective()
{
	// ticks only while the objective is active
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Description = INVTEXT("Interaja com o objeto");

	// the sphere shows the interaction range in the editor
	Range = CreateDefaultSubobject<USphereComponent>(TEXT("Range"));
	Range->SetupAttachment(RootComponent);

	Range->SetSphereRadius(300.0f);
	Range->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Range->SetHiddenInGame(true);
}

void AInteractObjective::OnObjectiveActivated()
{
	Progress = 0.0f;
	NoiseTimer = 0.0f;
	bPlayerInRange = false;

	SetActorTickEnabled(true);
}

void AInteractObjective::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// stop ticking once the objective is done
	if (!bIsActive)
	{
		bPlayerInRange = false;
		SetActorTickEnabled(false);
		return;
	}

	// find out if the living player is close enough
	AShooterCharacter* Player = Cast<AShooterCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));

	bPlayerInRange = Player && !Player->IsDead() && FVector::DistSquared(Player->GetActorLocation(), GetActorLocation()) <= FMath::Square(Range->GetScaledSphereRadius());

	if (bPlayerInRange && Player->IsInteractHeld())
	{
		Progress += DeltaSeconds;

		// using the objective makes noise that attracts enemies
		if (bMakeNoise)
		{
			NoiseTimer -= DeltaSeconds;

			if (NoiseTimer <= 0.0f)
			{
				UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), NoiseLoudness, Player, NoiseRange, FName("Interact"));
				NoiseTimer = NoiseInterval;
			}
		}

		if (Progress >= HoldTime)
		{
			CompleteObjective();
		}

	} else {

		// progress drains slowly when the player lets go
		Progress = FMath::Max(0.0f, Progress - (DeltaSeconds * 0.5f));
	}
}
