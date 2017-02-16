// Copyright 2016 Santoro Studio, All Rights Reserved.

/*=============================================================================
UNVehicleAnimInstance.cpp: Single Node Tree Instance
Only plays one animation at a time.
=============================================================================*/

#include "../NWheeledVehiclesPluginPrivatePCH.h"
#include "Vehicles/NVehicleAnimInstance.h"
#include "VehicleAnimInstance.h"
#include "WheeledVehicleMovementComponent.h"
#include "Vehicles/NWheeledVehicle.h"
#include "AnimationRuntime.h"

/////////////////////////////////////////////////////
// UNVehicleAnimInstance
/////////////////////////////////////////////////////

UNVehicleAnimInstance::UNVehicleAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

class ANWheeledVehicle * UNVehicleAnimInstance::GetVehicle()
{
	return Cast<ANWheeledVehicle>(GetOwningActor());
}

FAnimInstanceProxy* UNVehicleAnimInstance::CreateAnimInstanceProxy()
{
	const UWheeledVehicleMovementComponent* InWheeledVehicleMovementComponent = this->GetWheeledVehicleMovementComponent();
	SetNWheeledVehicleMovementComponent(InWheeledVehicleMovementComponent);
	return &NAnimInstanceProxy;
}

void UNVehicleAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy)
{
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
//// PROXY ///

void FNVehicleAnimInstanceProxy::SetNWheeledVehicleMovementComponent(const UWheeledVehicleMovementComponent* InWheeledVehicleMovementComponent)
{
	if (const UWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = InWheeledVehicleMovementComponent)
	{

	//initialize wheel data
	const int32 NumOfwheels = WheeledVehicleMovementComponent->WheelSetups.Num();
	NWheelInstances.Empty(NumOfwheels);
	if (NumOfwheels > 0)
	{
		NWheelInstances.AddZeroed(NumOfwheels);
		// now add wheel data
		for (int32 WheelIndex = 0; WheelIndex < NWheelInstances.Num(); ++WheelIndex)
		{
			FWheelAnimData& WheelInstance = NWheelInstances[WheelIndex];
			const FWheelSetup& WheelSetup = WheeledVehicleMovementComponent->WheelSetups[WheelIndex];

			// set data
			WheelInstance.BoneName = WheelSetup.BoneName;
			WheelInstance.LocOffset = FVector::ZeroVector;
			WheelInstance.RotOffset = FRotator::ZeroRotator;
		}
	}
	}
}


void FNVehicleAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	Super::PreUpdate(InAnimInstance, DeltaSeconds);

	const UNVehicleAnimInstance* NVehicleAnimInstance = CastChecked<UNVehicleAnimInstance>(InAnimInstance);
	const UWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = NVehicleAnimInstance->GetWheeledVehicleMovementComponent();

	if (const UNWheeledVehicleMovementComponent* NWheeledVehicleMovementComponent = Cast<UNWheeledVehicleMovementComponent>(WheeledVehicleMovementComponent))
	{
		TArray<FWheelAnimData> WheelInstances = this->GetWheelAnimData();
		for (int32 WheelIndex = 0; WheelIndex < WheelInstances.Num(); ++WheelIndex)
		{
			FWheelAnimData& WheelInstance = WheelInstances[WheelIndex];
			if (WheeledVehicleMovementComponent->Wheels.IsValidIndex(WheelIndex))
			{
				if (const UVehicleWheel* VehicleWheel = WheeledVehicleMovementComponent->Wheels[WheelIndex])
				{
					WheelInstance.RotOffset.Pitch = VehicleWheel->GetRotationAngle();
					WheelInstance.RotOffset.Yaw = VehicleWheel->GetSteerAngle();
					WheelInstance.RotOffset.Roll = 0.f;

					WheelInstance.LocOffset.X = 0.f;
					WheelInstance.LocOffset.Y = 0.f;
					WheelInstance.LocOffset.Z = VehicleWheel->GetSuspensionOffset();
				}
			}
		}
	}
}
