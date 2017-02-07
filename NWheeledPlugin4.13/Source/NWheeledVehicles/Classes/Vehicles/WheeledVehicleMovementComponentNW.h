// Copyright 2016 Santoro Studio, All Rights Reserved.

/*
* Base VehicleSim for the NW PhysX vehicle class
*/
#pragma once
#include "Vehicles/NWheeledVehicleMovementComponent.h"
#include "Runtime/Engine/Classes/Curves/CurveFloat.h"
#include "WheeledVehicleMovementComponentNW.generated.h"


#if WITH_VEHICLE
namespace physx
{
	class PxVehicleDriveNW;
}
#endif // WITH_VEHICLE

USTRUCT()
struct FDrivenWheelData
{
	GENERATED_USTRUCT_BODY()

	/** The wheel reference */
	UPROPERTY(Category = Setup, VisibleAnywhere)
	FName WheelReference;
	
	/** If we should connect this Wheel ID the Differential */
	UPROPERTY(EditAnywhere, Category = Setup)
	bool IsDrivenWheel;
};

USTRUCT()
struct FVehicleDifferentialNWData
{
	GENERATED_USTRUCT_BODY()

	/** Array of Wheel IDs and driven wheel state */
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay)
	TArray<FDrivenWheelData> DrivenWheels;
};

USTRUCT()
struct FVehicleEngineDataNW
{
	GENERATED_USTRUCT_BODY()

	/** Torque (Nm) at a given RPM*/
	UPROPERTY(EditAnywhere, Category = Setup)
	FRuntimeFloatCurve TorqueCurve;

	/** Maximum revolutions per minute of the engine */
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float MaxRPM;

	/** Moment of inertia of the engine around the axis of rotation (Kgm^2). */
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float MOI;

	/** Damping rate of engine when full throttle is applied (Kgm^2/s) */
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DampingRateFullThrottle;

	/** Damping rate of engine in at zero throttle when the clutch is engaged (Kgm^2/s)*/
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DampingRateZeroThrottleClutchEngaged;

	/** Damping rate of engine in at zero throttle when the clutch is disengaged (in neutral gear) (Kgm^2/s)*/
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DampingRateZeroThrottleClutchDisengaged;

	/** Find the peak torque produced by the TorqueCurve */
	float FindPeakTorque() const;
};

UCLASS()
class NWHEELEDVEHICLES_API UWheeledVehicleMovementComponentNW : public UNWheeledVehicleMovementComponent
{
	GENERATED_UCLASS_BODY()

	/** Engine */
	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
	FVehicleEngineDataNW EngineSetup;

	/** Differential */
	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
	FVehicleDifferentialNWData DifferentialSetup;

	/** Transmission data */
	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
	FVehicleTransmissionData TransmissionSetup;

	/** Maximum steering versus forward speed (km/h) */
	UPROPERTY(EditAnywhere, Category = SteeringSetup)
	FRuntimeFloatCurve SteeringCurve;

	virtual void Serialize(FArchive & Ar) override;
	virtual void ComputeConstants() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


	/** Get the Drag Magnitude */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|WheeledVehicleMovement")
	float GetVehicleDragMagnitude();

	/** Get the Drag Magnitude */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|WheeledVehicleMovement")
	float GetVehicleDragArea();

	/** Get number of gears */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|WheeledVehicleMovement")
	FVehicleTransmissionData GetGearboxDetails();

protected:

#if WITH_VEHICLE

	/** Allocate and setup the PhysX vehicle */
	virtual void SetupVehicle() override;

	virtual void UpdateSimulation(float DeltaTime) override;

#endif // WITH_VEHICLE

	/** update simulation data: engine */
	void UpdateEngineSetup(const FVehicleEngineDataNW& NewEngineSetup);

	/** update simulation data: differential */
	void UpdateDifferentialSetup(const FVehicleDifferentialNWData& NewDifferentialSetup);

	/** update simulation data: transmission */
	void UpdateTransmissionSetup(const FVehicleTransmissionData& NewGearSetup);
};

