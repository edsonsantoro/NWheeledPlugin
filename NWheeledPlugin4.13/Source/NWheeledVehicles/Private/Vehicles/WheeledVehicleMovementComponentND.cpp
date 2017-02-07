// Copyright 2016 Santoro Studio, All Rights Reserved.

#include "../NWheeledVehiclesPluginPrivatePCH.h"
#include "Vehicles/WheeledVehicleMovementComponentND.h"
#include "Runtime/Engine/Private/EnginePrivate.h"
#include "Runtime/Engine/Public/PhysicsPublic.h"

#if WITH_VEHICLE
#include "Runtime/Engine/Private/PhysicsEngine/PhysXSupport.h"
#include "Runtime/Engine/Private/Vehicles/PhysXVehicleManager.h"
#endif // WITH_VEHICLE

UWheeledVehicleMovementComponentND::UWheeledVehicleMovementComponentND(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if WITH_VEHICLE

	// Init steering speed curve
	FRichCurve* SteeringCurveData = SteeringCurve.GetRichCurve();
	SteeringCurveData->AddKey(0.f, 1.f);
	SteeringCurveData->AddKey(20.f, 0.9f);
	SteeringCurveData->AddKey(60.f, 0.8f);
	SteeringCurveData->AddKey(120.f, 0.7f);

	// Initialize WheelSetups array with 4 wheels
	WheelSetups.SetNum(4);
	WheelsData.DrivenWheels.SetNum(WheelSetups.Num());

#endif // WITH_VEHICLE
}

#if WITH_EDITOR
void UWheeledVehicleMovementComponentND::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == TEXT("SteeringCurve"))
	{
		//make sure values are capped between 0 and 1
		TArray<FRichCurveKey> SteerKeys = SteeringCurve.GetRichCurve()->GetCopyOfKeys();
		for (int32 KeyIdx = 0; KeyIdx < SteerKeys.Num(); ++KeyIdx)
		{
			float NewValue = FMath::Clamp(SteerKeys[KeyIdx].Value, 0.f, 1.f);
			SteeringCurve.GetRichCurve()->UpdateOrAddKey(SteerKeys[KeyIdx].Time, NewValue);
		}
	}
	else if ((PropertyName == TEXT("WheelSetups")) || (PropertyName == TEXT("BoneName")))
	{
		FVehicleWheelsData& Wheels = WheelsData;
		Wheels.DrivenWheels.SetNum(WheelSetups.Num());
		for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); WheelIdx++)
		{
			Wheels.DrivenWheels[WheelIdx].WheelReference = WheelSetups[WheelIdx].BoneName;
		}
	}

}
#endif

#if WITH_VEHICLE

static void GetVehicleWheelsDataSetup(const FVehicleWheelsData& Setup, PxVehicleWheelsSimData& PxSetup, const TArray<FWheelSetup> WheelSetups)
{
	for (int32 WheelIdx = 0; WheelIdx < Setup.DrivenWheels.Num(); ++WheelIdx)
	{
		if (Setup.DrivenWheels[WheelIdx].IsWheelEnabled == false)
		{
			PxSetup.disableWheel(WheelIdx);
		}
		else { 
			PxSetup.enableWheel(WheelIdx);
		}
	}
}

void SetupDriveHelper(const UWheeledVehicleMovementComponentND* VehicleData, const PxVehicleWheelsSimData* PWheelsSimData, PxVehicleWheelsSimData& DriveData)
{
	PxVehicleWheelsSimData WheelsData;
	GetVehicleWheelsDataSetup(VehicleData->WheelsData, WheelsData, VehicleData->WheelSetups);
}


void UWheeledVehicleMovementComponentND::SetupVehicle()
{
	if (!UpdatedPrimitive)
	{
		return;
	}

	for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx)
	{
		const FWheelSetup& WheelSetup = WheelSetups[WheelIdx];
		if (WheelSetup.BoneName == NAME_None)
		{
			return;
		}
	}

	// Setup the chassis and wheel shapes
	SetupVehicleShapes();

	// Setup mass properties
	SetupVehicleMass();

	// Setup the wheels
	PxVehicleWheelsSimData* PWheelsSimData = PxVehicleWheelsSimData::allocate(WheelSetups.Num());
	SetupWheels(PWheelsSimData);

	// Setup drive data
	PxVehicleWheelsSimData DriveData;
	SetupDriveHelper(this, PWheelsSimData, DriveData);

	// Create the vehicle
	PxVehicleNoDrive* PVehicleNoDrive = PxVehicleNoDrive::allocate(WheelSetups.Num());
	check(PVehicleNoDrive);

	ExecuteOnPxRigidDynamicReadWrite(UpdatedPrimitive->GetBodyInstance(), [&](PxRigidDynamic* PRigidDynamic)
	{
		PVehicleNoDrive->setup(GPhysXSDK, PRigidDynamic, *PWheelsSimData);
		PVehicleNoDrive->setToRestState();

		// cleanup
		PWheelsSimData->free();
	});

	PWheelsSimData = NULL;

	// cache values
	PVehicle = PVehicleNoDrive;
	PVehicleDrive = PVehicleNoDrive;

}

