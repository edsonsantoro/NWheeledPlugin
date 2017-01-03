// Copyright 2016 Santoro Studio, All Rights Reserved.

namespace UnrealBuildTool.Rules
{
    public class NWheeledVehicles : ModuleRules
    {
        public NWheeledVehicles(TargetInfo Target)
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
                    "PhysXVehicles",
                    "PhysXVehicleLib",
                }
            );

            SetupModulePhysXAPEXSupport(Target);
        }
    }
}
