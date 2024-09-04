// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class OriginalGameTarget : TargetRules
{
	public OriginalGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		CppStandard = CppStandardVersion.Default;
		ExtraModuleNames.Add("OriginalGame");
	}
}
