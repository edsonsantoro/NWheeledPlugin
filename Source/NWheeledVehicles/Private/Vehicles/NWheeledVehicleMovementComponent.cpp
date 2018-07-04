// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "NWheeledVehiclesPluginPrivatePCH.h"
#include "Vehicles/NWheeledVehicleMovementComponent.h"
#include "EngineGlobals.h"
#include "Vehicles/AdvancedVehicleWheel.h"
#include "WheeledVehicleMovementComponent.h"
#include "VehicleWheel.h"
#include "TireConfig.h"

#include "PhysXPublic.h"
#include "PhysXVehicleManager.h"

#define LOCTEXT_NAMESPACE "UNWheeledVehicleMovementComponent"

void NTireShader(const void* shaderData, const PxF32 tireFriction,
	const PxF32 longSlip, const PxF32 latSlip,
	const PxF32 camber, const PxF32 wheelOmega, const PxF32 wheelRadius, const PxF32 recipWheelRadius,
	const PxF32 restTireLoad, const PxF32 normalisedTireLoad, const PxF32 tireLoad,
	const PxF32 gravity, const PxF32 recipGravity,
	PxF32& wheelTorque, PxF32& tireLongForceMag, PxF32& tireLatForceMag, PxF32& tireAlignMoment)
{
	UVehicleWheel* Wheel = (UVehicleWheel*)shaderData;
	UAdvancedVehicleWheel* AdvancedWheel = (UAdvancedVehicleWheel*)shaderData;

	FTireShaderInputNW Input;
	FTireShaderInput DefaultInput;

	Input.TireFriction = tireFriction;
	Input.LongSlip = longSlip;
	Input.LatSlip = latSlip;
	Input.Camber = camber;
	Input.WheelOmega = wheelOmega;
	Input.WheelRadius = wheelRadius;
	Input.RecipWheelRadius = recipWheelRadius;
	Input.NormalizedTireLoad = normalisedTireLoad;
	Input.RestTireLoad = restTireLoad;
	Input.TireLoad = tireLoad;
	Input.Gravity = gravity;
	Input.RecipGravity = recipGravity;

	DefaultInput.TireFriction = tireFriction;
	DefaultInput.LongSlip = longSlip;
	DefaultInput.LatSlip = latSlip;
	DefaultInput.WheelOmega = wheelOmega;
	DefaultInput.WheelRadius = wheelRadius;
	DefaultInput.RecipWheelRadius = recipWheelRadius;
	DefaultInput.NormalizedTireLoad = normalisedTireLoad;
	DefaultInput.RestTireLoad = restTireLoad;
	DefaultInput.TireLoad = tireLoad;
	DefaultInput.Gravity = gravity;
	DefaultInput.RecipGravity = recipGravity;


	FTireShaderOutputNW Output(0.0f);
	FTireShaderOutput DefaultOutput(0.0f);

	UWheeledVehicleMovementComponent* inVehicle = AdvancedWheel->VehicleSim;
	UNWheeledVehicleMovementComponent* Vehicle = Cast<UNWheeledVehicleMovementComponent>(inVehicle);

	AdvancedWheel != nullptr ? Vehicle->GenerateFullTireForces(Wheel, Input, Output) : Wheel->VehicleSim->GenerateTireForces(Wheel, DefaultInput, DefaultOutput);

	if (AdvancedWheel != nullptr)
	{
		//AdvancedWheel->VehicleNWSim->GenerateTireForces(AdvancedWheel, Input, Output);
		wheelTorque = Output.WheelTorque;
		tireLongForceMag = Output.LongForce;
		tireLatForceMag = Output.LatForce;
	}
	else
	{
		//Wheel->VehicleSim->GenerateTireForces(Wheel, DefaultInput, DefaultOutput);
		wheelTorque = DefaultOutput.WheelTorque;
		tireLongForceMag = DefaultOutput.LongForce;
		tireLatForceMag = DefaultOutput.LatForce;
	}


	/**
	if ( Wheel->bDebugWheels true)
	{
		AdvancedWheel->DebugLongSlip = longSlip;
		AdvancedWheel->DebugLatSlip = latSlip;
		AdvancedWheel->DebugNormalizedTireLoad = normalisedTireLoad;
		AdvancedWheel->DebugTireLoad = tireLoad;
		AdvancedWheel->DebugWheelTorque = wheelTorque;
		AdvancedWheel->DebugLongForce = tireLongForceMag;
		AdvancedWheel->DebugLatForce = tireLatForceMag;
	}*/
}


