// Copyright 2016 Santoro Studio, All Rights Reserved.

#pragma once
/**
*
*/

#include "CoreMinimal.h"
#include "VehicleAnimInstance.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "NVehicleAnimInstance.generated.h"

class UNWheeledVehicleMovementComponent;

/** Proxy override for this UAnimInstance-derived class */
USTRUCT()
struct NWHEELEDVEHICLES_API FNVehicleAnimInstanceProxy : public FVehicleAnimInstanceProxy
{
	GENERATED_BODY()

		FNVehicleAnimInstanceProxy()
		: FVehicleAnimInstanceProxy()
	{
	}

	FNVehicleAnimInstanceProxy(UAnimInstance* Instance)
		: FVehicleAnimInstanceProxy(Instance)
	{
	}

public:

	void SetNWheeledVehicleMovementComponent(const UWheeledVehicleMovementComponent* InWheeledVehicleMovementComponent);


	/** FAnimInstanceProxy interface begin*/
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	/** FAnimInstanceProxy interface end*/

private:
	TArray<FWheelAnimData> NWheelInstances;

};

UCLASS(transient)
class NWHEELEDVEHICLES_API UNVehicleAnimInstance : public UVehicleAnimInstance
{
	GENERATED_UCLASS_BODY()

	/** Makes a montage jump to the end of a named section. */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	class ANWheeledVehicle * GetVehicle();

public:

	void SetNWheeledVehicleMovementComponent(const UWheeledVehicleMovementComponent* InWheeledVehicleMovementComponent)
	{
		NWheeledVehicleMovementComponent = InWheeledVehicleMovementComponent;
		NAnimInstanceProxy.SetNWheeledVehicleMovementComponent(InWheeledVehicleMovementComponent);
	}


private:
	/** UAnimInstance interface begin*/
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override;
	/** UAnimInstance interface end*/

	FNVehicleAnimInstanceProxy NAnimInstanceProxy;

	UPROPERTY(transient)
	const UWheeledVehicleMovementComponent* NWheeledVehicleMovementComponent;


};



