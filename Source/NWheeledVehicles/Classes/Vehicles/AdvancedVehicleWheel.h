// Copyright 2016 Santoro Studio, All Rights Reserved.

/*
* Component to handle the vehicle simulation for an actor
*/
#pragma once
#include "VehicleWheel.h"
#include "AdvancedVehicleWheel.generated.h"

class UPhysicalMaterial;

namespace physx
{
	class PxShape;
}


UCLASS(BlueprintType, Blueprintable)
class NWHEELEDVEHICLES_API UAdvancedVehicleWheel : public UVehicleWheel
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

	/** The Camber Stiffiness per unit Gravity */
	UPROPERTY(EditAnywhere, Category = Suspension)
	float                                       CamberStiffnessPerUnitGravity;

	/** If Engine should calc suspension strength automatically by [square(SuspensionNaturalFrequency * SprungMass)]   */
	UPROPERTY(EditAnywhere, Category = Suspension)
	bool										SuspensionAutoStrength;

	/** The strengh of this spring */
	UPROPERTY(EditAnywhere, Category = Suspension)
	float										SuspensionSpringStrength;

	/** The danper rate of this spring */
	UPROPERTY(EditAnywhere, Category = Suspension)
	float										DamperRate;



	UFUNCTION(BlueprintCallable, Category = "Game|Components|WheeledVehicleMovement")
	FQuat GetWheelRotationAngles();

	/** The vehicle that may owns us */
	UPROPERTY(transient)
	class UNWheeledVehicleMovementComponent*			VehicleNWSim;

#if WITH_EDITOR

	/**
	* Respond to a property change in editor
	*/
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

#endif //WITH_EDITOR

};