UNWheeledVehicleMovementComponent::UNWheeledVehicleMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Mass = 1500.0f;
	DragCoefficient = 0.3f;
	ChassisWidth = 180.f;
	ChassisHeight = 140.f;
	InertiaTensorScale = FVector(1.0f, 1.0f, 1.0f);
	AngErrorAccumulator = 0.0f;
	MinNormalizedTireLoad = 0.0f;
	MaxNormalizedTireLoad = 10.0f;

	IdleBrakeInput = 0.0f;
	StopThreshold = 10.0f;
	WrongDirectionThreshold = 100.f;
	ThrottleInputRate.RiseRate = 6.0f;
	ThrottleInputRate.FallRate = 10.0f;
	BrakeInputRate.RiseRate = 6.0f;
	BrakeInputRate.FallRate = 10.0f;
	HandbrakeInputRate.RiseRate = 12.0f;
	HandbrakeInputRate.FallRate = 12.0f;
	SteeringInputRate.RiseRate = 2.5f;
	SteeringInputRate.FallRate = 5.0f;

	bDeprecatedSpringOffsetMode = false;	//This is just for backwards compat. Newly tuned content should never need to use this

	bUseRVOAvoidance = false;
	AvoidanceVelocity = FVector::ZeroVector;
	AvoidanceLockVelocity = FVector::ZeroVector;
	AvoidanceLockTimer = 0.0f;
	AvoidanceGroup.bGroup0 = true;
	GroupsToAvoid.Packed = 0xFFFFFFFF;
	GroupsToIgnore.Packed = 0;
	RVOAvoidanceRadius = 400.0f;
	RVOAvoidanceHeight = 200.0f;
	AvoidanceConsiderationRadius = 2000.0f;
	RVOSteeringStep = 0.5f;
	RVOThrottleStep = 0.25f;

	ThresholdLongitudinalSpeed = 5.f;
	LowForwardSpeedSubStepCount = 3;
	HighForwardSpeedSubStepCount = 1;

	bReverseAsBrake = true;	//Treats reverse button as break for a more arcade feel (also automatically goes into reverse)

	// tire load filtering
	PxVehicleTireLoadFilterData PTireLoadFilterDef;
	MinNormalizedTireLoad = PTireLoadFilterDef.mMinNormalisedLoad;
	MinNormalizedTireLoadFiltered = PTireLoadFilterDef.mMinFilteredNormalisedLoad;
	MaxNormalizedTireLoad = PTireLoadFilterDef.mMaxNormalisedLoad;
	MaxNormalizedTireLoadFiltered = PTireLoadFilterDef.mMaxFilteredNormalisedLoad;

}

