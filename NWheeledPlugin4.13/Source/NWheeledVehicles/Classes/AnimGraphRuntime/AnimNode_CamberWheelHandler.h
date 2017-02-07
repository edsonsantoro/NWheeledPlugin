// Copyright 2016 Santoro Studio, All Rights Reserved.

#pragma once
#include "Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_WheelHandler.h"
#include "Vehicles/NWheeledVehicleMovementComponent.h"
#include "Vehicles/NDWheeledVehicleMovementComponent.h"
#include "AnimNode_CamberWheelHandler.generated.h"

/**
*	Simple controller that replaces or adds to the translation/rotation of a single bone.
*/
USTRUCT()
struct NWHEELEDVEHICLES_API FAnimNode_CamberWheelHandler : public FAnimNode_WheelHandler
{
	GENERATED_USTRUCT_BODY()

	FAnimNode_CamberWheelHandler();

	/** Current Asset being played **/
	UPROPERTY(transient)
	class UNWheeledVehicleMovementComponent* VehicleNWSimComponent;

	/** Current Asset being played **/
	UPROPERTY(transient)
	class UNDWheeledVehicleMovementComponent* VehicleNDSimComponent;



	virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;
	virtual void Initialize(const FAnimationInitializeContext& Context) override;


};