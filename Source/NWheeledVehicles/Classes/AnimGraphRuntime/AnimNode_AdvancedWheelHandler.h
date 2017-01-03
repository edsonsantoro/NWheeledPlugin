// Copyright 2016 Santoro Studio, All Rights Reserved.

#pragma once

#include "AnimNode_WheelHandler.h"
#include "Vehicles/NVehicleAnimInstance.h"
#include "AnimNode_AdvancedWheelHandler.generated.h"

/**
*	Simple controller that replaces or adds to the translation/rotation of a single bone.
*/
USTRUCT()
struct NWHEELEDVEHICLES_API FAnimNode_AdvancedWheelHandler : public FAnimNode_WheelHandler
{
	GENERATED_USTRUCT_BODY()

	FAnimNode_AdvancedWheelHandler();

	// FAnimNode_Base interface
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface


	// FAnimNode_SkeletalControlBase interface
	virtual void EvaluateBoneTransforms(USkeletalMeshComponent* SkelComp, FCSPose<FCompactPose>& MeshBases, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	virtual void Initialize(const FAnimationInitializeContext& Context) override;
	// End of FAnimNode_SkeletalControlBase interface
private:
	// FAnimNode_SkeletalControlBase interface
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface

	struct FNWheelLookupData
	{
		int32 WheelIndex;
		FBoneReference BoneReference;
	};

	TArray<FNWheelLookupData> NWheels;


	const FNVehicleAnimInstanceProxy* NAnimInstanceProxy;	//TODO: we only cache this to use in eval where it's safe. Should change API to pass proxy into eval
};
