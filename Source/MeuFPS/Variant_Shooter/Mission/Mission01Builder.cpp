// Copyright Epic Games, Inc. All Rights Reserved.


#include "Mission01Builder.h"
#include "MeuFPS.h"
#include "MissionManager.h"
#include "MissionObjective.h"
#include "ReachLocationObjective.h"
#include "EliminateObjective.h"
#include "SurviveObjective.h"
#include "InteractObjective.h"
#include "Checkpoint.h"
#include "Variant_Shooter/Infected/InfectedCharacter.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Variant_Shooter/AI/ShooterNPCSpawner.h"
#include "Variant_Shooter/Weapons/ShooterPickup.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/Brush.h"
#include "Engine/Light.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Info.h"
#include "GameFramework/PlayerStart.h"
#include "Materials/MaterialInterface.h"

namespace
{
	// the playable area, in cm. The road runs along X from the Abrigo gate to the exit
	constexpr float AreaMinX = -1500.0f;
	constexpr float AreaMaxX = 26500.0f;
	constexpr float AreaHalfWidth = 4500.0f;
	constexpr float GateX = 1200.0f;

	// where the gas station pump is. The player fills the can here
	const FVector PumpLocation(21500.0f, 0.0f, 100.0f);

	FText AsText(const TCHAR* Text)
	{
		return FText::FromString(Text);
	}
}

AMission01Builder::AMission01Builder()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AMission01Builder::BuildMission()
{
	UWorld* World = GetWorld();

	if (!World || World->IsGameWorld())
	{
		UE_LOG(LogMeuFPS, Warning, TEXT("Mission01Builder: the mission can only be built in the editor, outside of Play."));
		return;
	}

	// building removes the props that are already in the level, so only allow it in a dedicated level
	if (!World->GetOutermost()->GetName().Contains(TEXT("Mission01")))
	{
		UE_LOG(LogMeuFPS, Error, TEXT("Mission01Builder: this level is not called Mission01. Use File > Save Current Level As to make a copy named Lvl_Mission01 and build there. Nothing was changed."));
		return;
	}

	if (!LoadBuildAssets())
	{
		return;
	}

	// remember the weapon pickup types in the level before the template props are removed
	TArray<FM1PickupTemplate> Templates;

	for (TActorIterator<AShooterPickup> It(World); It; ++It)
	{
		const AShooterPickup* Pickup = *It;

		FString Kind = TEXT("Other");

		if (const FWeaponTableRow* Row = Pickup->WeaponType.GetRow<FWeaponTableRow>(FString()))
		{
			const FString WeaponName = Row->WeaponToSpawn ? Row->WeaponToSpawn->GetName() : FString();

			if (WeaponName.Contains(TEXT("Pistol")))
			{
				Kind = TEXT("Pistol");

			} else if (WeaponName.Contains(TEXT("Rifle"))) {

				Kind = TEXT("Rifle");

			} else if (WeaponName.Contains(TEXT("Grenade"))) {

				Kind = TEXT("Grenade");
			}
		}

		// one template per kind of weapon is enough
		const bool bAlreadyKnown = Templates.ContainsByPredicate([&Kind](const FM1PickupTemplate& Known)
		{
			return Known.Kind == Kind;
		});

		if (!bAlreadyKnown)
		{
			FM1PickupTemplate NewTemplate;
			NewTemplate.Class = Pickup->GetClass();
			NewTemplate.Handle = Pickup->WeaponType;
			NewTemplate.Kind = Kind;
			Templates.Add(NewTemplate);
		}
	}

	// start from a clean level
	ClearMission();
	RemoveTemplateLeftovers(this);

	PrepareStartAndNavigation();
	BuildWorld();
	PlacePickups(Templates);
	PlaceEnemies();
	PlaceMission();

	World->MarkPackageDirty();

	UE_LOG(LogMeuFPS, Display, TEXT("Mission01Builder: Mission 1 built. Use Build > Build Paths so the enemies can navigate, then save the level."));
}

void AMission01Builder::ClearMission()
{
	UWorld* World = GetWorld();

	if (!World || World->IsGameWorld())
	{
		return;
	}

	// collect first, then destroy, so we don't change the world while iterating it
	TArray<AActor*> BuiltActors;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		if (Actor != this && Actor->Tags.Contains(BuiltTag))
		{
			BuiltActors.Add(Actor);
		}
	}

	for (AActor* Actor : BuiltActors)
	{
		World->DestroyActor(Actor);
	}
}

