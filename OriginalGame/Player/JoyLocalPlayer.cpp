// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/JoyLocalPlayer.h"

#include "AudioMixerBlueprintLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Settings/JoySettingsLocal.h"
#include "Settings/JoySettingsShared.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(JoyLocalPlayer)

class UObject;

UJoyLocalPlayer::UJoyLocalPlayer()
{
}

void UJoyLocalPlayer::PostInitProperties()
{
	Super::PostInitProperties();
}

void UJoyLocalPlayer::SwitchController(class APlayerController* PC)
{
	Super::SwitchController(PC);

	OnPlayerControllerChanged(PlayerController);
}

bool UJoyLocalPlayer::SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
{
	const bool bResult = Super::SpawnPlayActor(URL, OutError, InWorld);

	OnPlayerControllerChanged(PlayerController);

	return bResult;
}

void UJoyLocalPlayer::InitOnlineSession()
{
	OnPlayerControllerChanged(PlayerController);

	Super::InitOnlineSession();
}

void UJoyLocalPlayer::OnPlayerControllerChanged(APlayerController* NewController)
{
}

UJoySettingsLocal* UJoyLocalPlayer::GetLocalSettings() const
{
	return UJoySettingsLocal::Get();
}

UJoySettingsShared* UJoyLocalPlayer::GetSharedSettings() const
{
	if (!SharedSettings)
	{
		SharedSettings = UJoySettingsShared::LoadOrCreateSettings(this);
	}

	return SharedSettings;
}

void UJoyLocalPlayer::OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId)
{
	FOnCompletedDeviceSwap DevicesSwappedCallback;
	DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedAudioDeviceSwap"));
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetWorld(), InAudioOutputDeviceId, DevicesSwappedCallback);
}

void UJoyLocalPlayer::OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{
	if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Failure)
	{
	}
}