void UWheeledVehicleMovementComponentND::UpdateSimulation(float DeltaTime)
{
	if (PVehicleDrive == NULL)
		return;

	UpdatedPrimitive->GetBodyInstance()->ExecuteOnPhysicsReadWrite([&]
	{

		// Convert from our curve to PxFixedSizeLookupTable
		PxFixedSizeLookupTable<8> SpeedSteerLookup;
		TArray<FRichCurveKey> SteerKeys = SteeringCurve.GetRichCurve()->GetCopyOfKeys();
		const int32 MaxSteeringSamples = FMath::Min(8, SteerKeys.Num());
		for (int32 KeyIdx = 0; KeyIdx < MaxSteeringSamples; KeyIdx++)
		{
			FRichCurveKey& Key = SteerKeys[KeyIdx];
			SpeedSteerLookup.addPair(KmHToCmS(Key.Time), FMath::Clamp(Key.Value, 0.f, 1.f));
		}

		PxVehiclePadSmoothingData SmoothData = {
			{ ThrottleInputRate.RiseRate, BrakeInputRate.RiseRate, HandbrakeInputRate.RiseRate, SteeringInputRate.RiseRate, SteeringInputRate.RiseRate },
			{ ThrottleInputRate.FallRate, BrakeInputRate.FallRate, HandbrakeInputRate.FallRate, SteeringInputRate.FallRate, SteeringInputRate.FallRate }
		};

		if (RawThrottleInput > 0) {
			for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx) {
				PVehicleDrive->setBrakeTorque(WheelIdx, 0);
				PVehicleDrive->setDriveTorque(WheelIdx, (RawThrottleInput * WheelsData.DrivenWheels[WheelIdx].DriveTorque));
			}
		}
		if (RawThrottleInput < 0) {
			int32 vehicleSpeed = PVehicle->computeForwardSpeed();
			for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx) {
				PxVehicleWheelData VehicleWheel = PVehicle->mWheelsSimData.getWheelData(WheelIdx);
				PVehicleDrive->setBrakeTorque(WheelIdx, (FMath::Square(RawThrottleInput) * VehicleWheel.mMaxBrakeTorque));

				if (vehicleSpeed == 0 && RawThrottleInput < 0) {
					PVehicleDrive->setBrakeTorque(WheelIdx, 0);
					PVehicleDrive->setDriveTorque(WheelIdx, (RawThrottleInput * WheelsData.DrivenWheels[WheelIdx].DriveTorque));
				}

			} 

		}
		if (RawThrottleInput == 0) {
			for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx) {
				int32 ThresholdLongSpeed = PVehicle->mWheelsSimData.getThresholdLongSpeed();
				PVehicleDrive->setBrakeTorque(WheelIdx, ThresholdLongSpeed);
			}
		}

		if (SteeringInput != 0) {
			for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx) {
				PxVehicleWheelData VehicleWheel = PVehicle->mWheelsSimData.getWheelData(WheelIdx);
				PVehicleDrive->setSteerAngle(WheelIdx, (VehicleWheel.mMaxSteer * SteeringInput));
			}
		} /**
		else {
			for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx) {
				int32 ThresholdLongSpeed = PVehicle->mWheelsSimData.getThresholdLongSpeed();
				PVehicleDrive->setBrakeTorque(WheelIdx, ThresholdLongSpeed);
				PVehicleDrive->setDriveTorque(WheelIdx, (RawThrottleInput * WheelsData.DrivenWheels[WheelIdx].DriveTorque));
			}
		}
		*/

	});
}
#endif // WITH_VEHICLE


void UWheeledVehicleMovementComponentND::UpdateWheelsDataSetup(const FVehicleWheelsData& NewWheelsDataSetup)
{
#if WITH_VEHICLE
	if (PVehicleDrive)
	{
		PxVehicleWheelsSimData WheelsData;
		GetVehicleWheelsDataSetup(NewWheelsDataSetup, WheelsData, WheelSetups);


		//PxVehicleNoDrive* PVehicleDrive = (PxVehicleNoDrive*)PVehicleDrive;
		
	}
#endif
}

void UWheeledVehicleMovementComponentND::Serialize(FArchive & Ar)
{
	Super::Serialize(Ar);
}


void UWheeledVehicleMovementComponentND::ComputeConstants()
{
	Super::ComputeConstants();
}