void UNWheeledVehicleMovementComponent::SetupWheels(PxVehicleWheelsSimData* PWheelsSimData)
{
	if (!UpdatedPrimitive)
	{
		return;
	}

	ExecuteOnPxRigidDynamicReadWrite(UpdatedPrimitive->GetBodyInstance(), [&](PxRigidDynamic* PVehicleActor)
	{

		const PxReal LengthScale = 100.f; // Convert default from m to cm

		// Control substepping
		PWheelsSimData->setSubStepCount(ThresholdLongitudinalSpeed * LengthScale, LowForwardSpeedSubStepCount, HighForwardSpeedSubStepCount);
		PWheelsSimData->setMinLongSlipDenominator(4.f * LengthScale);

		// Prealloc data for the sprung masses
		PxVec3 WheelOffsets[32];
		float SprungMasses[32];

		const int32 NumWheels = FMath::Min(32, WheelSetups.Num());

		// Calculate wheel offsets first, necessary for sprung masses
		for (int32 WheelIdx = 0; WheelIdx < NumWheels; ++WheelIdx)
		{
			WheelOffsets[WheelIdx] = U2PVector(GetWheelRestingPosition(WheelSetups[WheelIdx]));
		}

		// Now that we have all the wheel offsets, calculate the sprung masses
		PxVec3 PLocalCOM = U2PVector(GetLocalCOM());
		PxVehicleComputeSprungMasses(NumWheels, WheelOffsets, PLocalCOM, PVehicleActor->getMass(), 2, SprungMasses);

		for (int32 WheelIdx = 0; WheelIdx < NumWheels; ++WheelIdx)
		{
			UVehicleWheel* Wheel = WheelSetups[WheelIdx].WheelClass.GetDefaultObject();
			UAdvancedVehicleWheel* AdvancedWheel = Cast<UAdvancedVehicleWheel>(Wheel);

			// init wheel data
			PxVehicleWheelData PWheelData;
			PWheelData.mRadius = Wheel->ShapeRadius;
			PWheelData.mWidth = Wheel->ShapeWidth;
			PWheelData.mMaxSteer = FMath::DegreesToRadians(Wheel->SteerAngle);
			PWheelData.mMaxBrakeTorque = M2ToCm2(Wheel->MaxBrakeTorque);
			PWheelData.mMaxHandBrakeTorque = Wheel->bAffectedByHandbrake ? M2ToCm2(Wheel->MaxHandBrakeTorque) : 0.0f;

			PWheelData.mDampingRate = M2ToCm2(Wheel->DampingRate);
			PWheelData.mMass = Wheel->Mass;
			PWheelData.mMOI = 0.5f * PWheelData.mMass * FMath::Square(PWheelData.mRadius);

			// init tire data
			PxVehicleTireData PTireData;

			if (Wheel->TireConfig == nullptr)
			{
				Wheel->TireConfig = NewObject<UTireConfig>();
				Wheel->TireConfig->AddToRoot(); // prevent GC
				PTireData.mType = Wheel->TireConfig->GetTireConfigID();
			}
			else
			{
				PTireData.mType = Wheel->TireConfig->GetTireConfigID();
			}

			AdvancedWheel != NULL ? PTireData.mCamberStiffnessPerUnitGravity = AdvancedWheel->CamberStiffnessPerUnitGravity : 0.0f;
			//PTireData.mCamberStiffnessPerUnitGravity = 0.0f;
			PTireData.mLatStiffX = Wheel->LatStiffMaxLoad;
			PTireData.mLatStiffY = Wheel->LatStiffValue;
			PTireData.mLongitudinalStiffnessPerUnitGravity = Wheel->LongStiffValue;

			// init suspension data
			PxVehicleSuspensionData PSuspensionData;
			PSuspensionData.mSprungMass = SprungMasses[WheelIdx];
			PSuspensionData.mMaxCompression = Wheel->SuspensionMaxRaise;
			PSuspensionData.mMaxDroop = Wheel->SuspensionMaxDrop;
			if (AdvancedWheel != NULL){
				if (AdvancedWheel->SuspensionAutoStrength == true) {
					PSuspensionData.mSpringStrength = FMath::Square(Wheel->SuspensionNaturalFrequency) * PSuspensionData.mSprungMass;
					AdvancedWheel->SuspensionSpringStrength = PSuspensionData.mSpringStrength;
					AdvancedWheel->DamperRate = Wheel->SuspensionDampingRatio * 2.0f * FMath::Sqrt(PSuspensionData.mSpringStrength * PSuspensionData.mSprungMass);
				} else {
					PSuspensionData.mSpringStrength = AdvancedWheel->SuspensionSpringStrength;
					PSuspensionData.mSpringDamperRate = AdvancedWheel->DamperRate;
				}
				PSuspensionData.mCamberAtRest = FMath::DegreesToRadians(AdvancedWheel->SuspensionCamberAtRest);
				PSuspensionData.mCamberAtMaxCompression = FMath::DegreesToRadians(AdvancedWheel->SuspensionCamberAtMaxCompression);
				PSuspensionData.mCamberAtMaxDroop = FMath::DegreesToRadians(AdvancedWheel->SuspensionCamberAtMaxDroop);
			}
			else
			{
				PSuspensionData.mSpringStrength = FMath::Square(Wheel->SuspensionNaturalFrequency) * PSuspensionData.mSprungMass;
				PSuspensionData.mSpringDamperRate = Wheel->SuspensionDampingRatio * 2.0f * FMath::Sqrt(PSuspensionData.mSpringStrength * PSuspensionData.mSprungMass);

			}

			// init offsets
			const PxVec3 PWheelOffset = WheelOffsets[WheelIdx];

			PxVec3 PSuspTravelDirection = PxVec3(0.0f, 0.0f, -1.0f);
			PxVec3 PWheelCentreCMOffset = PWheelOffset - PLocalCOM;
			PxVec3 PSuspForceAppCMOffset = !bDeprecatedSpringOffsetMode ? PxVec3(PWheelCentreCMOffset.x, PWheelCentreCMOffset.y, Wheel->SuspensionForceOffset + PWheelCentreCMOffset.z)
				: PxVec3(PWheelCentreCMOffset.x, PWheelCentreCMOffset.y, Wheel->SuspensionForceOffset);
			PxVec3 PTireForceAppCMOffset = PSuspForceAppCMOffset;

			// finalize sim data
			PWheelsSimData->setWheelData(WheelIdx, PWheelData);
			PWheelsSimData->setTireData(WheelIdx, PTireData);
			PWheelsSimData->setSuspensionData(WheelIdx, PSuspensionData);
			PWheelsSimData->setSuspTravelDirection(WheelIdx, PSuspTravelDirection);
			PWheelsSimData->setWheelCentreOffset(WheelIdx, PWheelCentreCMOffset);
			PWheelsSimData->setSuspForceAppPointOffset(WheelIdx, PSuspForceAppCMOffset);
			PWheelsSimData->setTireForceAppPointOffset(WheelIdx, PTireForceAppCMOffset);
		}

		const int32 NumShapes = PVehicleActor->getNbShapes();
		const int32 NumChassisShapes = NumShapes - NumWheels;
		if (NumChassisShapes >= 1)
		{

		TArray<PxShape*> Shapes;
		Shapes.AddZeroed(NumShapes);

		PVehicleActor->getShapes(Shapes.GetData(), NumShapes);

		for (int32 WheelIdx = 0; WheelIdx < NumWheels; ++WheelIdx)
		{
			const int32 WheelShapeIndex = NumChassisShapes + WheelIdx;

			PWheelsSimData->setWheelShapeMapping(WheelIdx, WheelShapeIndex);
			PWheelsSimData->setSceneQueryFilterData(WheelIdx, Shapes[WheelShapeIndex]->getQueryFilterData());
		}
		}
		else
		{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			UE_LOG(LogPhysics, Warning, TEXT("Missing wheel shapes. Please ensure there's a body associated with each wheel, or deselect Don'tCreateShape in your wheel class for vehicle''%s''"), *GetPathNameSafe(this));
#endif
		}

		// tire load filtering
		PxVehicleTireLoadFilterData PTireLoadFilter;
		PTireLoadFilter.mMinNormalisedLoad = MinNormalizedTireLoad;
		PTireLoadFilter.mMinFilteredNormalisedLoad = MinNormalizedTireLoadFiltered;
		PTireLoadFilter.mMaxNormalisedLoad = MaxNormalizedTireLoad;
		PTireLoadFilter.mMaxFilteredNormalisedLoad = MaxNormalizedTireLoadFiltered;
		PWheelsSimData->setTireLoadFilterData(PTireLoadFilter);
	});
}

