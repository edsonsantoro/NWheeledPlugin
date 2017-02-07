#include "../NWheeledVehiclesPluginPrivatePCH.h"
#include "AnimGraphRuntime/AnimNode_CamberWheelHandler.h"
#include "Vehicles/NWheeledVehicle.h"
#include "Vehicles/WheeledVehicleNoDrive.h"
#include "Vehicles/VehicleWheelWithCamber.h"
#include "Runtime/AnimGraphRuntime/Private/AnimGraphRuntimePrivatePCH.h"
#include "Runtime/Engine/Public/AnimationRuntime.h"
#include "Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_WheelHandler.h"
#include "Runtime/Engine/Classes/GameFramework/WheeledVehicle.h"
#include "Runtime/Engine/Classes/Vehicles/WheeledVehicleMovementComponent.h"
#include "Runtime/Engine/Classes/Vehicles/VehicleWheel.h"
#include "Runtime/Engine/Public/Animation/AnimInstanceProxy.h"



FAnimNode_CamberWheelHandler::FAnimNode_CamberWheelHandler()
{
}

void FAnimNode_CamberWheelHandler::UpdateInternal(const FAnimationUpdateContext& Context)
{
	if (VehicleSimComponent)
	{
		for (auto & WheelSim : WheelSimulators)
		{
			if (VehicleSimComponent->Wheels.IsValidIndex(WheelSim.WheelIndex))
			{
				UVehicleWheel* Wheel = VehicleSimComponent->Wheels[WheelSim.WheelIndex];
				UVehicleWheelWithCamber* CamberWheel = Cast<UVehicleWheelWithCamber>(Wheel);
				if (CamberWheel != nullptr)
				{
					FQuat WheelSimRotator = CamberWheel->GetWheelRotationAngles();
					WheelSim.RotOffset = WheelSimRotator.Rotator();
				}
				else 
				{
					WheelSim.RotOffset.Yaw = Wheel->GetSteerAngle();
					WheelSim.RotOffset.Pitch = Wheel->GetRotationAngle();
					WheelSim.RotOffset.Roll = 0.0f;
				}

				WheelSim.LocOffset.X = 0.f;
				WheelSim.LocOffset.Y = 0.f;
				WheelSim.LocOffset.Z = Wheel->GetSuspensionOffset();

			}
		}
	}
	if (VehicleNWSimComponent)
	{
		for (auto & WheelSim : WheelSimulators)
		{
			if (VehicleNWSimComponent->Wheels.IsValidIndex(WheelSim.WheelIndex))
			{
				UVehicleWheel* Wheel = VehicleNWSimComponent->Wheels[WheelSim.WheelIndex];
				UVehicleWheelWithCamber* CamberWheel = Cast<UVehicleWheelWithCamber>(Wheel);
				if (CamberWheel != nullptr)
				{
					FQuat WheelSimRotator = CamberWheel->GetWheelRotationAngles();
					WheelSim.RotOffset = WheelSimRotator.Rotator();
				}
				else
				{
					WheelSim.RotOffset.Yaw = Wheel->GetSteerAngle();
					WheelSim.RotOffset.Pitch = Wheel->GetRotationAngle();
					WheelSim.RotOffset.Roll = 0.0f;
				}

				WheelSim.LocOffset.X = 0.f;
				WheelSim.LocOffset.Y = 0.f;
				WheelSim.LocOffset.Z = Wheel->GetSuspensionOffset();

			}
		}
	}
	if (VehicleNDSimComponent)
	{
		for (auto & WheelSim : WheelSimulators)
		{
			if (VehicleNDSimComponent->Wheels.IsValidIndex(WheelSim.WheelIndex))
			{
				UVehicleWheel* Wheel = VehicleNDSimComponent->Wheels[WheelSim.WheelIndex];
				UVehicleWheelWithCamber* CamberWheel = Cast<UVehicleWheelWithCamber>(Wheel);
				if (CamberWheel != nullptr)
				{
					FQuat WheelSimRotator = CamberWheel->GetWheelRotationAngles();
					WheelSim.RotOffset = WheelSimRotator.Rotator();

				}
				else
				{
					WheelSim.RotOffset.Yaw = Wheel->GetSteerAngle();
					WheelSim.RotOffset.Pitch = Wheel->GetRotationAngle();
					WheelSim.RotOffset.Roll = 0.0f;

				}

				WheelSim.LocOffset.X = 0.f;
				WheelSim.LocOffset.Y = 0.f;
				WheelSim.LocOffset.Z = Wheel->GetSuspensionOffset();

			}
		}
	}


	FAnimNode_SkeletalControlBase::UpdateInternal(Context);
}


