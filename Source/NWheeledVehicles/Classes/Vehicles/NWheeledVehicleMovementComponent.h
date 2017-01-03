// Copyright 2016 Santoro Studio, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "WheeledVehicleMovementComponent.h"
#include "GameFramework/MovementComponent.h"
#include "VehicleWheel.h"
#include "Vehicles/AdvancedVehicleWheel.h"
#include "NWheeledVehicleMovementComponent.generated.h"

class UCanvas;

namespace physx
{
	class PxVehicleDrive;
	class PxVehicleWheels;
	class PxVehicleWheelsSimData;
}


/**
* Values passed from PhysX to generate tire forces
*/
struct FTireShaderInputNW
{
	// Friction value of the tire contact.
	float TireFriction;

	// Longitudinal slip of the tire
	float LongSlip;

	// Lateral slip of the tire.
	float LatSlip;

	// Suspension Camber
	float Camber;

	// Rotational speed of the wheel, in radians.
	float WheelOmega;

	// The distance from the tire surface to the center of the wheel.
	float WheelRadius;

	// 1 / WheelRadius
	float RecipWheelRadius;

	// How much force (weight) is pushing on the tire when the vehicle is at rest.
	float RestTireLoad;

	// How much force (weight) is pushing on the tire right now.
	float TireLoad;

	// RestTireLoad / TireLoad
	float NormalizedTireLoad;

	// Acceleration due to gravity
	float Gravity;

	// 1 / Gravity
	float RecipGravity;
};

/**
* Generated tire forces to pass back to PhysX
*/
struct FTireShaderOutputNW
{
	// The torque to be applied to the wheel around the wheel axle. Opposes the engine torque on the wheel
	float WheelTorque;

	// The magnitude of the longitudinal tire force to be applied to the vehicle's rigid body.
	float LongForce;

	// The magnitude of the lateral tire force to be applied to the vehicle's rigid body.
	float LatForce;

	FTireShaderOutputNW() {}

	FTireShaderOutputNW(float f)
		: WheelTorque(f)
		, LongForce(f)
		, LatForce(f)
	{
	}
};

/**
* Component to handle the vehicle simulation for an actor.
*/
UCLASS(Abstract, hidecategories = (PlanarMovement, "Components|Movement|Planar", Activation, "Components|Activation"))
class NWHEELEDVEHICLES_API UNWheeledVehicleMovementComponent : public UWheeledVehicleMovementComponent
{
	GENERATED_UCLASS_BODY()

		/** Compute the forces generates from a spinning tire */
		void GenerateFullTireForces(class UVehicleWheel* Wheel, const FTireShaderInputNW& Input, FTireShaderOutputNW& Output);

		/** Used to create any physics engine information for this component */
		virtual void OnCreatePhysicsState() override;

protected:


		/** Set up the wheel data */
		void SetupWheels(physx::PxVehicleWheelsSimData* PWheelsSimData) override;

		/** Instantiate and setup our wheel objects */
		virtual void CreateWheels() override;


};