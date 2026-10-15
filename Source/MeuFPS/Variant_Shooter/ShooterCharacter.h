// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "MeuFPSCharacter.h"
#include "ShooterWeaponHolder.h"
#include "ShooterCharacter.generated.h"

class AShooterWeapon;
class UInputAction;
class UInputComponent;
class UInputMappingContext;
class UPawnNoiseEmitterComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBulletCountUpdatedDelegate, int32, MagazineSize, int32, Bullets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDamagedDelegate, float, LifePercent);

/**
 *  A player controllable first person shooter character
 *  Manages a weapon inventory through the IShooterWeaponHolder interface
 *  Manages health and death
 */
UCLASS(abstract)
class MEUFPS_API AShooterCharacter : public AMeuFPSCharacter, public IShooterWeaponHolder
{
	GENERATED_BODY()
	
	/** AI Noise emitter component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UPawnNoiseEmitterComponent* PawnNoiseEmitter;

protected:

	/** Fire weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* FireAction;

	/** Switch weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SwitchWeaponAction;

	/** Reload input action. If left empty, an action bound to the R key is created at runtime */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* ReloadAction;

	/** Aim down sights input action. If left empty, an action bound to the right mouse button is created at runtime */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* AimAction;

	/** Sprint input action. If left empty, an action bound to Left Shift is created at runtime */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SprintAction;

	/** Crouch input action. If left empty, an action bound to Left Ctrl is created at runtime */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* CrouchAction;

	/** Interact input action. If left empty, an action bound to the E key is created at runtime */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* InteractAction;

	/** Mapping context holding the keys that are bound from code instead of from assets */
	UPROPERTY(Transient)
	UInputMappingContext* RuntimeMappingContext;

	/** Sprint speed as a multiple of the normal walk speed */
	UPROPERTY(EditAnywhere, Category="Movement", meta = (ClampMin = 1, ClampMax = 3))
	float SprintSpeedMultiplier = 1.5f;

	/** Walk speed multiplier while aiming down sights */
	UPROPERTY(EditAnywhere, Category="Movement", meta = (ClampMin = 0.1, ClampMax = 1))
	float AimSpeedFactor = 0.6f;

	/** How far the eyes drop while crouched */
	UPROPERTY(EditAnywhere, Category="Movement", meta = (ClampMin = 0, ClampMax = 150, Units = "cm"))
	float CrouchEyeDrop = 70.0f;