bool AMission01Builder::LoadBuildAssets()
{
	CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube"));

	if (!CubeMesh)
	{
		UE_LOG(LogMeuFPS, Error, TEXT("Mission01Builder: could not load /Game/LevelPrototyping/Meshes/SM_Cube. Nothing was changed."));
		return false;
	}

	GroundMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray.MI_PrototypeGrid_Gray"));
	RoadMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_TopDark.MI_PrototypeGrid_TopDark"));
	PropMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray_02.MI_PrototypeGrid_Gray_02"));
	AccentMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/LevelPrototyping/Materials/MI_DefaultColorway.MI_DefaultColorway"));

	// the cube may not be exactly 1 m, and its pivot may not be in the middle
	const FBoxSphereBounds Bounds = CubeMesh->GetBounds();
	MeshSize = Bounds.BoxExtent * 2.0f;
	MeshOrigin = Bounds.Origin;

	if (MeshSize.GetMin() < 1.0f)
	{
		UE_LOG(LogMeuFPS, Error, TEXT("Mission01Builder: the cube mesh has no size. Nothing was changed."));
		return false;
	}

	return true;
}

void AMission01Builder::RemoveTemplateLeftovers(const AActor* ActorToKeep)
{
	UWorld* World = GetWorld();

	TArray<AActor*> ToDestroy;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		if (Actor == ActorToKeep)
		{
			continue;
		}

		// things that belong to the template level or to an earlier try at the mission
		const bool bRemove = Actor->IsA<AStaticMeshActor>()
			|| Actor->IsA<AShooterNPC>()
			|| Actor->IsA<AShooterPickup>()
			|| Actor->IsA<AShooterNPCSpawner>()
			|| Actor->IsA<AInfectedCharacter>()
			|| Actor->IsA<AMissionManager>()
			|| Actor->IsA<AMissionObjective>()
			|| Actor->IsA<ACheckpoint>();

		if (bRemove)
		{
			ToDestroy.Add(Actor);
			continue;
		}

		// lights, sky, fog, volumes and the player start stay
		if (Actor->IsA<AInfo>() || Actor->IsA<ALight>() || Actor->IsA<ABrush>() || Actor->IsA<APlayerStart>())
		{
			continue;
		}

		// any other Blueprint actor is a prop of the template level, such as the wobble targets
		if (Cast<UBlueprintGeneratedClass>(Actor->GetClass()) != nullptr)
		{
			ToDestroy.Add(Actor);
		}
	}

	for (AActor* Actor : ToDestroy)
	{
		World->DestroyActor(Actor);
	}
}

void AMission01Builder::PrepareStartAndNavigation()
{
	UWorld* World = GetWorld();

	// keep one player start and put it at the beginning of the road
	TArray<APlayerStart*> Starts;

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		Starts.Add(*It);
	}

	APlayerStart* Start = Starts.Num() > 0 ? Starts[0] : World->SpawnActor<APlayerStart>(FVector(300.0f, 0.0f, 110.0f), FRotator::ZeroRotator);

	for (int32 i = 1; i < Starts.Num(); ++i)
	{
		World->DestroyActor(Starts[i]);
	}

	if (Start)
	{
		Start->SetActorLocationAndRotation(FVector(300.0f, 0.0f, 110.0f), FRotator::ZeroRotator);
	}

	// stretch the navigation volume over the whole level so the enemies can find their way
	bool bFoundNavVolume = false;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		if (Actor->GetClass()->GetName() != TEXT("NavMeshBoundsVolume"))
		{
			continue;
		}

		bFoundNavVolume = true;

		FVector Origin;
		FVector Extent;
		Actor->GetActorBounds(false, Origin, Extent);

		if (Extent.GetMin() > 1.0f)
		{
			const FVector DesiredExtent(14500.0f, 5000.0f, 800.0f);

			Actor->SetActorScale3D(Actor->GetActorScale3D() * (DesiredExtent / Extent));
			Actor->SetActorLocation(FVector(12500.0f, 0.0f, 300.0f));
		}

		break;
	}

	if (!bFoundNavVolume)
	{
		UE_LOG(LogMeuFPS, Warning, TEXT("Mission01Builder: no NavMeshBoundsVolume in the level. Add one from Place Actors and scale it over the level, or the infected and raiders can't walk."));
	}
}

