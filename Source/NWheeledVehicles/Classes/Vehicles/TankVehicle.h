// Copyright 2016 Santoro Studio, All Rights Reserved.

#pragma once
#include "Runtime/Engine/Classes/GameFramework/Pawn.h"
#include "TankVehicle.generated.h"

class FDebugDisplayInfo;

/**
* TankVehicle is the my custom tank vehicle pawn actor.
*/

UCLASS(abstract, config = Game, BlueprintType)
class NWHEELEDVEHICLES_API ATankVehicle : public APawn
{
	GENERATED_UCLASS_BODY()

	private_subobject:
	/**  The main skeletal mesh associated with this Vehicle */
	DEPRECATED_FORGAME(4.6, "Mesh should not be accessed directly, please use GetMesh() function instead. Mesh will soon be private and your code will not compile.")
	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh;

	/** vehicle simulation component */
	DEPRECATED_FORGAME(4.6, "VehicleMovement should not be accessed directly, please use GetVehicleMovement() function instead. VehicleMovement will soon be private and your code will not compile.")
	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWheeledVehicleMovementComponent* VehicleMovement;

public:

	/** Name of the MeshComponent. Use this name if you want to prevent creation of the component (with ObjectInitializer.DoNotCreateDefaultSubobject). */
	static FName VehicleMeshComponentName;

	/** Name of the VehicleMovement. Use this name if you want to use a different class (with ObjectInitializer.SetDefaultSubobjectClass). */
	static FName VehicleMovementComponentName;

	/** Util to get the wheeled vehicle movement component */
	class UWheeledVehicleMovementComponent* GetVehicleMovementComponent() const;

	//~ Begin AActor Interface
	virtual void DisplayDebug(class UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	//~ End Actor Interface

	/** Returns Mesh subobject **/
	class USkeletalMeshComponent* GetMesh() const;
	/** Returns VehicleMovement subobject **/
	class UWheeledVehicleMovementComponent* GetVehicleMovement() const;
};
