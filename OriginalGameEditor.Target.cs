// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class OriginalGameEditorTarget : TargetRules
{
	public OriginalGameEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		CppStandard = CppStandardVersion.Default;
		ExtraModuleNames.AddRange( new string[] { "OriginalGame" } );
	}
}
