// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "NWheeledVehiclesPluginPrivatePCH.h"
#include "Vehicles/NWheeledVehicleMovementComponent.h"
#include "Runtime/Engine/Private/EnginePrivate.h"
#include "Runtime/Engine/Public/PhysicsPublic.h"
#include "Vehicles/VehicleWheelWithCamber.h"
#include "Runtime/Engine/Classes/Vehicles/WheeledVehicleMovementComponent.h"
#include "Runtime/Engine/Classes/Vehicles/VehicleWheel.h"
#include "Runtime/Engine/Classes/Vehicles/TireType.h"


#if WITH_PHYSX
#include "Runtime/Engine/Private/PhysicsEngine/PhysXSupport.h"
#include "Runtime/Engine/Private/Collision/PhysXCollision.h"
#endif //WITH_PHYSX

#define LOCTEXT_NAMESPACE "UNWheeledVehicleMovementComponent"

#if WITH_VEHICLE

/**
* PhysX shader for tire friction forces
* tireFriction - friction value of the tire contact.
* longSlip - longitudinal slip of the tire.
* latSlip - lateral slip of the tire.
* camber - camber angle of the tire
* wheelOmega - rotational speed of the wheel.
* wheelRadius - the distance from the tire surface and the center of the wheel.
* recipWheelRadius - the reciprocal of wheelRadius.
* restTireLoad - the load force experienced by the tire when the vehicle is at rest.
* normalisedTireLoad - a value equal to the load force on the tire divided by the restTireLoad.
* tireLoad - the load force currently experienced by the tire.
* gravity - magnitude of gravitational acceleration.
* recipGravity - the reciprocal of the magnitude of gravitational acceleration.
* wheelTorque - the torque to be applied to the wheel around the wheel axle.
* tireLongForceMag - the magnitude of the longitudinal tire force to be applied to the vehicle's rigid body.
* tireLatForceMag - the magnitude of the lateral tire force to be applied to the vehicle's rigid body.
* tireAlignMoment - the aligning moment of the tire that is to be applied to the vehicle's rigid body (not currently used).
*/

void PTireShader1(const void* shaderData, const PxF32 tireFriction,
	const PxF32 longSlip, const PxF32 latSlip,
	const PxF32 camber, const PxF32 wheelOmega, const PxF32 wheelRadius, const PxF32 recipWheelRadius,
	const PxF32 restTireLoad, const PxF32 normalisedTireLoad, const PxF32 tireLoad,
	const PxF32 gravity, const PxF32 recipGravity,
	PxF32& wheelTorque, PxF32& tireLongForceMag, PxF32& tireLatForceMag, PxF32& tireAlignMoment)
{
	UVehicleWheelWithCamber* Wheel = (UVehicleWheelWithCamber*)shaderData;

	FTireShaderInputNW Input;

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

	FTireShaderOutputNW Output(0.0f);
	
	Wheel->VehicleNWSim->GenerateTireForces(Wheel, Input, Output);

	wheelTorque = Output.WheelTorque;
	tireLongForceMag = Output.LongForce;
	tireLatForceMag = Output.LatForce;

	if ( /*Wheel->bDebugWheels*/true)
	{
		Wheel->DebugLongSlip = longSlip;
		Wheel->DebugLatSlip = latSlip;
		Wheel->DebugNormalizedTireLoad = normalisedTireLoad;
		Wheel->DebugWheelTorque = wheelTorque;
		Wheel->DebugLongForce = tireLongForceMag;
		Wheel->DebugLatForce = tireLatForceMag;
	}
}

#endif // WITH_PHYSX


