// Copyright 2016 Santoro Studio, All Rights Reserved.

#include "NWheeledVehiclesPluginPrivatePCH.h"
#include "Vehicles/WheeledVehicleMovementComponentTank.h"
#include "Components/PrimitiveComponent.h"

#include "PhysXPublic.h"


UWheeledVehicleMovementComponentTank::UWheeledVehicleMovementComponentTank(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

	// grab default values from physx
	PxVehicleEngineData DefEngineData;
	EngineSetup.MOI = DefEngineData.mMOI;
	EngineSetup.MaxRPM = OmegaToRPM(DefEngineData.mMaxOmega);
	EngineSetup.DampingRateFullThrottle = DefEngineData.mDampingRateFullThrottle;
	EngineSetup.DampingRateZeroThrottleClutchEngaged = DefEngineData.mDampingRateZeroThrottleClutchEngaged;
	EngineSetup.DampingRateZeroThrottleClutchDisengaged = DefEngineData.mDampingRateZeroThrottleClutchDisengaged;

	// Convert from PhysX curve to ours
	FRichCurve* TorqueCurveData = EngineSetup.TorqueCurve.GetRichCurve();
	for (PxU32 KeyIdx = 0; KeyIdx < DefEngineData.mTorqueCurve.getNbDataPairs(); KeyIdx++)
	{
		float Input = DefEngineData.mTorqueCurve.getX(KeyIdx) * EngineSetup.MaxRPM;
		float Output = DefEngineData.mTorqueCurve.getY(KeyIdx) * DefEngineData.mPeakTorque;
		TorqueCurveData->AddKey(Input, Output);
	}

	PxVehicleClutchData DefClutchData;
	TransmissionSetup.ClutchStrength = DefClutchData.mStrength;

	PxVehicleGearsData DefGearSetup;
	TransmissionSetup.GearSwitchTime = DefGearSetup.mSwitchTime;
	TransmissionSetup.ReverseGearRatio = DefGearSetup.mRatios[PxVehicleGearsData::eREVERSE];
	TransmissionSetup.FinalRatio = DefGearSetup.mFinalRatio;

	PxVehicleAutoBoxData DefAutoBoxSetup;
	TransmissionSetup.NeutralGearUpRatio = DefAutoBoxSetup.mUpRatios[PxVehicleGearsData::eNEUTRAL];
	TransmissionSetup.GearAutoBoxLatency = DefAutoBoxSetup.getLatency();
	TransmissionSetup.bUseGearAutoBox = true;

	for (uint32 i = PxVehicleGearsData::eFIRST; i < DefGearSetup.mNbRatios; i++)
	{
		FTankGearData GearData;
		GearData.DownRatio = DefAutoBoxSetup.mDownRatios[i];
		GearData.UpRatio = DefAutoBoxSetup.mUpRatios[i];
		GearData.Ratio = DefGearSetup.mRatios[i];
		TransmissionSetup.ForwardGears.Add(GearData);
	}

	FRichCurve* SteeringCurveData = SteeringCurve.GetRichCurve();
	SteeringCurveData->AddKey(0.f, 1.f);
	SteeringCurveData->AddKey(20.f, 0.9f);
	SteeringCurveData->AddKey(60.f, 0.8f);
	SteeringCurveData->AddKey(120.f, 0.7f);

	// Initialize WheelSetups array with 4 wheels
	WheelSetups.SetNum(4);
}

