// Copyright 2016 Santoro Studio, All Rights Reserved.

/*=============================================================================
Vehicle.cpp: ANWheeledVehicle implementation
TODO: Put description here
=============================================================================*/

#include "../NWheeledVehiclesPluginPrivatePCH.h"
#include "Vehicles/NWheeledVehicle.h"
//#include "Vehicles/NWheeledVehicleMovementComponent.h"
#include "Vehicles/WheeledVehicleMovementComponentNW.h"
//#include "Runtime/Engine/Private/EnginePrivate.h"
#include "Runtime/Engine/Public/DisplayDebugHelpers.h"

FName ANWheeledVehicle::VehicleMovementComponentName(TEXT("MovementComp"));
FName ANWheeledVehicle::VehicleMeshComponentName(TEXT("VehicleMesh"));

ANWheeledVehicle::ANWheeledVehicle(const FObjectInitializer& ObjectInitializer)
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

	VehicleMovement = CreateDefaultSubobject<UNWheeledVehicleMovementComponent, UWheeledVehicleMovementComponentNW>(VehicleMovementComponentName);
	VehicleMovement->SetIsReplicated(true); // Enable replication by default
	VehicleMovement->UpdatedComponent = Mesh;
}

void ANWheeledVehicle::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
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

class UNWheeledVehicleMovementComponent* ANWheeledVehicle::GetVehicleMovementComponent() const
{
	return VehicleMovement;
}

/** Returns Mesh subobject **/
USkeletalMeshComponent* ANWheeledVehicle::GetMesh() const { return Mesh; }
/** Returns VehicleMovement subobject **/
UNWheeledVehicleMovementComponent* ANWheeledVehicle::GetVehicleMovement() const { return VehicleMovement; }
