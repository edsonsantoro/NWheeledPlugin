// Copyright 2016 Santoro Studio, All Rights Reserved.

/*=============================================================================
Vehicle.cpp: ATankVehicle implementation
TODO: Put description here
=============================================================================*/

#include "../NWheeledVehiclesPluginPrivatePCH.h"
#include "Vehicles/TankVehicle.h"
#include "Vehicles/WheeledVehicleMovementComponent.h"
#include "Vehicles/WheeledVehicleMovementComponentTank.h"
#include "Runtime/Engine/Private/EnginePrivate.h"
#include "Runtime/Engine/Public/DisplayDebugHelpers.h"

FName ATankVehicle::VehicleMovementComponentName(TEXT("MovementComp"));
FName ATankVehicle::VehicleMeshComponentName(TEXT("VehicleMesh"));

ATankVehicle::ATankVehicle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(VehicleMeshComponentName);
	Mesh->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);
	Mesh->BodyInstance.bSimulatePhysics = true;
	Mesh->BodyInstance.bNotifyRigidBodyCollision = true;
	Mesh->BodyInstance.bUseCCD = true;
	Mesh->bBlendPhysics = true;
	Mesh->bGenerateOverlapEvents = true;
	Mesh->SetCanEverAffectNavigation(false);
	RootComponent = Mesh;

	VehicleMovement = CreateDefaultSubobject<UWheeledVehicleMovementComponent, UWheeledVehicleMovementComponentTank>(VehicleMovementComponentName);
	VehicleMovement->SetIsReplicated(true); // Enable replication by default
	VehicleMovement->UpdatedComponent = Mesh;
}

void ATankVehicle::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	static FName NAME_Vehicle = FName(TEXT("Vehicle"));

	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

#if WITH_VEHICLE
	if (DebugDisplay.IsDisplayOn(NAME_Vehicle))
	{
		GetVehicleMovementComponent()->DrawDebug(Canvas, YL, YPos);
	}
#endif
}

class UWheeledVehicleMovementComponent* ATankVehicle::GetVehicleMovementComponent() const
{
	return VehicleMovement;
}

/** Returns Mesh subobject **/
USkeletalMeshComponent* ATankVehicle::GetMesh() const { return Mesh; }
/** Returns VehicleMovement subobject **/
UWheeledVehicleMovementComponent* ATankVehicle::GetVehicleMovement() const { return VehicleMovement; }