#if WITH_EDITOR
void UWheeledVehicleMovementComponentTank::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == TEXT("DownRatio"))
	{
		for (int32 GearIdx = 0; GearIdx < TransmissionSetup.ForwardGears.Num(); ++GearIdx)
		{
			FTankGearData & GearData = TransmissionSetup.ForwardGears[GearIdx];
			GearData.DownRatio = FMath::Min(GearData.DownRatio, GearData.UpRatio);
		}
	}
	else if (PropertyName == TEXT("UpRatio"))
	{
		for (int32 GearIdx = 0; GearIdx < TransmissionSetup.ForwardGears.Num(); ++GearIdx)
		{
			FTankGearData & GearData = TransmissionSetup.ForwardGears[GearIdx];
			GearData.UpRatio = FMath::Max(GearData.DownRatio, GearData.UpRatio);
		}
	}
	else if (PropertyName == TEXT("SteeringCurve"))
	{
		//make sure values are capped between 0 and 1
		TArray<FRichCurveKey> SteerKeys = SteeringCurve.GetRichCurve()->GetCopyOfKeys();
		for (int32 KeyIdx = 0; KeyIdx < SteerKeys.Num(); ++KeyIdx)
		{
			float NewValue = FMath::Clamp(SteerKeys[KeyIdx].Value, 0.f, 1.f);
			SteeringCurve.GetRichCurve()->UpdateOrAddKey(SteerKeys[KeyIdx].Time, NewValue);
		}
	}

}
#endif //WITH_EDITOR

float FTankEngineData::FindPeakTorque() const
{
// Find max torque
float PeakTorque = 0.f;
TArray<FRichCurveKey> TorqueKeys = TorqueCurve.GetRichCurveConst()->GetCopyOfKeys();
for (int32 KeyIdx = 0; KeyIdx < TorqueKeys.Num(); KeyIdx++)
{
FRichCurveKey& Key = TorqueKeys[KeyIdx];
PeakTorque = FMath::Max(PeakTorque, Key.Value);
}
return PeakTorque;
}

static void GetTankDriveSetup(const FTankControlData& Setup, PxVehicleDriveTank* PxSetup)
{
switch (Setup.TankControlModel) {
	case TankDriveControlModel::STANDARD:
		PxSetup->setDriveModel(PxVehicleDriveTankControlModel::eSTANDARD);
		break;
	case TankDriveControlModel::SPECIAL:
		PxSetup->setDriveModel(PxVehicleDriveTankControlModel::eSPECIAL);
		break;
	default:
		PxSetup->setDriveModel(PxVehicleDriveTankControlModel::eSTANDARD);
		break;
	}

}

static void GetTankVehicleEngineSetup(const FTankEngineData& Setup, PxVehicleEngineData& PxSetup)
{
	PxSetup.mMOI = M2ToCm2(Setup.MOI);
	PxSetup.mMaxOmega = RPMToOmega(Setup.MaxRPM);
	PxSetup.mDampingRateFullThrottle = M2ToCm2(Setup.DampingRateFullThrottle);
	PxSetup.mDampingRateZeroThrottleClutchEngaged = M2ToCm2(Setup.DampingRateZeroThrottleClutchEngaged);
	PxSetup.mDampingRateZeroThrottleClutchDisengaged = M2ToCm2(Setup.DampingRateZeroThrottleClutchDisengaged);

	float PeakTorque = Setup.FindPeakTorque(); // In Nm
	PxSetup.mPeakTorque = M2ToCm2(PeakTorque);	// convert Nm to (kg cm^2/s^2)

												// Convert from our curve to PhysX
	PxSetup.mTorqueCurve.clear();
	TArray<FRichCurveKey> TorqueKeys = Setup.TorqueCurve.GetRichCurveConst()->GetCopyOfKeys();
	int32 NumTorqueCurveKeys = FMath::Min<int32>(TorqueKeys.Num(), PxVehicleEngineData::eMAX_NB_ENGINE_TORQUE_CURVE_ENTRIES);
	for (int32 KeyIdx = 0; KeyIdx < NumTorqueCurveKeys; KeyIdx++)
	{
		FRichCurveKey& Key = TorqueKeys[KeyIdx];
		PxSetup.mTorqueCurve.addPair(FMath::Clamp(Key.Time / Setup.MaxRPM, 0.f, 1.f), Key.Value / PeakTorque); // Normalize torque to 0-1 range
	}
}