	/** Camera field of view while aiming down sights. Lower means more zoom */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 20, ClampMax = 120, Units = "Degrees"))
	float AimFOV = 60.0f;

	/** How fast the camera zooms in and out when aiming */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 1))
	float AimInterpSpeed = 14.0f;

	/** If true, the character is aiming down sights */
	bool bIsAiming = false;

	/** If true, the sprint key is held */
	bool bWantsToSprint = false;

	/** If true, the interact key is held */
	bool bInteractHeld = false;

	/** Normal walk speed, read from the movement component at BeginPlay */
	float WalkSpeed = 0.0f;

	/** Normal camera field of view, read at BeginPlay */
	float DefaultFOV = 90.0f;

	/** Normal relative location of the first person mesh, read at BeginPlay */
	FVector DefaultFPMeshLocation = FVector::ZeroVector;

	/** Current vertical drop of the eyes, interpolated towards CrouchEyeDrop while crouched */
	float CurrentEyeDrop = 0.0f;

	/** Game time of the last shot that hit a character, used by the HUD hit marker */
	float LastHitConfirmTime = -100.0f;

	/** True if the last confirmed hit was a headshot */
	bool bLastHitWasHeadshot = false;

	/** Name of the first person mesh weapon socket */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName FirstPersonWeaponSocket = FName("HandGrip_R");

	/** Name of the third person mesh weapon socket */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName ThirdPersonWeaponSocket = FName("HandGrip_R");

	/** Max distance to use for aim traces */
	UPROPERTY(EditAnywhere, Category ="Aim", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxAimDistance = 10000.0f;

	/** Max HP this character can have */
	UPROPERTY(EditAnywhere, Category="Health")
	float MaxHP = 500.0f;

	/** Current HP remaining to this character */
	float CurrentHP = 0.0f;

	/** Team ID for this character*/
	UPROPERTY(EditAnywhere, Category="Team")
	uint8 TeamByte = 0;

	/** Actor tag to grant this character when it dies */
	UPROPERTY(EditAnywhere, Category="Team")
	FName DeathTag = FName("Dead");

	/** Tag to pass to weapons and projectiles to identify their AI perception noise as player-generated */
	UPROPERTY(EditAnywhere, Category="Tags")
	FName PlayerTag = FName("Player");

	/** List of weapons picked up by the character */
	TArray<AShooterWeapon*> OwnedWeapons;

	/** Weapon currently equipped and ready to shoot with */
	TObjectPtr<AShooterWeapon> CurrentWeapon;

	UPROPERTY(EditAnywhere, Category ="Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RespawnTime = 5.0f;

	FTimerHandle RespawnTimer;

public:

	/** Bullet count updated delegate */
	FBulletCountUpdatedDelegate OnBulletCountUpdated;

	/** Damaged delegate */
	FDamagedDelegate OnDamaged;

public:

	/** Constructor */
	AShooterCharacter();

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Per-frame update of the aim zoom, movement speed and crouch eye height */
	virtual void Tick(float DeltaSeconds) override;

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:

	/** Handle incoming damage */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:

	/** Handles aim inputs from either controls or UI interfaces */
	virtual void DoAim(float Yaw, float Pitch) override;

	/** Handles move inputs from either controls or UI interfaces */
	virtual void DoMove(float Right, float Forward)  override;

	/** Handles jump start inputs from either controls or UI interfaces */
	virtual void DoJumpStart()  override;

	/** Handles jump end inputs from either controls or UI interfaces */
	virtual void DoJumpEnd()  override;

	/** Handles start firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartFiring();

	/** Handles stop firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopFiring();

	/** Handles switch weapon input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSwitchWeapon();

	/** Handles reload input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoReload();

	/** Handles start aiming down sights input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartAim();

	/** Handles stop aiming down sights input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopAim();

	/** Handles start sprinting input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartSprint();

	/** Handles stop sprinting input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopSprint();

	/** Handles start interacting input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartInteract();

	/** Handles stop interacting input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopInteract();

	/** Handles start crouching input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartCrouch();

	/** Handles stop crouching input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopCrouch();

public:

	//~Begin IShooterWeaponHolder interface

	/** Attaches a weapon's meshes to the owner */
	virtual void AttachWeaponMeshes(AShooterWeapon* Weapon) override;

	/** Plays the firing montage for the weapon */
	virtual void PlayFiringMontage(UAnimMontage* Montage) override;

	/** Applies weapon recoil to the owner */
	virtual void AddWeaponRecoil(float Recoil) override;

	/** Updates the weapon's HUD with the current ammo count */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;

	/** Calculates and returns the aim location for the weapon */
	virtual FVector GetWeaponTargetLocation() override;

	/** Gives a weapon of this class to the owner */
	virtual void AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass) override;

	/** Activates the passed weapon */
	virtual void OnWeaponActivated(AShooterWeapon* Weapon) override;

	/** Deactivates the passed weapon */
	virtual void OnWeaponDeactivated(AShooterWeapon* Weapon) override;

	/** Notifies the owner that the weapon cooldown has expired and it's ready to shoot again */
	virtual void OnSemiWeaponRefire() override;

	/** Notifies the owner that the weapon's shot hit a character */
	virtual void OnHitConfirmed(bool bHeadshot) override;

	//~End IShooterWeaponHolder interface

protected:

	/** Returns true if the character already owns a weapon of the given class */
	AShooterWeapon* FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const;

	/** Creates an input action at runtime and binds it to a key, for inputs that have no asset yet */
	UInputAction* CreateRuntimeKeyAction(const TCHAR* ActionName, const FKey& Key);

	/** Called when this character's HP is depleted */
	void Die();

	/** Called to allow Blueprint code to react to this character's death */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "On Death"))
	void BP_OnDeath();

	/** Called from the respawn timer to destroy this character and force the PC to respawn */
	void OnRespawn();

public:

	/** Returns true if the character is dead */
	bool IsDead() const;

	/** Returns the weapon currently equipped, or null */
	AShooterWeapon* GetCurrentWeapon() const;

	/** Returns true while the interact key is held */
	bool IsInteractHeld() const { return bInteractHeld; }

	/** Returns the game time of the last shot that hit a character */
	float GetLastHitConfirmTime() const { return LastHitConfirmTime; }

	/** Returns true if the last confirmed hit was a headshot */
	bool WasLastHitHeadshot() const { return bLastHitWasHeadshot; }

	/** Sets the team ID for this character */
	void SetTeam(uint8 Team);
};