#define ONE_TWENTYSEVENTH 0.037037f
#define ONE_THIRD 0.33333f
PX_FORCE_INLINE PxF32 smoothingFunction1(const PxF32 K)
{
	//Equation 20 in CarSimEd manual Appendix F.
	//Looks a bit like a curve of sqrt(x) for 0<x<1 but reaching 1.0 on y-axis at K=3. 
	PX_ASSERT(K >= 0.0f);
	return PxMin(1.0f, K - ONE_THIRD*K*K + ONE_TWENTYSEVENTH*K*K*K);
}
PX_FORCE_INLINE PxF32 smoothingFunction2(const PxF32 K)
{
	//Equation 21 in CarSimEd manual Appendix F.
	//Rises to a peak at K=0.75 and falls back to zero by K=3
	PX_ASSERT(K >= 0.0f);
	return (K - K*K + ONE_THIRD*K*K*K - ONE_TWENTYSEVENTH*K*K*K*K);
}

void PxVehicleComputeTireForceDefault
(const void* tireShaderData,
	const PxF32 tireFriction,
	const PxF32 longSlip, const PxF32 latSlip, const PxF32 camber,
	const PxF32 wheelOmega, const PxF32 wheelRadius, const PxF32 recipWheelRadius,
	const PxF32 restTireLoad, const PxF32 normalisedTireLoad, const PxF32 tireLoad,
	const PxF32 gravity, const PxF32 recipGravity,
	PxF32& wheelTorque, PxF32& tireLongForceMag, PxF32& tireLatForceMag, PxF32& tireAlignMoment)
{
	PX_UNUSED(wheelOmega);
	PX_UNUSED(recipWheelRadius);

	const PxVehicleTireData& tireData = *((PxVehicleTireData*)tireShaderData);

	PX_ASSERT(tireFriction>0);
	PX_ASSERT(tireLoad>0);

	wheelTorque = 0.0f;
	tireLongForceMag = 0.0f;
	tireLatForceMag = 0.0f;
	tireAlignMoment = 0.0f;

	//If long slip/lat slip/camber are all zero than there will be zero tire force.
	if (FMath::IsNearlyZero(latSlip) && FMath::IsNearlyZero(longSlip) && FMath::IsNearlyZero(camber))
	{
		return;
	}

	//Compute the lateral stiffness
	const PxF32 latStiff = restTireLoad*tireData.mLatStiffY*smoothingFunction1(normalisedTireLoad*3.0f / tireData.mLatStiffX);

	//Get the longitudinal stiffness
	const PxF32 longStiff = tireData.mLongitudinalStiffnessPerUnitGravity*gravity;
	const PxF32 recipLongStiff = tireData.getRecipLongitudinalStiffnessPerUnitGravity()*recipGravity;

	//Get the camber stiffness.
	const PxF32 camberStiff = tireData.mCamberStiffnessPerUnitGravity*gravity;

	//Carry on and compute the forces.
	const PxF32 TEff = PxTan(latSlip - camber*camberStiff / latStiff);
	const PxF32 K = PxSqrt(latStiff*TEff*latStiff*TEff + longStiff*longSlip*longStiff*longSlip) / (tireFriction*tireLoad);
	//const PxF32 KAbs=PxAbs(K);
	PxF32 FBar = smoothingFunction1(K);//K - ONE_THIRD*PxAbs(K)*K + ONE_TWENTYSEVENTH*K*K*K;
	PxF32 MBar = smoothingFunction2(K); //K - KAbs*K + ONE_THIRD*K*K*K - ONE_TWENTYSEVENTH*KAbs*K*K*K;
										//Mbar = PxMin(Mbar, 1.0f);
	PxF32 nu = 1;
	if (K <= 2.0f*PxPi)
	{
		const PxF32 latOverlLong = latStiff*recipLongStiff;
		nu = 0.5f*(1.0f + latOverlLong - (1.0f - latOverlLong)*PxCos(K*0.5f));
	}
	const PxF32 FZero = tireFriction*tireLoad / (PxSqrt(longSlip*longSlip + nu*TEff*nu*TEff));
	const PxF32 fz = longSlip*FBar*FZero;
	const PxF32 fx = -nu*TEff*FBar*FZero;
	//TODO: pneumatic trail.
	const PxF32 pneumaticTrail = 1.0f;
	const PxF32	fMy = nu * pneumaticTrail * TEff * MBar * FZero;

	//We can add the torque to the wheel.
	wheelTorque = -fz*wheelRadius;
	tireLongForceMag = fz;
	tireLatForceMag = fx;
	tireAlignMoment = fMy;
}