static void GetTankVehicleGearSetup(const FTankTransmissionData& Setup, PxVehicleGearsData& PxSetup)
{
	PxSetup.mSwitchTime = Setup.GearSwitchTime;
	PxSetup.mRatios[PxVehicleGearsData::eREVERSE] = Setup.ReverseGearRatio;
	for (int32 i = 0; i < Setup.ForwardGears.Num(); i++)
	{
		PxSetup.mRatios[i + PxVehicleGearsData::eFIRST] = Setup.ForwardGears[i].Ratio;
	}
	PxSetup.mFinalRatio = Setup.FinalRatio;
	PxSetup.mNbRatios = Setup.ForwardGears.Num() + PxVehicleGearsData::eFIRST;
}

static void GetTankVehicleAutoBoxSetup(const FTankTransmissionData& Setup, PxVehicleAutoBoxData& PxSetup)
{
	for (int32 i = 0; i < Setup.ForwardGears.Num(); i++)
	{
		const FTankGearData& GearData = Setup.ForwardGears[i];
		PxSetup.mUpRatios[i] = GearData.UpRatio;
		PxSetup.mDownRatios[i] = GearData.DownRatio;
	}
	PxSetup.mUpRatios[PxVehicleGearsData::eNEUTRAL] = Setup.NeutralGearUpRatio;
	PxSetup.setLatency(Setup.GearAutoBoxLatency);
}

void SetupTankDriveHelper(const UWheeledVehicleMovementComponentTank* VehicleData, const PxVehicleWheelsSimData* PWheelsSimData, PxVehicleDriveSimData& DriveData)
{

	PxVehicleEngineData EngineSetup;
	GetTankVehicleEngineSetup(VehicleData->EngineSetup, EngineSetup);
	DriveData.setEngineData(EngineSetup);

	PxVehicleClutchData ClutchSetup;
	ClutchSetup.mStrength = M2ToCm2(VehicleData->TransmissionSetup.ClutchStrength);
	DriveData.setClutchData(ClutchSetup);

	PxVehicleGearsData GearSetup;
	GetTankVehicleGearSetup(VehicleData->TransmissionSetup, GearSetup);
	DriveData.setGearsData(GearSetup);

	PxVehicleAutoBoxData AutoBoxSetup;
	GetTankVehicleAutoBoxSetup(VehicleData->TransmissionSetup, AutoBoxSetup);
	DriveData.setAutoBoxData(AutoBoxSetup);

}


void UWheeledVehicleMovementComponentTank::SetupVehicleDrive(PxVehicleWheelsSimData* PWheelsSimData)
{
	if (WheelSetups.Num() % 2 != 0 && WheelSetups.Num() > 20)
	{
		PVehicle = nullptr;
		PVehicleDrive = nullptr;
		return;
	}

	// Setup drive data
	PxVehicleDriveSimData DriveData;
	SetupTankDriveHelper(this, PWheelsSimData, DriveData);

	// Create the vehicle
	PxVehicleDriveTank* PVehicleDriveTank = PxVehicleDriveTank::allocate(WheelSetups.Num());
	check(PVehicleDriveTank);

	// Set drive model
	GetTankDriveSetup(this->TankControlSetup, PVehicleDriveTank);

	ExecuteOnPxRigidDynamicReadWrite(UpdatedPrimitive->GetBodyInstance(), [&](PxRigidDynamic* PRigidDynamic)
	{
		PVehicleDriveTank->setup(GPhysXSDK, PRigidDynamic, *PWheelsSimData, DriveData, WheelSetups.Num());
		PVehicleDriveTank->setToRestState();

		// cleanup
		PWheelsSimData->free();
	});

	// cache values
	PVehicle = PVehicleDriveTank;
	PVehicleDrive = PVehicleDriveTank;

	SetUseAutoGears(TransmissionSetup.bUseGearAutoBox);
}