void FAnimNode_CamberWheelHandler::Initialize(const FAnimationInitializeContext& Context)
{
	// TODO: only check vehicle anim instance
	// UVehicleAnimInstance
	AWheeledVehicle * StandardVehicle = Cast<AWheeledVehicle>(Context.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner());
	ANWheeledVehicle * NWheeledVehicle = Cast<ANWheeledVehicle>(Context.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner());
	AWheeledVehicleNoDrive * NoDriveVehicle = Cast<AWheeledVehicleNoDrive>(Context.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner());

	if (StandardVehicle != nullptr)
	{
		VehicleSimComponent = StandardVehicle->GetVehicleMovementComponent();

		int32 NumOfwheels = VehicleSimComponent->WheelSetups.Num();
		if (NumOfwheels > 0)
		{
			WheelSimulators.Empty(NumOfwheels);
			WheelSimulators.AddZeroed(NumOfwheels);
			// now add wheel data
			for (int32 WheelIndex = 0; WheelIndex<WheelSimulators.Num(); ++WheelIndex)
			{
				FWheelSimulator & WheelSim = WheelSimulators[WheelIndex];
				const FWheelSetup& WheelSetup = VehicleSimComponent->WheelSetups[WheelIndex];

				// set data
				WheelSim.WheelIndex = WheelIndex;
				WheelSim.BoneReference.BoneName = WheelSetup.BoneName;
				WheelSim.LocOffset = FVector::ZeroVector;
				WheelSim.RotOffset = FRotator::ZeroRotator;
			}
		}
	} if (NWheeledVehicle != nullptr)
	{
		VehicleNWSimComponent = NWheeledVehicle->GetVehicleMovementComponent();

		int32 NumOfwheels = VehicleNWSimComponent->WheelSetups.Num();
		if (NumOfwheels > 0)
		{
			WheelSimulators.Empty(NumOfwheels);
			WheelSimulators.AddZeroed(NumOfwheels);
			// now add wheel data
			for (int32 WheelIndex = 0; WheelIndex<WheelSimulators.Num(); ++WheelIndex)
			{
				FWheelSimulator & WheelSim = WheelSimulators[WheelIndex];
				const FWheelSetup& WheelSetup = VehicleNWSimComponent->WheelSetups[WheelIndex];

				// set dataVehicleNWSimComponent
				WheelSim.WheelIndex = WheelIndex;
				WheelSim.BoneReference.BoneName = WheelSetup.BoneName;
				WheelSim.LocOffset = FVector::ZeroVector;
				WheelSim.RotOffset = FRotator::ZeroRotator;
			}
		}


	} if (NoDriveVehicle != nullptr)
	{
		VehicleNDSimComponent = NoDriveVehicle->GetVehicleMovementComponent();

		int32 NumOfwheels = VehicleNDSimComponent->WheelSetups.Num();
		if (NumOfwheels > 0)
		{
			WheelSimulators.Empty(NumOfwheels);
			WheelSimulators.AddZeroed(NumOfwheels);
			// now add wheel data
			for (int32 WheelIndex = 0; WheelIndex<WheelSimulators.Num(); ++WheelIndex)
			{
				FWheelSimulator & WheelSim = WheelSimulators[WheelIndex];
				const FWheelSetup& WheelSetup = VehicleNDSimComponent->WheelSetups[WheelIndex];

				// set dataVehicleNWSimComponent
				WheelSim.WheelIndex = WheelIndex;
				WheelSim.BoneReference.BoneName = WheelSetup.BoneName;
				WheelSim.LocOffset = FVector::ZeroVector;
				WheelSim.RotOffset = FRotator::ZeroRotator;
			}
		}


	}


	FAnimNode_WheelHandler::Initialize(Context);
}
