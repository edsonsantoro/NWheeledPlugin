// Copyright 2016 Santoro Studio, All Rights Reserved.

/*
* Component to handle the vehicle simulation for an actor
*/
#pragma once
#include "Runtime/Engine/Classes/Vehicles/VehicleWheel.h"
#include "VehicleWheelWithCamber.generated.h"

#if WITH_PHYSX
namespace physx
{
	class PxShape;
}
#endif // WITH_PHYSX


UCLASS(BlueprintType, Blueprintable)
class NWHEELEDVEHICLES_API UVehicleWheelWithCamber : public UVehicleWheel
{
	GENERATED_UCLASS_BODY()

	/** Camber angle (in radians) of wheel when the suspension is at its rest position.  */
	UPROPERTY(EditAnywhere, Category = Suspension)
	float										SuspensionCamberAtRest;

	/** Camber angle (in radians) of wheel when the suspension is at maximum compression.   */
	UPROPERTY(EditAnywhere, Category = Suspension)
	float										SuspensionCamberAtMaxCompression;

	/** Camber angle (in radians) of wheel when the suspension is at maximum droop.    */
	UPROPERTY(EditAnywhere, Category = Suspension)
	float										SuspensionCamberAtMaxDroop;

	/** If Engine should calc suspension strength and damping automatically by [square(SuspensionNaturalFrequency * SprungMass)]   */
	UPROPERTY(EditAnywhere, Category = Suspension)
	bool										SuspensionAutoStrength;

	/** The strengh of this spring */
	UPROPERTY(EditAnywhere, Category = Suspension)
	float										SuspensionSpringStrength;

	/** The damping ratio of this spring */
	UPROPERTY(EditAnywhere, Category = Suspension)
	float										SuspensionDamperRatio;

	UFUNCTION(BlueprintCallable, Category = "Game|Components|WheeledVehicleMovement")
	FQuat GetWheelRotationAngles();

	/** The vehicle that may owns us */
	UPROPERTY(transient)
	class UNWheeledVehicleMovementComponent*			VehicleNWSim;

	/** The vehicle that may owns us */
	UPROPERTY(transient)
	class UNDWheeledVehicleMovementComponent*			VehicleNDSim;

#if WITH_EDITOR

	/**
	* Respond to a property change in editor
	*/
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

#endif //WITH_EDITOR

};