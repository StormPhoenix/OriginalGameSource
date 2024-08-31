// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"

class UObject;

ORIGINALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogQin, Log, All);
ORIGINALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogQinExperience, Log, All);
ORIGINALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogQinAbilitySystem, Log, All);
ORIGINALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogQinTeams, Log, All);

ORIGINALGAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
