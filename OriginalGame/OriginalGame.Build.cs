// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OriginalGame : ModuleRules
{
	public OriginalGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				"OriginalGame"
			});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"AIModule",
			"GameplayAbilities",
			"CommonGame",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AudioMixer",
			"ApplicationCore",
			"DeveloperSettings",
			"CommonLoadingScreen",
			"CommonInput",
			"CommonUI",
			"GameFeatures",
			"GameplayMessageRuntime",
			"GameplayTasks",
			"ModularGameplay",
			"ModularGameplayActors",
			"Slate",
			"EnhancedInput",
			"RHI",
			"UMG"
		});

		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd", "AnimGraph"
			});
		}
	}
}