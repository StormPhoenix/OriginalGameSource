// Copyright Epic Games, Inc. All Rights Reserved.

#include "JoyInputConfig.h"

#include "JoyLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(JoyInputConfig)

UJoyInputConfig::UJoyInputConfig(const FObjectInitializer& ObjectInitializer)
{
}

const UInputAction* UJoyInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FJoyInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogJoy, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."),
			*InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const UInputAction* UJoyInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FJoyInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogJoy, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."),
			*InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
