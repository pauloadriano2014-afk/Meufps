// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Mission01Builder.generated.h"

class USceneComponent;
class UStaticMesh;
class UMaterialInterface;
class AStaticMeshActor;
class AMissionObjective;
class AReachLocationObjective;
class AEliminateObjective;
class ASurviveObjective;
class AInteractObjective;
class ACheckpoint;
class AInfectedCharacter;

/** A weapon pickup type found in the level, remembered so the builder can place copies of it */
struct FM1PickupTemplate
{
	/** Pickup class to spawn */
	UClass* Class = nullptr;

	/** Which weapon the pickup grants */
	FDataTableRowHandle Handle;

	/** Pistol, Rifle, Grenade or Other */
	FString Kind;
};

/**
 *  Builds Mission 1 ("Estrada") in the level it is placed in
 *  Place it in a level whose name contains "Mission01" and press Build Mission in the Details panel
 *  It lays out the road, the abandoned cars, the ruined house and the gas station with simple blocks,
 *  then places weapon pickups, infected, raiders, checkpoints and every mission objective
 *  Press Build Mission again to rebuild from scratch. Clear Mission removes only what the builder made
 *  Everything it creates is a normal actor, so you can move or delete any of it by hand afterwards
 */
UCLASS(ClassGroup=(Mission))
class MEUFPS_API AMission01Builder : public AActor
{
	GENERATED_BODY()

	/** Root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

protected:

	/** Tag put on every actor the builder creates, so it can find them again to clear them */
	UPROPERTY(EditAnywhere, Category="Mission 01")
	FName BuiltTag = FName("M1Built");

	/** Seed for the random placement of the cars. Change it for a different arrangement */
	UPROPERTY(EditAnywhere, Category="Mission 01")
	int32 RandomSeed = 1337;

	/** Cube used to build everything. Loaded while building */
	UStaticMesh* CubeMesh = nullptr;

	/** Size of the cube mesh, used to scale it to the size wanted */
	FVector MeshSize = FVector(100.0f);

	/** Pivot offset of the cube mesh */
	FVector MeshOrigin = FVector::ZeroVector;

	/** Materials used while building */
	UMaterialInterface* GroundMaterial = nullptr;
	UMaterialInterface* RoadMaterial = nullptr;
	UMaterialInterface* PropMaterial = nullptr;
	UMaterialInterface* AccentMaterial = nullptr;

public:

	AMission01Builder();

	/** Builds the whole mission in this level, replacing anything built before */
	UFUNCTION(CallInEditor, Category="Mission 01")
	void BuildMission();

	/** Removes everything the builder created */
	UFUNCTION(CallInEditor, Category="Mission 01")
	void ClearMission();

protected:

	/** Loads the cube and materials. Returns false if the cube is missing */
	bool LoadBuildAssets();

	/** Removes what the level had before: the template props, enemies, pickups and old mission actors */
	void RemoveTemplateLeftovers(const AActor* ActorToKeep);

	/** Puts the player start at the beginning of the road and fits the navigation volume to the level */
	void PrepareStartAndNavigation();

	/** Builds the ground, walls, gate, road, cars and the other structures */
	void BuildWorld();

	/** Builds the abandoned cars and barriers along the road */
	void BuildRoadClutter();

	/** Builds the ruined house */
	void BuildRuinedHouse();

	/** Builds the gas station */
	void BuildGasStation();

	/** Places the weapon pickups, copying the pickup types found in the level */
	void PlacePickups(const TArray<FM1PickupTemplate>& Templates);

	/** Places infected and raiders */
	void PlaceEnemies();

	/** Places the mission manager, objectives and checkpoints */
	void PlaceMission();

	/** Spawns a scaled cube. Size is the final size in cm and Center is where its middle goes */
	AStaticMeshActor* SpawnBlock(const FVector& Center, const FVector& Size, UMaterialInterface* Material, const FString& Label, float Yaw = 0.0f);

	/** Spawns an actor of the class and marks it as built by this builder */
	AActor* SpawnTagged(UClass* Class, const FVector& Location, const FRotator& Rotation, const FString& Label);

	/** Marks an actor as built by this builder and names it in the Outliner */
	void Tag(AActor* Actor, const FString& Label);

	/** Spawns an infected of the given type */
	void SpawnInfected(uint8 Type, const FVector& Location, float Yaw, const FString& Label);

	/** Spawns a reach location objective */
	AReachLocationObjective* SpawnReach(int32 Order, const FText& Description, const FVector& Center, const FVector& Extent, const FString& Label);

	/** Sets the order and description of an objective */
	static void SetupObjective(AMissionObjective* Objective, int32 Order, const FText& Description);

	/** Spawns a checkpoint */
	void SpawnCheckpoint(const FVector& Location, const FString& Label);
};
