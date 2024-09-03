// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"

class UObject;

ORIGINALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJoy, Log, All);
ORIGINALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJoyExperience, Log, All);
ORIGINALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJoyAbilitySystem, Log, All);
ORIGINALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJoyCamera, Log, All);
ORIGINALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJoyTimeDilation, Log, All);

ORIGINALGAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
