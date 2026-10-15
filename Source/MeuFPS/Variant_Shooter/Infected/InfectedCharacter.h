// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InfectedCharacter.generated.h"

/** The kinds of infected. Each one has its own speed, health and damage */
UENUM(BlueprintType)
enum class EInfectedType : uint8
{
	/** Slow and sturdy. Wanders around until it notices the player */
	Walker,

	/** Fast and fragile. Hits often */
	Runner,

	/** Huge, very tough and hits hard */
	Brute
};

/**
 *  A melee-only enemy that hunts the player by sight and by sound
 *  Its behavior is in the Infected AI Controller. This class holds its body, health and attack
 *  Uses the template mannequin as a placeholder. Swap the mesh and animation for proper infected art when available
 */
UCLASS()
class MEUFPS_API AInfectedCharacter : public ACharacter
{
	GENERATED_BODY()

	friend class AMission01Builder;

protected:

	/** Which kind of infected this is. Use SetInfectedType to also apply the matching stats */
	UPROPERTY(EditAnywhere, Category="Infected")
	EInfectedType InfectedType = EInfectedType::Walker;

	/** Health. Dies when it reaches zero */
	UPROPERTY(EditAnywhere, Category="Infected|Stats", meta = (ClampMin = 1))
	float MaxHP = 80.0f;

	/** Movement speed when going after something */
	UPROPERTY(EditAnywhere, Category="Infected|Stats", meta = (ClampMin = 0))
	float MoveSpeed = 130.0f;

	/** Damage of each hit */
	UPROPERTY(EditAnywhere, Category="Infected|Stats", meta = (ClampMin = 0))
	float AttackDamage = 15.0f;

	/** Distance from which it can hit the target */
	UPROPERTY(EditAnywhere, Category="Infected|Stats", meta = (ClampMin = 0, Units = "cm"))
	float AttackRange = 130.0f;

	/** Time between hits */
	UPROPERTY(EditAnywhere, Category="Infected|Stats", meta = (ClampMin = 0.1, Units = "s"))
	float AttackCooldown = 1.2f;

	/** Time between starting an attack and the damage landing. The player can step away in this time */
	UPROPERTY(EditAnywhere, Category="Infected|Stats", meta = (ClampMin = 0, Units = "s"))
	float AttackWindup = 0.35f;

	/** If true, wanders around its spawn point while it has nothing to do */
	UPROPERTY(EditAnywhere, Category="Infected|Stats")
	bool bWanders = true;

	/** Team of this character. Deaths of team 1 count towards kill objectives */
	UPROPERTY(EditAnywhere, Category="Infected|Team")
	uint8 TeamByte = 1;

	/** Actor tag granted on death */
	UPROPERTY(EditAnywhere, Category="Infected|Team")
	FName DeathTag = FName("Dead");

	/** How long the body stays after death */
	UPROPERTY(EditAnywhere, Category="Infected", meta = (ClampMin = 0, Units = "s"))
	float BodyLifeSpan = 10.0f;

	/** Current health */
	float CurrentHP = 0.0f;

	/** True once dead */
	bool bIsDead = false;

	/** Game time of the last attack */
	float LastAttackTime = -100.0f;

	/** Who the current attack is aimed at */
	TWeakObjectPtr<AActor> AttackTarget;

	/** Timer for the attack windup */
	FTimerHandle AttackTimer;

public:

	AInfectedCharacter();

	/** Sets the kind of infected and applies its stats, speed and size */
	void SetInfectedType(EInfectedType NewType);

	/** Handle incoming damage */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Starts an attack on the target if the cooldown allows it. Returns true if the attack started */
	bool TryAttack(AActor* Target);

	/** Returns true once dead */
	bool IsDead() const { return bIsDead; }

	/** Returns the distance from which it can hit */
	float GetAttackRange() const { return AttackRange; }

	/** Returns true if it should wander while idle */
	bool ShouldWander() const { return bWanders; }

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Applies the damage of an attack once the windup is over */
	void DealAttackDamage();

	/** Handles death: ragdoll, score and cleanup */
	void Die();

	/** Called when an attack starts. Implement in Blueprint for sounds and animations */
	UFUNCTION(BlueprintImplementableEvent, Category="Infected", meta = (DisplayName = "On Attack"))
	void BP_OnAttack();

	/** Called when it dies. Implement in Blueprint for sounds and effects */
	UFUNCTION(BlueprintImplementableEvent, Category="Infected", meta = (DisplayName = "On Death"))
	void BP_OnDeath();
};
