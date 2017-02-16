// Copyright 2016 Santoro Studio, All Rights Reserved.

#include "NWheeledVehiclesPluginPrivatePCH.h"
#include "AnimGraphRuntime/AnimNode_AdvancedWheelHandler.h"
#include "AnimationRuntime.h"
#include "Vehicles/NWheeledVehicle.h"
#include "WheeledVehicleMovementComponent.h"


FAnimNode_AdvancedWheelHandler::FAnimNode_AdvancedWheelHandler()
{
	NAnimInstanceProxy = nullptr;
}

void FAnimNode_AdvancedWheelHandler::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);

	DebugLine += "(";
	AddDebugNodeData(DebugLine);
	DebugLine += ")";

	DebugData.AddDebugItem(DebugLine);

	const TArray<FWheelAnimData>& WheelAnimData = NAnimInstanceProxy->GetWheelAnimData();
	for (const FNWheelLookupData& Wheel : NWheels)
	{
		if (Wheel.BoneReference.BoneIndex != INDEX_NONE)
		{
			DebugLine = FString::Printf(TEXT(" [Wheel Index : %d] Bone: %s , Rotation Offset : %s, Location Offset : %s"),
				Wheel.WheelIndex, *Wheel.BoneReference.BoneName.ToString(), *WheelAnimData[Wheel.WheelIndex].RotOffset.ToString(), *WheelAnimData[Wheel.WheelIndex].LocOffset.ToString());
		}
		else
		{
			DebugLine = FString::Printf(TEXT(" [Wheel Index : %d] Bone: %s (invalid bone)"),
				Wheel.WheelIndex, *Wheel.BoneReference.BoneName.ToString());
		}

		DebugData.AddDebugItem(DebugLine);
	}

	ComponentPose.GatherDebugData(DebugData);
}

void FAnimNode_AdvancedWheelHandler::EvaluateBoneTransforms(USkeletalMeshComponent* SkelComp, FCSPose<FCompactPose>& MeshBases, TArray<FBoneTransform>& OutBoneTransforms)
{
	check(OutBoneTransforms.Num() == 0);

	const TArray<FWheelAnimData>& WheelAnimData = NAnimInstanceProxy->GetWheelAnimData();

	const FBoneContainer& BoneContainer = MeshBases.GetPose().GetBoneContainer();
	for (const FNWheelLookupData& Wheel : NWheels)
	{
		if (Wheel.BoneReference.IsValid(BoneContainer))
		{
			FCompactPoseBoneIndex WheelSimBoneIndex = Wheel.BoneReference.GetCompactPoseIndex(BoneContainer);

			// the way we apply transform is same as FMatrix or FTransform
			// we apply scale first, and rotation, and translation
			// if you'd like to translate first, you'll need two nodes that first node does translate and second nodes to rotate. 
			FTransform NewBoneTM = MeshBases.GetComponentSpaceTransform(WheelSimBoneIndex);

			FAnimationRuntime::ConvertCSTransformToBoneSpace(SkelComp, MeshBases, NewBoneTM, WheelSimBoneIndex, BCS_ComponentSpace);

			// Apply rotation offset
			const FQuat BoneQuat(WheelAnimData[Wheel.WheelIndex].RotOffset);
			NewBoneTM.SetRotation(BoneQuat * NewBoneTM.GetRotation());

			// Apply loc offset
			NewBoneTM.AddToTranslation(WheelAnimData[Wheel.WheelIndex].LocOffset);

			// Convert back to Component Space.
			FAnimationRuntime::ConvertBoneSpaceTransformToCS(SkelComp, MeshBases, NewBoneTM, WheelSimBoneIndex, BCS_ComponentSpace);

			// add back to it
			OutBoneTransforms.Add(FBoneTransform(WheelSimBoneIndex, NewBoneTM));
		}
	}
}

bool FAnimNode_AdvancedWheelHandler::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	// if both bones are valid
	for (const FNWheelLookupData& Wheel : NWheels)
	{
		// if one of them is valid
		if (Wheel.BoneReference.IsValid(RequiredBones) == true)
		{
			return true;
		}
	}

	return false;
}


void FAnimNode_AdvancedWheelHandler::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	const TArray<FWheelAnimData>& WheelAnimData = NAnimInstanceProxy->GetWheelAnimData();
	const int32 NumWheels = WheelAnimData.Num();
	NWheels.Empty(NumWheels);

	for (int32 WheelIndex = 0; WheelIndex < NumWheels; ++WheelIndex)
	{
		FNWheelLookupData* Wheel = new(NWheels)FNWheelLookupData();
		Wheel->WheelIndex = WheelIndex;
		Wheel->BoneReference.BoneName = WheelAnimData[WheelIndex].BoneName;
		Wheel->BoneReference.Initialize(RequiredBones);
	}

	// sort by bone indices
	NWheels.Sort([](const FNWheelLookupData& L, const FNWheelLookupData& R) { return L.BoneReference.BoneIndex < R.BoneReference.BoneIndex; });
}


void FAnimNode_AdvancedWheelHandler::Initialize(const FAnimationInitializeContext& Context)
{
	NAnimInstanceProxy = (FNVehicleAnimInstanceProxy*)Context.AnimInstanceProxy;	//TODO: This is cached for now because we need it in eval bone transforms.
}
