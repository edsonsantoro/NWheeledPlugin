// Copyright 2016 Santoro Studio, All Rights Reserved.

#pragma once

#include "Editor/AnimGraph/Classes/AnimGraphNode_SkeletalControlBase.h"
#include "Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_WheelHandler.h"
#include "AnimGraphRuntime/AnimNode_CamberWheelHandler.h"
#include "AnimGraphNode_WheelHandlerWithCamber.generated.h"

UCLASS( meta = (Keywords = "Modify Wheel Camber Vehicle"))
class NWHEELEDVEHICLES_API UAnimGraphNode_WheelHandlerWithCamber : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_UCLASS_BODY()

		UPROPERTY(EditAnywhere, Category = Settings)
		FAnimNode_CamberWheelHandler Node;

public:
	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	// validate if this is within VehicleAnimInstance
	virtual void ValidateAnimNodePostCompile(class FCompilerResultsLog& MessageLog, class UAnimBlueprintGeneratedClass* CompiledClass, int32 CompiledNodeIndex) override;
	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;
	// End of UEdGraphNode interface

protected:
	// UAnimGraphNode_SkeletalControlBase interface
	virtual FText GetControllerDescription() const override;
	virtual const FAnimNode_SkeletalControlBase* GetNode() const override { return &Node; }
	// End of UAnimGraphNode_SkeletalControlBase interface
};
