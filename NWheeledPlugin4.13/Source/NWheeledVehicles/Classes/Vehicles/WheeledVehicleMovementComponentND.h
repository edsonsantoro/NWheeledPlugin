// Copyright 2016 Santoro Studio, All Rights Reserved.

/*
* Base VehicleSim for the NoDrive PhysX vehicle class
*/

#pragma once
#include "Vehicles/NDWheeledVehicleMovementComponent.h"
#include "Runtime/Engine/Classes/Curves/CurveFloat.h"
#include "WheeledVehicleMovementComponentND.generated.h"

#if WITH_VEHICLE
namespace physx
{
	class PxVehicleNoDrive;
}
#endif // WITH_VEHICLE

USTRUCT()
struct FDrivenWheelsData
{
	GENERATED_USTRUCT_BODY()

		/** The wheel reference */
		UPROPERTY(Category = Setup, VisibleAnywhere)
		FName WheelReference;

	/** If we should connect this Wheel ID the Differential */
	UPROPERTY(EditAnywhere, Category = Setup)
		bool IsWheelEnabled;

	/** Amount of torque to deliver to this wheel id */
	UPROPERTY(EditAnywhere, Category = Setup)
		float DriveTorque;
};


USTRUCT()
struct FVehicleWheelsData
{
	GENERATED_USTRUCT_BODY()

		/** Array of Wheel IDs and driven wheel state */
		UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay)
		TArray<FDrivenWheelsData> DrivenWheels;
};


UCLASS()
class NWHEELEDVEHICLES_API UWheeledVehicleMovementComponentND : public UNDWheeledVehicleMovementComponent
{
	GENERATED_UCLASS_BODY()

	/** Maximum steering versus forward speed (km/h) */
	UPROPERTY(EditAnywhere, Category = SteeringSetup)
	FRuntimeFloatCurve SteeringCurve;

	/** Wheel States */
	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
	FVehicleWheelsData WheelsData;

	virtual void Serialize(FArchive & Ar) override;
	virtual void ComputeConstants() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

#if WITH_VEHICLE

	/** Allocate and setup the PhysX vehicle */
	virtual void SetupVehicle() override;

	virtual void UpdateSimulation(float DeltaTime) override;

#endif // WITH_VEHICLE

	/** update simulation data: differential */
	void UpdateWheelsDataSetup(const FVehicleWheelsData& NewWheelsDataSetup);


};