UNWheeledVehicleMovementComponent::UNWheeledVehicleMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
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
		check(WheelSetups.Num() <= 32);

		// Calculate wheel offsets first, necessary for sprung masses
		for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx)
		{
			WheelOffsets[WheelIdx] = U2PVector(GetWheelRestingPosition(WheelSetups[WheelIdx]));
		}

		// Now that we have all the wheel offsets, calculate the sprung masses
		PxVec3 PLocalCOM = U2PVector(GetLocalCOM());
		PxVehicleComputeSprungMasses(WheelSetups.Num(), WheelOffsets, PLocalCOM, PVehicleActor->getMass(), 2, SprungMasses);

		for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx)
		{
			UVehicleWheel* Wheel = WheelSetups[WheelIdx].WheelClass.GetDefaultObject();
			UVehicleWheelWithCamber* WheelCamber = Cast<UVehicleWheelWithCamber>(Wheel);

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
			PTireData.mType = Wheel->TireType ? Wheel->TireType->GetTireTypeID() : GEngine->DefaultTireType->GetTireTypeID();
			//PTireData.mCamberStiffnessPerUnitGravity = 0.0f;
			PTireData.mLatStiffX = Wheel->LatStiffMaxLoad;
			PTireData.mLatStiffY = Wheel->LatStiffValue;
			PTireData.mLongitudinalStiffnessPerUnitGravity = Wheel->LongStiffValue;

			// init suspension data
			PxVehicleSuspensionData PSuspensionData;
			PSuspensionData.mSprungMass = SprungMasses[WheelIdx];
			PSuspensionData.mMaxCompression = Wheel->SuspensionMaxRaise;
			PSuspensionData.mMaxDroop = Wheel->SuspensionMaxDrop;
			if (WheelCamber != NULL){
				if (WheelCamber->SuspensionAutoStrength == true) {
					PSuspensionData.mSpringStrength = FMath::Square(Wheel->SuspensionNaturalFrequency) * PSuspensionData.mSprungMass;
					WheelCamber->SuspensionSpringStrength = PSuspensionData.mSpringStrength;
					PSuspensionData.mSpringDamperRate = Wheel->SuspensionDampingRatio * 2.0f * FMath::Sqrt(PSuspensionData.mSpringStrength * PSuspensionData.mSprungMass);
					WheelCamber->SuspensionDamperRatio = PSuspensionData.mSpringDamperRate;
				} else {
					PSuspensionData.mSpringStrength = WheelCamber->SuspensionSpringStrength;
					PSuspensionData.mSpringDamperRate = WheelCamber->SuspensionDamperRatio;
				}
				PSuspensionData.mCamberAtRest = FMath::DegreesToRadians(WheelCamber->SuspensionCamberAtRest);
				PSuspensionData.mCamberAtMaxCompression = FMath::DegreesToRadians(WheelCamber->SuspensionCamberAtMaxCompression);
				PSuspensionData.mCamberAtMaxDroop = FMath::DegreesToRadians(WheelCamber->SuspensionCamberAtMaxDroop);
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
			PxVec3 PSuspForceAppCMOffset = PxVec3(PWheelCentreCMOffset.x, PWheelCentreCMOffset.y, Wheel->SuspensionForceOffset);
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
		const int32 NumChassisShapes = NumShapes - WheelSetups.Num();
		check(NumChassisShapes >= 1);

		TArray<PxShape*> Shapes;
		Shapes.AddZeroed(NumShapes);

		PVehicleActor->getShapes(Shapes.GetData(), NumShapes);

		for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx)
		{
			const int32 WheelShapeIndex = NumChassisShapes + WheelIdx;

			PWheelsSimData->setWheelShapeMapping(WheelIdx, WheelShapeIndex);
			PWheelsSimData->setSceneQueryFilterData(WheelIdx, Shapes[WheelShapeIndex]->getQueryFilterData());
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

////////////////////////////////////////////////////////////////////////////
//Default tire force shader function.
//Taken from Michigan tire model.
//Computes tire long and lat forces plus the aligning moment arising from 
//the lat force and the torque to apply back to the wheel arising from the 
//long force (application of Newton's 3rd law).
////////////////////////////////////////////////////////////////////////////


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

void PxVehicleComputeTireForceDefault1
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


void UNWheeledVehicleMovementComponent::GenerateTireForces(UVehicleWheelWithCamber* Wheel, const FTireShaderInputNW& Input, FTireShaderOutputNW& Output)
{
	const void* realShaderData = &PVehicle->mWheelsSimData.getTireData(Wheel->WheelIndex);

	float Dummy;

	PxVehicleComputeTireForceDefault1(
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

	//UE_LOG( LogVehicles, Warning, TEXT("Friction = %f	LongSlip = %f	LatSlip = %f"), Input.TireFriction, Input.LongSlip, Input.LatSlip );	
	//UE_LOG( LogVehicles, Warning, TEXT("WheelTorque= %f	LongForce = %f	LatForce = %f"), Output.WheelTorque, Output.LongForce, Output.LatForce );
	//UE_LOG( LogVehicles, Warning, TEXT("RestLoad= %f	NormLoad = %f	TireLoad = %f"),Input.RestTireLoad, Input.NormalizedTireLoad, Input.TireLoad );
}