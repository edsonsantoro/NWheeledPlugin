// Copyright 2016 Santoro Studio, All Rights Reserved.

/*=============================================================================
Vehicle.cpp: AWheeledVehicleNoDrive implementation
TODO: Put description here
=============================================================================*/

#include "../NWheeledVehiclesPluginPrivatePCH.h"
#include "Vehicles/WheeledVehicleNoDrive.h"
#include "Runtime/Engine/Private/EnginePrivate.h"
#include "Vehicles/WheeledVehicleMovementComponentND.h"
#include "Runtime/Engine/Public/DisplayDebugHelpers.h"

FName AWheeledVehicleNoDrive::VehicleMovementComponentName(TEXT("MovementComp"));
FName AWheeledVehicleNoDrive::VehicleMeshComponentName(TEXT("VehicleMesh"));

AWheeledVehicleNoDrive::AWheeledVehicleNoDrive(const FObjectInitializer& ObjectInitializer)
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

	VehicleMovement = CreateDefaultSubobject<UNDWheeledVehicleMovementComponent, UWheeledVehicleMovementComponentND>(VehicleMovementComponentName);
	VehicleMovement->SetIsReplicated(true); // Enable replication by default
	VehicleMovement->UpdatedComponent = Mesh;
}

void AWheeledVehicleNoDrive::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
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

class UNDWheeledVehicleMovementComponent* AWheeledVehicleNoDrive::GetVehicleMovementComponent() const
{
	return VehicleMovement;
}

/** Returns Mesh subobject **/
USkeletalMeshComponent* AWheeledVehicleNoDrive::GetMesh() const { return Mesh; }
/** Returns VehicleMovement subobject **/
UNDWheeledVehicleMovementComponent* AWheeledVehicleNoDrive::GetVehicleMovement() const { return VehicleMovement; }
