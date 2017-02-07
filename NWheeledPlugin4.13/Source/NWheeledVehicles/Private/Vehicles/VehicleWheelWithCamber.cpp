// Copyright 2016 Santoro Studio, All Rights Reserved.

#include "../NWheeledVehiclesPluginPrivatePCH.h"
#include "Vehicles/VehicleWheelWithCamber.h"
#include "Vehicles/NWheeledVehicleMovementComponent.h"
#include "Vehicles/NDWheeledVehicleMovementComponent.h"
#include "Runtime/Engine/Classes/GameFramework/PawnMovementComponent.h"
#include "Runtime/Engine/Public/PhysicsPublic.h"
#include "Runtime/Engine/Classes/Vehicles/VehicleWheel.h"
#include "Runtime/Engine/Classes/Vehicles/WheeledVehicleMovementComponent.h"
#include "Runtime/Engine/Classes/Vehicles/TireType.h"
#include "Runtime/Engine/Private/PhysicsEngine/PhysXSupport.h"
#include "Runtime/Engine/Private/Vehicles/PhysXVehicleManager.h"

UVehicleWheelWithCamber::UVehicleWheelWithCamber(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SuspensionCamberAtRest = 0.0f;
	SuspensionCamberAtMaxDroop = 0.0f;
	SuspensionCamberAtMaxCompression = 0.0f;
	SuspensionSpringStrength = 0.0f;
	SuspensionDamperRatio   = 0.0f;


}

FQuat UVehicleWheelWithCamber::GetWheelRotationAngles()
{
#if WITH_VEHICLE
	PxTransform localPose = WheelShape->getLocalPose();
	FQuat wheelPose(localPose.q.x, localPose.q.y, localPose.q.z, localPose.q.w);
	return wheelPose;
#else
	FQuat zero;
	return zero;
#endif // WITH_PHYSX
}

#if WITH_EDITOR

void UVehicleWheelWithCamber::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	/**
	if (PropertyName == TEXT("SuspensionAutoStrength"))
	{
		UVehicleWheel* VehicleWheel = Cast<UVehicleWheel>(this);
		//VehicleWheel->PostEditChangeProperty(PropertyChangedEvent);

		UVehicleWheelWithCamber.get
		if (VehicleSim != nullptr) {

			// Prealloc data for the sprung masses
			PxVec3 WheelOffsets[32];
			float SprungMasses[32];
			uint32 VehicleMass;

			for (int32 WheelIdx = 0; WheelIdx < VehicleSim->WheelSetups.Num(); ++WheelIdx)
			{
				WheelOffsets[WheelIdx] = U2PVector(VehicleSim->WheelSetups[WheelIdx].WheelClass.GetDefaultObject()->Offset + VehicleSim->WheelSetups[WheelIdx].AdditionalOffset);
			}

			FVector LocalCOM = FVector::ZeroVector;

			if (VehicleSim->UpdatedPrimitive)
			{
				if (const FBodyInstance* BodyInst = VehicleSim->UpdatedPrimitive->GetBodyInstance())
				{
					ExecuteOnPxRigidDynamicReadOnly(BodyInst, [&](const PxRigidDynamic* PVehicleActor)
					{
						PxTransform PCOMTransform = PVehicleActor->getCMassLocalPose();
						VehicleMass = PVehicleActor->getMass();
						LocalCOM = P2UVector(PCOMTransform.p);
					});
				}
			}

			PxVec3 PLocalCOM = U2PVector(LocalCOM);
			// Now that we have all the wheel offsets, calculate the sprung masses
			PxVehicleComputeSprungMasses(VehicleSim->WheelSetups.Num(), WheelOffsets, PLocalCOM, VehicleMass, 2, SprungMasses);

		}
	} 
	if (VehicleNWSim != nullptr) {

		// Prealloc data for the sprung masses
		PxVec3 WheelOffsets[32];
		float SprungMasses[32];
		uint32 VehicleMass;

		for (int32 WheelIdx = 0; WheelIdx < VehicleNWSim->WheelSetups.Num(); ++WheelIdx)
		{
			WheelOffsets[WheelIdx] = U2PVector(VehicleNWSim->WheelSetups[WheelIdx].WheelClass.GetDefaultObject()->Offset + VehicleNWSim->WheelSetups[WheelIdx].AdditionalOffset);
		}

		FVector LocalCOM = FVector::ZeroVector;

		if (VehicleNWSim->UpdatedPrimitive)
		{
			if (const FBodyInstance* BodyInst = VehicleNWSim->UpdatedPrimitive->GetBodyInstance())
			{
				ExecuteOnPxRigidDynamicReadOnly(BodyInst, [&](const PxRigidDynamic* PVehicleActor)
				{
					PxTransform PCOMTransform = PVehicleActor->getCMassLocalPose();
					VehicleMass = PVehicleActor->getMass();
					LocalCOM = P2UVector(PCOMTransform.p);
				});
			}
		}

		PxVec3 PLocalCOM = U2PVector(LocalCOM);
		// Now that we have all the wheel offsets, calculate the sprung masses
		PxVehicleComputeSprungMasses(VehicleNWSim->WheelSetups.Num(), WheelOffsets, PLocalCOM, VehicleMass, 2, SprungMasses);

	} 
	if (VehicleNDSim != nullptr) {

		// Prealloc data for the sprung masses
		PxVec3 WheelOffsets[32];
		float SprungMasses[32];
		uint32 VehicleMass;

		for (int32 WheelIdx = 0; WheelIdx < VehicleNDSim->WheelSetups.Num(); ++WheelIdx)
		{
			WheelOffsets[WheelIdx] = U2PVector(VehicleNDSim->WheelSetups[WheelIdx].WheelClass.GetDefaultObject()->Offset + VehicleNDSim->WheelSetups[WheelIdx].AdditionalOffset);
		}

		FVector LocalCOM = FVector::ZeroVector;

		if (VehicleNDSim->UpdatedPrimitive)
		{
			if (const FBodyInstance* BodyInst = VehicleNDSim->UpdatedPrimitive->GetBodyInstance())
			{
				ExecuteOnPxRigidDynamicReadOnly(BodyInst, [&](const PxRigidDynamic* PVehicleActor)
				{
					PxTransform PCOMTransform = PVehicleActor->getCMassLocalPose();
					VehicleMass = PVehicleActor->getMass();
					LocalCOM = P2UVector(PCOMTransform.p);
				});
			}
		}

		PxVec3 PLocalCOM = U2PVector(LocalCOM);
		// Now that we have all the wheel offsets, calculate the sprung masses
		PxVehicleComputeSprungMasses(VehicleNDSim->WheelSetups.Num(), WheelOffsets, PLocalCOM, VehicleMass, 2, SprungMasses);

	}
	*/
}

#endif //WITH_EDITOR