void AMission01Builder::BuildWorld()
{
	const float Length = AreaMaxX - AreaMinX;
	const float MidX = (AreaMinX + AreaMaxX) * 0.5f;

	// ground and road
	SpawnBlock(FVector(MidX, 0.0f, -50.0f), FVector(Length, AreaHalfWidth * 2.0f, 100.0f), GroundMaterial, TEXT("Ground"));
	SpawnBlock(FVector(12000.0f, 0.0f, 5.0f), FVector(25000.0f, 1100.0f, 10.0f), RoadMaterial, TEXT("Road"));

	// walls around the whole area
	const float WallHeight = 700.0f;
	const float WallThickness = 100.0f;

	SpawnBlock(FVector(MidX, -AreaHalfWidth - (WallThickness * 0.5f), WallHeight * 0.5f), FVector(Length + (WallThickness * 2.0f), WallThickness, WallHeight), GroundMaterial, TEXT("Wall_South"));
	SpawnBlock(FVector(MidX, AreaHalfWidth + (WallThickness * 0.5f), WallHeight * 0.5f), FVector(Length + (WallThickness * 2.0f), WallThickness, WallHeight), GroundMaterial, TEXT("Wall_North"));
	SpawnBlock(FVector(AreaMinX - (WallThickness * 0.5f), 0.0f, WallHeight * 0.5f), FVector(WallThickness, AreaHalfWidth * 2.0f, WallHeight), GroundMaterial, TEXT("Wall_West"));
	SpawnBlock(FVector(AreaMaxX + (WallThickness * 0.5f), 0.0f, WallHeight * 0.5f), FVector(WallThickness, AreaHalfWidth * 2.0f, WallHeight), GroundMaterial, TEXT("Wall_East"));

	// the Abrigo gate: two pillars and a lintel, with walls that funnel the player through it
	SpawnBlock(FVector(GateX, -850.0f, 300.0f), FVector(200.0f, 200.0f, 600.0f), PropMaterial, TEXT("Gate_PillarLeft"));
	SpawnBlock(FVector(GateX, 850.0f, 300.0f), FVector(200.0f, 200.0f, 600.0f), PropMaterial, TEXT("Gate_PillarRight"));
	SpawnBlock(FVector(GateX, 0.0f, 660.0f), FVector(200.0f, 1900.0f, 120.0f), PropMaterial, TEXT("Gate_Lintel"));

	const float FunnelLength = AreaHalfWidth - 950.0f;
	const float FunnelCenterY = 950.0f + (FunnelLength * 0.5f);

	SpawnBlock(FVector(GateX, -FunnelCenterY, 225.0f), FVector(200.0f, FunnelLength, 450.0f), PropMaterial, TEXT("Gate_WallLeft"));
	SpawnBlock(FVector(GateX, FunnelCenterY, 225.0f), FVector(200.0f, FunnelLength, 450.0f), PropMaterial, TEXT("Gate_WallRight"));

	BuildRoadClutter();
	BuildRuinedHouse();
	BuildGasStation();
}