void UNWheeledVehicleMovementComponent::GenerateFullTireForces(UVehicleWheel* Wheel, const FTireShaderInputNW& Input, FTireShaderOutputNW& Output)
{
	const void* realShaderData = &PVehicle->mWheelsSimData.getTireData(Wheel->WheelIndex);

	float Dummy;

	PxVehicleComputeTireForceDefault(
		realShaderData, Input.TireFriction,
		Input.LongSlip, Input.LatSlip,
		Input.Camber, Input.WheelOmega, Input.WheelRadius, Input.RecipWheelRadius,
		Input.RestTireLoad, Input.NormalizedTireLoad, Input.TireLoad,
		Input.Gravity, Input.RecipGravity,
		Output.WheelTorque, Output.LongForce, Output.LatForce, Dummy
		);

	ensureMsgf(Output.WheelTorque == Output.WheelTorque, TEXT("Output.WheelTorque is bad: %f"), Output.WheelTorque);
	ensureMsgf(Output.LongForce == Output.LongForce, TEXT("Output.LongForce is bad: %f"), Output.LongForce);
	ensureMsgf(Output.LatForce == Output.LatForce, TEXT("Output.LatForce is bad: %f"), Output.LatForce);
}

void UNWheeledVehicleMovementComponent::CreateWheels()
{
	// Wheels num is getting copied when blueprint recompiles, so we have to manually reset here
	Wheels.Reset();

	PVehicle->mWheelsDynData.setTireForceShaderFunction(NTireShader);

	// Instantiate the wheels
	for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx)
	{
		UVehicleWheel* Wheel = NewObject<UVehicleWheel>(this, WheelSetups[WheelIdx].WheelClass);
		check(Wheel);

		Wheels.Add(Wheel);
	}

	// Initialize the wheels
	for (int32 WheelIdx = 0; WheelIdx < Wheels.Num(); ++WheelIdx)
	{
		PVehicle->mWheelsDynData.setTireForceShaderData(WheelIdx, Wheels[WheelIdx]);

		Wheels[WheelIdx]->Init(this, WheelIdx);
	}
}
