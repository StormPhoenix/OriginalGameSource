// Copyright Epic Games, Inc. All Rights Reserved.

#include "QinPerformanceSettings.h"

#include "Engine/PlatformSettingsManager.h"
#include "Misc/EnumRange.h"
#include "QinPerformanceStatTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(QinPerformanceSettings)

//////////////////////////////////////////////////////////////////////

UQinPlatformSpecificRenderingSettings::UQinPlatformSpecificRenderingSettings()
{
	MobileFrameRateLimits.Append({ 20, 30, 45, 60, 90, 120 });
}

const UQinPlatformSpecificRenderingSettings* UQinPlatformSpecificRenderingSettings::Get()
{
	UQinPlatformSpecificRenderingSettings* Result = UPlatformSettingsManager::Get().GetSettingsForPlatform<ThisClass>();
	check(Result);
	return Result;
}

//////////////////////////////////////////////////////////////////////

UQinPerformanceSettings::UQinPerformanceSettings()
{
	PerPlatformSettings.Initialize(UQinPlatformSpecificRenderingSettings::StaticClass());

	CategoryName = TEXT("Game");

	DesktopFrameRateLimits.Append({ 30, 60, 120, 144, 160, 165, 180, 200, 240, 360 });

	// Default to all stats are allowed
	FQinPerformanceStatGroup& StatGroup = UserFacingPerformanceStats.AddDefaulted_GetRef();
	for (EQinDisplayablePerformanceStat PerfStat : TEnumRange<EQinDisplayablePerformanceStat>())
	{
		StatGroup.AllowedStats.Add(PerfStat);
	}
}