void AMission01Builder::BuildRoadClutter()
{
	FRandomStream Random(RandomSeed);

	// abandoned cars along the road. Each is a body with a cabin on top, at a random angle, for cover
	for (int32 i = 0; i < 14; ++i)
	{
		const float X = 3400.0f + (i * 440.0f) + Random.FRandRange(-120.0f, 120.0f);
		const float Y = Random.FRandRange(-1700.0f, 1700.0f);
		const float Yaw = Random.FRandRange(-40.0f, 40.0f) + (Random.RandRange(0, 2) == 0 ? 90.0f : 0.0f);
		const FRotator Rotation(0.0f, Yaw, 0.0f);

		SpawnBlock(FVector(X, Y, 55.0f), FVector(450.0f, 200.0f, 110.0f), PropMaterial, FString::Printf(TEXT("Car_%d_Body"), i), Yaw);

		const FVector CabinCenter = FVector(X, Y, 150.0f) + Rotation.RotateVector(FVector(-30.0f, 0.0f, 0.0f));
		SpawnBlock(CabinCenter, FVector(230.0f, 180.0f, 80.0f), PropMaterial, FString::Printf(TEXT("Car_%d_Cabin"), i), Yaw);
	}

	// concrete barriers scattered over the first half of the road
	for (int32 i = 0; i < 8; ++i)
	{
		const float X = 3000.0f + (i * 800.0f) + Random.FRandRange(-200.0f, 200.0f);
		const float Y = Random.FRandRange(-2200.0f, 2200.0f);
		const float Yaw = Random.FRandRange(-30.0f, 30.0f);

		SpawnBlock(FVector(X, Y, 50.0f), FVector(300.0f, 80.0f, 100.0f), GroundMaterial, FString::Printf(TEXT("Barrier_%d"), i), Yaw);
	}

	// the ambush spot: a bus and a truck across the road, and low barricades
	SpawnBlock(FVector(11400.0f, -350.0f, 150.0f), FVector(1200.0f, 260.0f, 300.0f), AccentMaterial, TEXT("Bus"), 14.0f);
	SpawnBlock(FVector(12700.0f, 750.0f, 175.0f), FVector(900.0f, 250.0f, 350.0f), PropMaterial, TEXT("Truck"), -18.0f);
	SpawnBlock(FVector(11900.0f, 1800.0f, 60.0f), FVector(500.0f, 80.0f, 120.0f), GroundMaterial, TEXT("Barricade_A"), 10.0f);
	SpawnBlock(FVector(12300.0f, -1800.0f, 60.0f), FVector(500.0f, 80.0f, 120.0f), GroundMaterial, TEXT("Barricade_B"), -8.0f);
	SpawnBlock(FVector(13400.0f, -600.0f, 60.0f), FVector(400.0f, 80.0f, 120.0f), GroundMaterial, TEXT("Barricade_C"), 25.0f);
}

void AMission01Builder::BuildRuinedHouse()
{
	// a house with broken walls on the right side of the road. A place to loot and to flank from
	SpawnBlock(FVector(16400.0f, 3400.0f, 200.0f), FVector(1600.0f, 100.0f, 400.0f), PropMaterial, TEXT("House_WallNorth"));
	SpawnBlock(FVector(15850.0f, 1600.0f, 200.0f), FVector(700.0f, 100.0f, 400.0f), PropMaterial, TEXT("House_WallSouthA"));
	SpawnBlock(FVector(17000.0f, 1600.0f, 200.0f), FVector(400.0f, 100.0f, 400.0f), PropMaterial, TEXT("House_WallSouthB"));
	SpawnBlock(FVector(15600.0f, 2500.0f, 120.0f), FVector(100.0f, 1800.0f, 240.0f), PropMaterial, TEXT("House_WallWest"));
	SpawnBlock(FVector(17200.0f, 2800.0f, 200.0f), FVector(100.0f, 1200.0f, 400.0f), PropMaterial, TEXT("House_WallEast"));

	// furniture and rubble inside
	SpawnBlock(FVector(16400.0f, 2900.0f, 50.0f), FVector(300.0f, 100.0f, 100.0f), GroundMaterial, TEXT("House_Table"));
	SpawnBlock(FVector(16900.0f, 2400.0f, 60.0f), FVector(150.0f, 150.0f, 120.0f), GroundMaterial, TEXT("House_Rubble"), 20.0f);
}

