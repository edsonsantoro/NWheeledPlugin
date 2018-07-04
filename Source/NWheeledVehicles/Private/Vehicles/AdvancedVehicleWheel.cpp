// Copyright 2016 Santoro Studio, All Rights Reserved.

#include "NWheeledVehiclesPluginPrivatePCH.h"
#include "Vehicles/AdvancedVehicleWheel.h"
#include "Vehicles/NWheeledVehicleMovementComponent.h"
#include "VehicleWheel.h"
#include "Vehicles/TireType.h"
#include "WheeledVehicleMovementComponent.h"
#include "TireConfig.h"
#include "PhysXVehicleManager.h"
#include "PhysXPublic.h"



UAdvancedVehicleWheel::UAdvancedVehicleWheel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SuspensionCamberAtRest = 0.0f;
	SuspensionCamberAtMaxDroop = 0.01f;
	SuspensionCamberAtMaxCompression = -0.01f;
	SuspensionSpringStrength = 35000.0f;
	CamberStiffnessPerUnitGravity = 0.0f;
	DamperRate = 4500.0f;

}

FQuat UAdvancedVehicleWheel::GetWheelRotationAngles()
{
	PxTransform localPose = WheelShape->getLocalPose();
	FQuat wheelPose(localPose.q.x, localPose.q.y, localPose.q.z, localPose.q.w);
	return wheelPose;
}

#if WITH_EDITOR

void UAdvancedVehicleWheel::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : "SuspensionAutoStrength";
	
	// Trigger a runtime rebuild of the PhysX vehicle
	//FPhysXVehicleManager::VehicleSetupTag++;

}

#endif //WITH_EDITOR
