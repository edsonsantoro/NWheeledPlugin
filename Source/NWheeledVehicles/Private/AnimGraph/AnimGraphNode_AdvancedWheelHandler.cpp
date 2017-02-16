// Copyright 2016 Santoro Studio, All Rights Reserved.

#include "NWheeledVehiclesPluginPrivatePCH.h"
#include "AnimGraph/AnimGraphNode_AdvancedWheelHandler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_AdvancedWheelHandler

#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_AdvancedWheelHandler::UAnimGraphNode_AdvancedWheelHandler(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FText UAnimGraphNode_AdvancedWheelHandler::GetControllerDescription() const
{
	return LOCTEXT("AnimGraphNode_AdvancedWheelHandler", "Advanced Wheel Handler for WheeledVehicle");
}

FText UAnimGraphNode_AdvancedWheelHandler::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_AdvancedWheelHandler_Tooltip", "This alters the wheel transform based on set up in NWheeled Vehicle. This only works when the owner is NWheeledVehicle.");
}

FText UAnimGraphNode_AdvancedWheelHandler::GetNodeTitle(ENodeTitleType::Type TitleType) const
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
		NodeTitle = FText(LOCTEXT("AnimGraphNode_AdvancedWheelHandler_Title", "Advanced Wheel Handler"));
	}
	return NodeTitle;
}

void UAnimGraphNode_AdvancedWheelHandler::ValidateAnimNodePostCompile(class FCompilerResultsLog& MessageLog, class UAnimBlueprintGeneratedClass* CompiledClass, int32 CompiledNodeIndex)
{
	// we only support vehicle anim instance
	if (CompiledClass->IsChildOf(UNVehicleAnimInstance::StaticClass()) == false)
	{
		MessageLog.Error(TEXT("@@ is only allowwed in NVehicleAnimInstance. If this is for vehicle, please change parent to be NVehicleAnimInstancen (Reparent Class)."), this);
	}
}

bool UAnimGraphNode_AdvancedWheelHandler::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(TargetGraph);
	return (Blueprint != nullptr) && Blueprint->ParentClass->IsChildOf<UNVehicleAnimInstance>() && Super::IsCompatibleWithGraph(TargetGraph);
}

#undef LOCTEXT_NAMESPACE