void AMission01Builder::BuildGasStation()
{
	// the forecourt
	SpawnBlock(FVector(21750.0f, 0.0f, 10.0f), FVector(5500.0f, 5200.0f, 20.0f), RoadMaterial, TEXT("Forecourt"));

	// low walls along both sides, with gaps as entrances
	const float WallY = 2800.0f;
	const float Segments[3][2] = { { 19200.0f, 20800.0f }, { 21400.0f, 23000.0f }, { 23600.0f, 24400.0f } };

	for (int32 i = 0; i < 3; ++i)
	{
		const float CenterX = (Segments[i][0] + Segments[i][1]) * 0.5f;
		const float SegmentLength = Segments[i][1] - Segments[i][0];

		SpawnBlock(FVector(CenterX, -WallY, 65.0f), FVector(SegmentLength, 60.0f, 130.0f), GroundMaterial, FString::Printf(TEXT("StationWall_South_%d"), i));
		SpawnBlock(FVector(CenterX, WallY, 65.0f), FVector(SegmentLength, 60.0f, 130.0f), GroundMaterial, FString::Printf(TEXT("StationWall_North_%d"), i));
	}

	// cover right at the entrance
	SpawnBlock(FVector(19500.0f, -900.0f, 60.0f), FVector(300.0f, 300.0f, 120.0f), PropMaterial, TEXT("Entrance_CoverA"));
	SpawnBlock(FVector(19500.0f, 900.0f, 60.0f), FVector(300.0f, 300.0f, 120.0f), PropMaterial, TEXT("Entrance_CoverB"));

	// the canopy over the pumps
	SpawnBlock(FVector(21500.0f, 0.0f, 520.0f), FVector(2000.0f, 2800.0f, 40.0f), AccentMaterial, TEXT("Canopy_Roof"));

	for (int32 i = 0; i < 4; ++i)
	{
		const float PillarX = 21500.0f + ((i % 2 == 0) ? -800.0f : 800.0f);
		const float PillarY = (i < 2) ? -1100.0f : 1100.0f;

		SpawnBlock(FVector(PillarX, PillarY, 260.0f), FVector(100.0f, 100.0f, 520.0f), PropMaterial, FString::Printf(TEXT("Canopy_Pillar_%d"), i));
	}

	// the pumps. The objective sits in the middle of them
	SpawnBlock(FVector(21150.0f, -400.0f, 80.0f), FVector(90.0f, 90.0f, 160.0f), AccentMaterial, TEXT("Pump_A"));
	SpawnBlock(FVector(21150.0f, 400.0f, 80.0f), FVector(90.0f, 90.0f, 160.0f), AccentMaterial, TEXT("Pump_B"));
	SpawnBlock(FVector(21850.0f, -400.0f, 80.0f), FVector(90.0f, 90.0f, 160.0f), AccentMaterial, TEXT("Pump_C"));
	SpawnBlock(FVector(21850.0f, 400.0f, 80.0f), FVector(90.0f, 90.0f, 160.0f), AccentMaterial, TEXT("Pump_D"));

	// the store, open at the front through a doorway
	SpawnBlock(FVector(24400.0f, 0.0f, 200.0f), FVector(100.0f, 1800.0f, 400.0f), PropMaterial, TEXT("Store_WallBack"));
	SpawnBlock(FVector(23900.0f, -900.0f, 200.0f), FVector(1000.0f, 100.0f, 400.0f), PropMaterial, TEXT("Store_WallSouth"));
	SpawnBlock(FVector(23900.0f, 900.0f, 200.0f), FVector(1000.0f, 100.0f, 400.0f), PropMaterial, TEXT("Store_WallNorth"));
	SpawnBlock(FVector(23400.0f, -600.0f, 200.0f), FVector(100.0f, 600.0f, 400.0f), PropMaterial, TEXT("Store_FrontLeft"));
	SpawnBlock(FVector(23400.0f, 600.0f, 200.0f), FVector(100.0f, 600.0f, 400.0f), PropMaterial, TEXT("Store_FrontRight"));

	// shelves and a counter inside
	SpawnBlock(FVector(24100.0f, -500.0f, 100.0f), FVector(100.0f, 500.0f, 200.0f), GroundMaterial, TEXT("Store_ShelfA"));
	SpawnBlock(FVector(24100.0f, 500.0f, 100.0f), FVector(100.0f, 500.0f, 200.0f), GroundMaterial, TEXT("Store_ShelfB"));
	SpawnBlock(FVector(23750.0f, 0.0f, 60.0f), FVector(80.0f, 500.0f, 120.0f), GroundMaterial, TEXT("Store_Counter"));
}

