// Copyright 2016 Santoro Studio, All Rights Reserved.

namespace UnrealBuildTool.Rules
{
    public class NWheeledVehicles : ModuleRules
    {
        public NWheeledVehicles(ReadOnlyTargetRules Target) : base(Target)
        {
            PublicIncludePaths.Add("NWheeledVehicles/Classes");
            PrivateIncludePaths.Add("NWheeledVehicles/Private");
            
            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "UnrealEd",
                    "BlueprintGraph",
                    "AnimGraph",
                    "AnimGraphRuntime",
                    "PhysX",
                    "APEX",
                    "PhysXVehicles",
                    "PhysXVehicleLib",
                }
            );
            
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "PhysXVehicles",
                    "PhysXVehicleLib",
                }
            );
        }
    }
}
