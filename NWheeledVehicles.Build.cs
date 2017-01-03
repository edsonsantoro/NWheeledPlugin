// Copyright 2016 Santoro Studio, All Rights Reserved.


using UnrealBuildTool;

	public class NWheeledVehicles : ModuleRules
	{
		public NWheeledVehicles(TargetInfo Target)
		{
			PublicIncludePaths.AddRange(
				new string[] {
                    "NWheeledVehicles/Classes",
                    "NWheeledVehicles/Private",
  				    "Runtime/AnimGraphRuntime/Private",

                    // ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"NWheeledVehicles/Private",
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
                    "Engine",
                    "Core",
                    "UnrealEd",
            		"CoreUObject", 
                    "AnimGraph",
				    "InputCore",
                    "BlueprintGraph",
				    "RHI",
				    "PhysX", "APEX",
                    "AnimGraphRuntime",
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
    			}

				);
		}
	}