void AMission01Builder::PlacePickups(const TArray<FM1PickupTemplate>& Templates)
{
	if (Templates.Num() == 0)
	{
		UE_LOG(LogMeuFPS, Warning, TEXT("Mission01Builder: found no weapon pickups in the level to copy. Run Build Mission in a copy of Lvl_Shooter, which has pickups, or place weapon pickups by hand."));
		return;
	}

	// find the template for a kind of weapon, falling back to whatever exists
	auto FindTemplate = [&Templates](const TCHAR* Kind) -> const FM1PickupTemplate&
	{
		for (const FM1PickupTemplate& Template : Templates)
		{
			if (Template.Kind == Kind)
			{
				return Template;
			}
		}

		return Templates[0];
	};

	auto PlacePickup = [this, &FindTemplate](const TCHAR* Kind, float X, float Y, const FString& Label)
	{
		const FM1PickupTemplate& Template = FindTemplate(Kind);

		AShooterPickup* Pickup = Cast<AShooterPickup>(SpawnTagged(Template.Class, FVector(X, Y, 0.0f), FRotator::ZeroRotator, Label));

		if (Pickup)
		{
			Pickup->WeaponType = Template.Handle;

			// ammo is scarce: a pickup does not come back
			Pickup->RespawnTime = 100000.0f;

			// show the right weapon mesh
			Pickup->RerunConstructionScripts();
		}
	};

	// the pistol is the starting weapon. More ammo for it is rare
	PlacePickup(TEXT("Pistol"), 500.0f, 200.0f, TEXT("Pistol_Start"));
	PlacePickup(TEXT("Pistol"), 5600.0f, -900.0f, TEXT("Ammo_Pistol_Road"));
	PlacePickup(TEXT("Pistol"), 13800.0f, 600.0f, TEXT("Ammo_Pistol_AfterAmbush"));
	PlacePickup(TEXT("Pistol"), 19900.0f, 0.0f, TEXT("Ammo_Pistol_Station"));

	// the rifle is found on the road, and its ammo is rarer still
	PlacePickup(TEXT("Rifle"), 9000.0f, 1500.0f, TEXT("Rifle_Road"));
	PlacePickup(TEXT("Rifle"), 16400.0f, 2500.0f, TEXT("Ammo_Rifle_House"));
	PlacePickup(TEXT("Rifle"), 24000.0f, -600.0f, TEXT("Ammo_Rifle_Store"));
}

void AMission01Builder::PlaceEnemies()
{
	// infected types are 0 Walker, 1 Runner and 2 Brute
	const uint8 Walker = 0;
	const uint8 Runner = 1;
	const uint8 Brute = 2;

	// walkers drifting along the road
	SpawnInfected(Walker, FVector(5200.0f, 900.0f, 0.0f), 200.0f, TEXT("Infected_Road_1"));
	SpawnInfected(Walker, FVector(7000.0f, -1300.0f, 0.0f), 160.0f, TEXT("Infected_Road_2"));
	SpawnInfected(Walker, FVector(8800.0f, 1500.0f, 0.0f), 220.0f, TEXT("Infected_Road_3"));
	SpawnInfected(Walker, FVector(9800.0f, -800.0f, 0.0f), 180.0f, TEXT("Infected_Road_4"));

	// the ambush: two runners waiting behind the bus and the truck
	SpawnInfected(Runner, FVector(11900.0f, -1000.0f, 0.0f), 180.0f, TEXT("Infected_Ambush_1"));
	SpawnInfected(Runner, FVector(12800.0f, 1400.0f, 0.0f), 180.0f, TEXT("Infected_Ambush_2"));

	// two near the ruined house
	SpawnInfected(Walker, FVector(16200.0f, 2500.0f, 0.0f), 270.0f, TEXT("Infected_House_1"));
	SpawnInfected(Walker, FVector(17500.0f, 1000.0f, 0.0f), 200.0f, TEXT("Infected_House_2"));

	// the horde, waiting around the gas station. The pump noise wakes them up
	const FVector2D HordeWalkers[] =
	{
		FVector2D(20000.0f, 3600.0f), FVector2D(21000.0f, -3800.0f), FVector2D(22000.0f, 3500.0f), FVector2D(23000.0f, -3600.0f),
		FVector2D(24000.0f, 3700.0f), FVector2D(24500.0f, -3500.0f), FVector2D(25200.0f, 2500.0f), FVector2D(25200.0f, -2500.0f)
	};

	for (int32 i = 0; i < static_cast<int32>(UE_ARRAY_COUNT(HordeWalkers)); ++i)
	{
		SpawnInfected(Walker, FVector(HordeWalkers[i].X, HordeWalkers[i].Y, 0.0f), 180.0f, FString::Printf(TEXT("Horde_Walker_%d"), i));
	}

	SpawnInfected(Runner, FVector(23500.0f, 3900.0f, 0.0f), 270.0f, TEXT("Horde_Runner_1"));
	SpawnInfected(Runner, FVector(22500.0f, -3900.0f, 0.0f), 90.0f, TEXT("Horde_Runner_2"));
	SpawnInfected(Runner, FVector(25500.0f, 0.0f, 0.0f), 180.0f, TEXT("Horde_Runner_3"));
	SpawnInfected(Brute, FVector(25000.0f, 3000.0f, 0.0f), 180.0f, TEXT("Horde_Brute"));

	// the raiders holding the station, behind the low walls and inside the store
	UClass* RaiderClass = LoadClass<AShooterNPC>(nullptr, TEXT("/Game/Variant_Shooter/Blueprints/AI/BP_ShooterNPC.BP_ShooterNPC_C"));

	if (!RaiderClass)
	{
		UE_LOG(LogMeuFPS, Warning, TEXT("Mission01Builder: could not load BP_ShooterNPC, so no raiders were placed."));
		return;
	}

	const FVector RaiderLocations[] =
	{
		FVector(20100.0f, -2500.0f, 100.0f), FVector(20100.0f, 2500.0f, 100.0f),
		FVector(22700.0f, -2400.0f, 100.0f), FVector(22700.0f, 2400.0f, 100.0f),
		FVector(24000.0f, 0.0f, 100.0f)
	};

	for (int32 i = 0; i < static_cast<int32>(UE_ARRAY_COUNT(RaiderLocations)); ++i)
	{
		SpawnTagged(RaiderClass, RaiderLocations[i], FRotator(0.0f, 180.0f, 0.0f), FString::Printf(TEXT("Raider_%d"), i));
	}
}

