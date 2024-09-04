// Copyright Epic Games, Inc. All Rights Reserved.

#include "JoyPerformanceSettings.h"

#include "Engine/PlatformSettingsManager.h"
#include "JoyPerformanceStatTypes.h"
#include "Misc/EnumRange.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(JoyPerformanceSettings)

//////////////////////////////////////////////////////////////////////

UJoyPlatformSpecificRenderingSettings::UJoyPlatformSpecificRenderingSettings()
{
	MobileFrameRateLimits.Append({20, 30, 45, 60, 90, 120});
}

const UJoyPlatformSpecificRenderingSettings* UJoyPlatformSpecificRenderingSettings::Get()
{
	UJoyPlatformSpecificRenderingSettings* Result = UPlatformSettingsManager::Get().GetSettingsForPlatform<ThisClass>();
	check(Result);
	return Result;
}

//////////////////////////////////////////////////////////////////////

UJoyPerformanceSettings::UJoyPerformanceSettings()
{
	PerPlatformSettings.Initialize(UJoyPlatformSpecificRenderingSettings::StaticClass());

	CategoryName = TEXT("Game");

	DesktopFrameRateLimits.Append({30, 60, 120, 144, 160, 165, 180, 200, 240, 360});

	// Default to all stats are allowed
	FJoyPerformanceStatGroup& StatGroup = UserFacingPerformanceStats.AddDefaulted_GetRef();
	for (EJoyDisplayablePerformanceStat PerfStat : TEnumRange<EJoyDisplayablePerformanceStat>())
	{
		StatGroup.AllowedStats.Add(PerfStat);
	}
}
