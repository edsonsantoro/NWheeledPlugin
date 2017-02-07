// Copyright 2016 Santoro Studio, All Rights Reserved.


#pragma once
#include "Runtime/Engine/Classes/Vehicles/WheeledVehicleMovementComponent.h"
#include "Runtime/Engine/Classes/Curves/CurveFloat.h"
#include "WheeledVehicleMovementComponentTank.generated.h"

#if WITH_VEHICLE
namespace physx
{
	class PxVehicleDriveTank;
}
#endif // WITH_VEHICLE


UENUM()
enum class TankDriveControlModel : uint8
{
	STANDARD, //Left/Right thrust range [0,1]
	SPECIAL, //Left/Right thrust range [-1,1]
};

UENUM()
enum class TankDriveControlMethod : uint8
{
		SingleStick,
		DualStick,
};

enum DriveMode {
	eDRIVE_MODE_ACCEL_FORWARDS = 0,
	eDRIVE_MODE_ACCEL_REVERSE,
	eDRIVE_MODE_HARD_TURN_LEFT,
	eDRIVE_MODE_SOFT_TURN_LEFT,
	eDRIVE_MODE_HARD_TURN_RIGHT,
	eDRIVE_MODE_SOFT_TURN_RIGHT,
	eDRIVE_MODE_BRAKE,
	eDRIVE_MODE_NONE
	};
/**
int gDriveMode[] =
	{
	eDRIVE_MODE_BRAKE,
	eDRIVE_MODE_ACCEL_FORWARDS,
	eDRIVE_MODE_BRAKE,
	eDRIVE_MODE_ACCEL_REVERSE,
	eDRIVE_MODE_BRAKE,
	eDRIVE_MODE_HARD_TURN_LEFT,
	eDRIVE_MODE_BRAKE,
	eDRIVE_MODE_HARD_TURN_RIGHT,
	eDRIVE_MODE_BRAKE,
	eDRIVE_MODE_SOFT_TURN_LEFT,
	eDRIVE_MODE_BRAKE,
	eDRIVE_MODE_SOFT_TURN_RIGHT,
	eDRIVE_MODE_NONE
	};
	*/
	
USTRUCT()
struct FTankControlData
{
    GENERATED_USTRUCT_BODY()

	/** Tank Control Method */
	UPROPERTY(EditAnywhere, Category = Setup)
	TankDriveControlModel TankControlModel;

	/** Tank Control Model */
	UPROPERTY(EditAnywhere, Category = Setup)
	TankDriveControlMethod TankControlMethod;

	UPROPERTY(EditAnywhere, Category = "TankInput", AdvancedDisplay)
	FVehicleInputRate LeftThrustRate;

	UPROPERTY(EditAnywhere, Category = "TankInput", AdvancedDisplay)
	FVehicleInputRate RightThrustRate;

	UPROPERTY(EditAnywhere, Category = "TankInput", AdvancedDisplay)
	FVehicleInputRate RightBrakeRate;

	UPROPERTY(EditAnywhere, Category = "TankInput", AdvancedDisplay)
	FVehicleInputRate LeftBrakeRate;

};

USTRUCT()
struct FTankEngineData
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
class NWHEELEDVEHICLES_API UWheeledVehicleMovementComponentTank : public UWheeledVehicleMovementComponent
{
	GENERATED_UCLASS_BODY()

	/** Engine Data */
	UPROPERTY(EditAnywhere, Category = "MechanicalSetup")
	FTankEngineData EngineSetup;

	/** Transmission data */
	UPROPERTY(EditAnywhere, Category = "MechanicalSetup")
	FVehicleTransmissionData TransmissionSetup;

	/** Tank Control Model */
	UPROPERTY(EditAnywhere, Category = "ControlSetup")
	FTankControlData TankControlSetup;

	/** Maximum steering versus forward speed (km/h) */
	UPROPERTY(EditAnywhere, Category = SteeringSetup)
	FRuntimeFloatCurve SteeringCurve;

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

	float Acceleration;

	float RawLeftBrake;

	float RawRightBrake;

	float RawLeftThrust;

	float RawRightThrust;

#endif // WITH_VEHICLE

public:

	/** Set the user input for the vehicle throttle */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|WheeledVehicleMovement")
	void SetAccelInput(float Accel);

	/** Set the user input for the vehicle throttle */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|WheeledVehicleMovement")
	void SetLeftThrottleInput(float LeftThrottle);

	/** Set the user input for the vehicle throttle */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|WheeledVehicleMovement")
	void SetRightThrottleInput(float RightThrottle);

	/** Set the user input for the vehicle throttle */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|WheeledVehicleMovement")
	void SetLeftBrakeInput(float LeftBrake);

	/** Set the user input for the vehicle throttle */
	UFUNCTION(BlueprintCallable, Category = "Game|Components|WheeledVehicleMovement")
	void SetRightBrakeInput(float RightBrake);		  

protected:
	/** update simulation data: engine */
	void UpdateEngineSetup(const FTankEngineData& NewEngineSetup);

	/** update simulation data: transmission */
	void UpdateTransmissionSetup(const FVehicleTransmissionData& NewGearSetup);

	/** Update simulation data: control method */
	void UpdateControlSetup(const FTankControlData& NewControlSetup);

};