void AMission01Builder::PlaceMission()
{
	// the manager holds the story text
	AMissionManager* Manager = Cast<AMissionManager>(SpawnTagged(AMissionManager::StaticClass(), FVector(0.0f, 0.0f, 150.0f), FRotator::ZeroRotator, TEXT("MissionManager")));

	if (Manager)
	{
		Manager->MissionTitle = AsText(TEXT("Estrada"));
		Manager->Briefing = AsText(TEXT("O gerador do Abrigo morre em dois dias.\nO único combustível por perto está no posto da estrada, e os saqueadores sabem disso.\nVá, pegue o que der e volte vivo."));
		Manager->CompletionText = AsText(TEXT("O galão está cheio. Ao longe, a fumaça de outro grupo: voltar com pouco, ou arriscar mais. Paul volta com pouco."));
		Manager->BriefingDuration = 10.0f;
	}

	// 0: step out of the gate
	SpawnReach(0, AsText(TEXT("Saia do Abrigo")), FVector(2200.0f, 0.0f, 150.0f), FVector(200.0f, 1000.0f, 200.0f), TEXT("Objective_0_LeaveGate"));

	// 1: clear the road of infected
	AEliminateObjective* ClearRoad = Cast<AEliminateObjective>(SpawnTagged(AEliminateObjective::StaticClass(), FVector(6000.0f, 0.0f, 150.0f), FRotator::ZeroRotator, TEXT("Objective_1_ClearRoad")));

	if (ClearRoad)
	{
		SetupObjective(ClearRoad, 1, AsText(TEXT("Abra caminho pela estrada")));
		ClearRoad->KillsRequired = 4;
	}

	// 2: reach the gas station
	SpawnReach(2, AsText(TEXT("Chegue ao posto de gasolina")), FVector(18800.0f, 0.0f, 150.0f), FVector(250.0f, 1500.0f, 200.0f), TEXT("Objective_2_ReachStation"));

	// 3: take out the raiders
	AEliminateObjective* ClearStation = Cast<AEliminateObjective>(SpawnTagged(AEliminateObjective::StaticClass(), FVector(20000.0f, 0.0f, 150.0f), FRotator::ZeroRotator, TEXT("Objective_3_ClearStation")));

	if (ClearStation)
	{
		SetupObjective(ClearStation, 3, AsText(TEXT("Limpe o posto")));
		ClearStation->KillsRequired = 4;
	}

	// 4: fill the can at the pump. The noise wakes the horde
	AInteractObjective* FillCan = Cast<AInteractObjective>(SpawnTagged(AInteractObjective::StaticClass(), PumpLocation, FRotator::ZeroRotator, TEXT("Objective_4_FillCan")));

	if (FillCan)
	{
		SetupObjective(FillCan, 4, AsText(TEXT("Encha o galão")));
		FillCan->Prompt = AsText(TEXT("Segure E para encher o galão"));
		FillCan->HoldTime = 12.0f;
		FillCan->bMakeNoise = true;
		FillCan->NoiseInterval = 1.5f;
		FillCan->NoiseRange = 4500.0f;

		if (USphereComponent* Range = FillCan->FindComponentByClass<USphereComponent>())
		{
			Range->SetSphereRadius(450.0f);
		}
	}

	// 5: hold out against the horde
	ASurviveObjective* HoldOut = Cast<ASurviveObjective>(SpawnTagged(ASurviveObjective::StaticClass(), FVector(21500.0f, 0.0f, 150.0f), FRotator::ZeroRotator, TEXT("Objective_5_HoldOut")));

	if (HoldOut)
	{
		SetupObjective(HoldOut, 5, AsText(TEXT("Resista à horda")));
		HoldOut->Duration = 45.0f;
	}

	// 6: leave by the east road
	SpawnReach(6, AsText(TEXT("Saia pela estrada a leste")), FVector(25900.0f, 0.0f, 150.0f), FVector(250.0f, 1800.0f, 200.0f), TEXT("Objective_6_Exit"));

	// checkpoints after the ambush and at the gas station entrance
	SpawnCheckpoint(FVector(14300.0f, 0.0f, 100.0f), TEXT("Checkpoint_1_AfterAmbush"));
	SpawnCheckpoint(FVector(19300.0f, 0.0f, 100.0f), TEXT("Checkpoint_2_Station"));
}