void UWheeledVehicleMovementComponentTank::UpdateSimulation(float DeltaTime)
{

	if (PVehicleDrive == NULL)
		return;

	UpdatedPrimitive->GetBodyInstance()->ExecuteOnPhysicsReadWrite([&]
	{

		PxVehicleDriveTankRawInputData VehicleInputData((PxVehicleDriveTankControlModel::Enum)TankControlSetup.TankControlModel);


		VehicleInputData.setAnalogAccel(ThrottleInput);


		// Turning to left while no accel
		if (SteeringInput > 0 && ThrottleInput == 0)
		{
			VehicleInputData.setAnalogAccel(1.0f);
			VehicleInputData.setAnalogLeftThrust(SteeringInput);
			VehicleInputData.setAnalogRightBrake(1.0f);
		}

		// Turning to left while accelerating
		if (SteeringInput > 0 && ThrottleInput > 0)
		{
			VehicleInputData.setAnalogLeftThrust(SteeringInput);
			VehicleInputData.setAnalogRightThrust(SteeringInput / 3);
		}

		// Turning to left while braking
		if (SteeringInput > 0 && ThrottleInput == 0 && BrakeInput > 0)
		{
			VehicleInputData.setAnalogAccel(SteeringInput);
			VehicleInputData.setAnalogLeftThrust(SteeringInput);
			VehicleInputData.setAnalogRightBrake(BrakeInput);
		}
		/**
		// Turning to right while no accel
		else if (RawSteeringInput < 0 && ThrottleInput == 0)
		{
			VehicleInputData.setAnalogLeftBrake(RawLeftBrake);
			VehicleInputData.setAnalogRightBrake(RawRightBrake);
			VehicleInputData.setAnalogAccel(FMath::Square(RawSteeringInput));
			VehicleInputData.setAnalogRightThrust(FMath::Square(RawSteeringInput));
			VehicleInputData.setAnalogLeftThrust(0.0001);
		}

		// Turning to right while accelerating
		else if (RawSteeringInput < 0 && ThrottleInput > 0)
		{
			VehicleInputData.setAnalogLeftBrake(RawLeftBrake);
			VehicleInputData.setAnalogRightBrake(RawRightBrake);
			VehicleInputData.setAnalogAccel(ThrottleInput);
			VehicleInputData.setAnalogRightThrust(FMath::Square(RawSteeringInput));
		}

		// Turning to right while braking
		else if (RawSteeringInput < 0 && ThrottleInput == 0 && RawLeftBrake > 0 && RawRightBrake > 0)
		{
			VehicleInputData.setAnalogAccel(FMath::Square(RawSteeringInput));
			VehicleInputData.setAnalogRightThrust(FMath::Square(RawSteeringInput));
			VehicleInputData.setAnalogLeftBrake(RawLeftBrake);
		}
		else if (PxVehicleGearsData::eREVERSE == GetCurrentGear() && ThrottleInput > 0 && RawSteeringInput > 0)
		{ 
		
		}
		else if (PxVehicleGearsData::eREVERSE == GetCurrentGear() && ThrottleInput > 0 && RawSteeringInput < 0)
		{

		};


		/**
		VehicleInputData.setAnalogAccel(ThrottleInput);
		if (SteeringInput > 0)
			{
				VehicleInputData.setAnalogAccel(SteeringInput);
				VehicleInputData.setAnalogLeftThrust(SteeringInput);
				VehicleInputData.setAnalogRightThrust(SteeringInput/3);

			}
			else if (SteeringInput < 0)
			{
				VehicleInputData.setAnalogAccel(FMath::Square(SteeringInput));
				VehicleInputData.setAnalogRightThrust(FMath::Square(SteeringInput));
				VehicleInputData.setAnalogLeftThrust(FMath::Square(SteeringInput) / 3);

			}

			VehicleInputData.setAnalogLeftBrake(BrakeInput);
			VehicleInputData.setAnalogRightBrake(BrakeInput);

		
		/**
		switch (TankControlSetup.TankControlModel) {
			case TankDriveControlModel::STANDARD:

				switch (TankControlSetup.TankControlMethod) {
					case TankDriveControlMethod::SingleStick:

						/**
						Get X and Y from the Joystick, do whatever scaling and calibrating you need to do based on your hardware.
						Invert X
						Calculate R+L (Call it V): V =(100-ABS(X)) * (Y/100) + Y
						Calculate R-L (Call it W): W= (100-ABS(Y)) * (X/100) + X
						Calculate R (thrust): R = (V+W) /2
						Calculate L (thrust): L= (V-W)/2
						
						float scaledSteering;
						scaledSteering = SteeringInput * 100;
						float scaledThrottle;
						scaledThrottle = ThrottleInput * 100;
						float invSteering;
						invSteering = -scaledSteering;
						float v1;
						v1 = (100 - FMath::Abs(invSteering)) * (scaledThrottle / 100) + scaledThrottle;
						float w1; 
						w1 = (100 - FMath::Abs(scaledThrottle))* (invSteering / 100) + invSteering;
						float r1; 
						r1= (v1 + w1) / 2;
						float l1;
						l1 = (v1 - w1) / 2;


						VehicleInputData.setAnalogLeftThrust(l1 / 100);
						VehicleInputData.setAnalogRightThrust(r1 / 100);
						VehicleInputData.setAnalogLeftBrake(BrakeInput);
						VehicleInputData.setAnalogRightBrake(BrakeInput);

						break;
					case TankDriveControlMethod::DualStick:

						VehicleInputData.setAnalogAccel(Acceleration);
						 
						if (RawSteeringInput < 0 && RawThrottleInput < 0) {
							 //VehicleInputData.setAnalogAccel(1.0f);
						     //RawLeftThrust = FMath::Square(RawSteeringInput);
						     VehicleInputData.setAnalogLeftThrust(RawLeftThrust);
							 //RawRightBrake = FMath::Square(RawThrottleInput);
							 VehicleInputData.setAnalogRightBrake(RawRightBrake);
						} else if (RawSteeringInput > 0 && RawThrottleInput < 0) {
							//VehicleInputData.setAnalogAccel(1.0f);
							//RawRightThrust = RawSteeringInput;
							VehicleInputData.setAnalogRightThrust(RawRightThrust);
							//RawLeftBrake = FMath::Square(RawThrottleInput);
							VehicleInputData.setAnalogLeftBrake(RawLeftBrake);
						}  else if (RawSteeringInput < 0 ) {
							//VehicleInputData.setAnalogAccel(1.0f);
						    //RawLeftThrust = FMath::Square(RawSteeringInput);
						    VehicleInputData.setAnalogLeftThrust(RawLeftThrust);
							VehicleInputData.setAnalogRightThrust(0.3f);
						} else if (RawSteeringInput > 0) {
							//VehicleInputData.setAnalogAccel(1.0f);
							//RawRightThrust = RawSteeringInput;
							VehicleInputData.setAnalogRightThrust(RawRightThrust);
							VehicleInputData.setAnalogLeftThrust(0.3f);
						} else if (RawSteeringInput < 0) {
							//VehicleInputData.setAnalogAccel(1.0f);
							//RawLeftThrust = FMath::Square(RawSteeringInput);
							VehicleInputData.setAnalogLeftThrust(RawLeftThrust);
							VehicleInputData.setAnalogRightThrust(0.3f);
							} else {};
										 
						VehicleInputData.setAnalogLeftThrust(RawLeftThrust);
						VehicleInputData.setAnalogRightThrust(RawRightThrust);
						VehicleInputData.setAnalogLeftBrake(RawLeftBrake);
						VehicleInputData.setAnalogRightBrake(RawRightBrake);

						break;
					default:
						break;
					}

				break;
			case TankDriveControlModel::SPECIAL:

				VehicleInputData.setAnalogAccel(Acceleration);
				VehicleInputData.setAnalogLeftThrust(RawLeftThrust);
				VehicleInputData.setAnalogRightThrust(RawRightThrust);
				VehicleInputData.setAnalogLeftBrake(BrakeInput);
				VehicleInputData.setAnalogRightBrake(BrakeInput);

				break;
			default:
				break;
			} */

		if (!PVehicleDrive->mDriveDynData.getUseAutoGears())
		{
			VehicleInputData.setGearUp(bRawGearUpInput);
			VehicleInputData.setGearDown(bRawGearDownInput);
		}

		// Convert from our curve to PxFixedSizeLookupTable

		
		PxVehiclePadSmoothingData SmoothData = {
			{ ThrottleInputRate.RiseRate, TankControlSetup.LeftBrakeRate.RiseRate, TankControlSetup.RightBrakeRate.RiseRate, TankControlSetup.LeftThrustRate.RiseRate, TankControlSetup.RightThrustRate.RiseRate },
			{ ThrottleInputRate.FallRate, TankControlSetup.LeftBrakeRate.FallRate, TankControlSetup.RightBrakeRate.FallRate, TankControlSetup.LeftThrustRate.FallRate, TankControlSetup.RightThrustRate.FallRate }
		};
		
		PxVehicleDriveTank* PVehicleDriveTank = (PxVehicleDriveTank*)PVehicleDrive;
		PxVehicleDriveTankSmoothAnalogRawInputsAndSetAnalogInputs(SmoothData, VehicleInputData, DeltaTime, *PVehicleDriveTank);

		//LeftTrackSpeed = PVehicle->mWheelsDynData.getWheelRotationSpeed(0) / 1000.f;
		//RightTrackSpeed = PVehicle->mWheelsDynData.getWheelRotationSpeed(1) / 1000.f;
	});
}

