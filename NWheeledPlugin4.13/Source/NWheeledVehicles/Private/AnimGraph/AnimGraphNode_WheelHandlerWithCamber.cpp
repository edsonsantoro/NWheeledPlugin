// Copyright 2016 Santoro Studio, All Rights Reserved.

#include "NWheeledVehiclesPluginPrivatePCH.h"
#include "AnimGraph/AnimGraphNode_WheelHandlerWithCamber.h"
#include "Runtime/Engine/Classes/Vehicles/VehicleAnimInstance.h"
#include "Editor/AnimGraph/Private/AnimGraphPrivatePCH.h"
#include "Editor/UnrealEd/Public/Kismet2/CompilerResultsLog.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_WheelHandlerWithCamber

#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_WheelHandlerWithCamber::UAnimGraphNode_WheelHandlerWithCamber(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FText UAnimGraphNode_WheelHandlerWithCamber::GetControllerDescription() const
{
	return LOCTEXT("AnimGraphNode_WheelHandlerWithCamber", "Wheel Handler for WheeledVehicle with camber support");
}

FText UAnimGraphNode_WheelHandlerWithCamber::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_WheelHandlerWithCamber_Tooltip", "This alters the wheel transform based on set up in Wheeled Vehicle. This only works when the owner is WheeledVehicle and NWheeledVehicle.");
}

FText UAnimGraphNode_WheelHandlerWithCamber::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FText NodeTitle;
	if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
	{
		NodeTitle = GetControllerDescription();
	}
	else
	{
		// we don't have any run-time information, so it's limited to print  
		// anymore than what it is it would be nice to print more data such as 
		// name of bones for wheels, but it's not available in Persona
		NodeTitle = FText(LOCTEXT("AnimGraphNode_WheelHandlerWithCamber_Title", "Wheel Handler with Camber"));
	}
	return NodeTitle;
}

void UAnimGraphNode_WheelHandlerWithCamber::ValidateAnimNodePostCompile(class FCompilerResultsLog& MessageLog, class UAnimBlueprintGeneratedClass* CompiledClass, int32 CompiledNodeIndex)
{
	// we only support vehicle anim instance
	if (CompiledClass->IsChildOf(UVehicleAnimInstance::StaticClass()) == false)
	{
		MessageLog.Error(TEXT("@@ is only allowwed in VehicleAnimInstance. If this is for vehicle, please change parent to be VehicleAnimInstancen (Reparent Class)."), this);
	}
}

bool UAnimGraphNode_WheelHandlerWithCamber::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(TargetGraph);
	return (Blueprint != nullptr) && Blueprint->ParentClass->IsChildOf<UVehicleAnimInstance>() && Super::IsCompatibleWithGraph(TargetGraph);
}

#undef LOCTEXT_NAMESPACE