AStaticMeshActor* AMission01Builder::SpawnBlock(const FVector& Center, const FVector& Size, UMaterialInterface* Material, const FString& Label, float Yaw)
{
	const FVector Scale = Size / MeshSize;
	const FRotator Rotation(0.0f, Yaw, 0.0f);

	// the mesh pivot may not be at its middle, so move the actor to compensate
	const FVector PivotOffset = Rotation.RotateVector(MeshOrigin * Scale);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStaticMeshActor* Block = GetWorld()->SpawnActor<AStaticMeshActor>(Center - PivotOffset, Rotation, SpawnParams);

	if (!Block)
	{
		return nullptr;
	}

	// the mesh can only be changed on a movable component. Make it static again afterwards
	UStaticMeshComponent* MeshComponent = Block->GetStaticMeshComponent();
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetStaticMesh(CubeMesh);

	if (Material)
	{
		MeshComponent->SetMaterial(0, Material);
	}

	Block->SetActorScale3D(Scale);
	MeshComponent->SetMobility(EComponentMobility::Static);

	Tag(Block, Label);

	return Block;
}

AActor* AMission01Builder::SpawnTagged(UClass* Class, const FVector& Location, const FRotator& Rotation, const FString& Label)
{
	if (!Class)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* Actor = GetWorld()->SpawnActor(Class, &Location, &Rotation, SpawnParams);

	if (Actor)
	{
		Tag(Actor, Label);
	}

	return Actor;
}

void AMission01Builder::Tag(AActor* Actor, const FString& Label)
{
	Actor->Tags.AddUnique(BuiltTag);

#if WITH_EDITOR
	Actor->SetActorLabel(Label);
	Actor->SetFolderPath(FName("Mission01"));
#endif
}

void AMission01Builder::SpawnInfected(uint8 Type, const FVector& Location, float Yaw, const FString& Label)
{
	// a brute is bigger, so spawn it higher to keep it out of the floor
	const EInfectedType InfectedType = static_cast<EInfectedType>(Type);
	const float Height = (InfectedType == EInfectedType::Brute) ? 130.0f : 100.0f;

	AInfectedCharacter* Infected = Cast<AInfectedCharacter>(SpawnTagged(AInfectedCharacter::StaticClass(), Location + FVector(0.0f, 0.0f, Height), FRotator(0.0f, Yaw, 0.0f), Label));

	if (Infected)
	{
		Infected->SetInfectedType(InfectedType);
	}
}

AReachLocationObjective* AMission01Builder::SpawnReach(int32 Order, const FText& Description, const FVector& Center, const FVector& Extent, const FString& Label)
{
	AReachLocationObjective* Reach = Cast<AReachLocationObjective>(SpawnTagged(AReachLocationObjective::StaticClass(), Center, FRotator::ZeroRotator, Label));

	if (Reach)
	{
		SetupObjective(Reach, Order, Description);

		if (UBoxComponent* Trigger = Reach->FindComponentByClass<UBoxComponent>())
		{
			Trigger->SetBoxExtent(Extent);
		}
	}

	return Reach;
}

void AMission01Builder::SetupObjective(AMissionObjective* Objective, int32 Order, const FText& Description)
{
	Objective->Order = Order;
	Objective->Description = Description;
}

void AMission01Builder::SpawnCheckpoint(const FVector& Location, const FString& Label)
{
	SpawnTagged(ACheckpoint::StaticClass(), Location, FRotator::ZeroRotator, Label);
}