void UWheeledVehicleMovementComponentTank::Serialize(FArchive & Ar)
{
	Super::Serialize(Ar);
	if (Ar.IsLoading() && Ar.UE4Ver() < VER_UE4_VEHICLES_UNIT_CHANGE)
	{
		PxVehicleEngineData DefEngineData;
		float DefaultRPM = OmegaToRPM(DefEngineData.mMaxOmega);

		//we need to convert from old units to new. This backwards compatable code fails in the rare case that they were using very strange values that are the new defaults in the correct units.
		EngineSetup.MaxRPM = EngineSetup.MaxRPM != DefaultRPM ? OmegaToRPM(EngineSetup.MaxRPM) : DefaultRPM;	//need to convert from rad/s to RPM
	}

	if (Ar.IsLoading() && Ar.UE4Ver() < VER_UE4_VEHICLES_UNIT_CHANGE2)
	{
		PxVehicleEngineData DefEngineData;
		PxVehicleClutchData DefClutchData;

		//we need to convert from old units to new. This backwards compatable code fails in the rare case that they were using very strange values that are the new defaults in the correct units.
		BackwardsConvertCm2ToM2(EngineSetup.DampingRateFullThrottle, DefEngineData.mDampingRateFullThrottle);
		BackwardsConvertCm2ToM2(EngineSetup.DampingRateZeroThrottleClutchDisengaged, DefEngineData.mDampingRateZeroThrottleClutchDisengaged);
		BackwardsConvertCm2ToM2(EngineSetup.DampingRateZeroThrottleClutchEngaged, DefEngineData.mDampingRateZeroThrottleClutchEngaged);
		BackwardsConvertCm2ToM2(EngineSetup.MOI, DefEngineData.mMOI);
		BackwardsConvertCm2ToM2(TransmissionSetup.ClutchStrength, DefClutchData.mStrength);
	}
}

void UWheeledVehicleMovementComponentTank::ComputeConstants()
{
	Super::ComputeConstants();
	MaxEngineRPM = EngineSetup.MaxRPM;
}

void UWheeledVehicleMovementComponentTank::SetLeftThrottleInput(float LeftThrottle) {
	RawLeftThrust = FMath::Clamp(LeftThrottle, 0.0f, 1.0f);
	}

void UWheeledVehicleMovementComponentTank::SetRightThrottleInput(float RightThrottle) {
	RawRightThrust = FMath::Clamp(RightThrottle, 0.0f, 1.0f);
	}

void UWheeledVehicleMovementComponentTank::SetLeftBrakeInput(float LeftBrake) {
	RawLeftBrake = FMath::Clamp(LeftBrake, 0.0f, 1.0f);
	}

void UWheeledVehicleMovementComponentTank::SetRightBrakeInput(float RightBrake) {
	RawRightBrake = FMath::Clamp(RightBrake, 0.0f, 1.0f);
	}

void UWheeledVehicleMovementComponentTank::SetAccelInput(float Accel) {
	Acceleration = FMath::Clamp(Accel, 0.0f, 1.0f);
	}
