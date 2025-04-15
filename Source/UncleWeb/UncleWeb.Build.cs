// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UncleWeb : ModuleRules
{
	public UncleWeb(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "CableComponent" });
	}
}
