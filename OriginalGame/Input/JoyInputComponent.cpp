// Copyright Epic Games, Inc. All Rights Reserved.

#include "JoyInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Player/JoyLocalPlayer.h"
#include "Settings/JoySettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(JoyInputComponent)

class UJoyInputConfig;

UJoyInputComponent::UJoyInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UJoyInputComponent::AddInputMappings(const UJoyInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	UJoyLocalPlayer* LocalPlayer = InputSubsystem->GetLocalPlayer<UJoyLocalPlayer>();
	check(LocalPlayer);

	// Add any registered input mappings from the settings!
	if (const UJoySettingsLocal* LocalSettings = UJoySettingsLocal::Get())
	{	
		// Tell enhanced input about any custom keymappings that the player may have customized
		for (const TPair<FName, FKey>& Pair : LocalSettings->GetCustomPlayerInputConfig())
		{
			if (Pair.Key != NAME_None && Pair.Value.IsValid())
			{
				InputSubsystem->AddPlayerMappedKeyInSlot(Pair.Key, Pair.Value);
			}
		}
	}
}

void UJoyInputComponent::RemoveInputMappings(const UJoyInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	UJoyLocalPlayer* LocalPlayer = InputSubsystem->GetLocalPlayer<UJoyLocalPlayer>();
	check(LocalPlayer);
	
	if (UJoySettingsLocal* LocalSettings = UJoySettingsLocal::Get())
	{
		// Remove any registered input contexts
		const TArray<FLoadedMappableConfigPair>& Configs = LocalSettings->GetAllRegisteredInputConfigs();
		for (const FLoadedMappableConfigPair& Pair : Configs)
		{
			InputSubsystem->RemovePlayerMappableConfig(Pair.Config);
		}
		
		// Clear any player mapped keys from enhanced input
		for (const TPair<FName, FKey>& Pair : LocalSettings->GetCustomPlayerInputConfig())
		{
			InputSubsystem->RemovePlayerMappedKeyInSlot(Pair.Key);
		}
	}
}

void UJoyInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
