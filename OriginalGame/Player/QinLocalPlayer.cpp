// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/QinLocalPlayer.h"

#include "AudioMixerBlueprintLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Settings/QinSettingsLocal.h"
#include "Settings/QinSettingsShared.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(QinLocalPlayer)

class UObject;

UQinLocalPlayer::UQinLocalPlayer()
{
}

void UQinLocalPlayer::PostInitProperties()
{
	Super::PostInitProperties();
}

void UQinLocalPlayer::SwitchController(class APlayerController* PC)
{
	Super::SwitchController(PC);

	OnPlayerControllerChanged(PlayerController);
}

bool UQinLocalPlayer::SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
{
	const bool bResult = Super::SpawnPlayActor(URL, OutError, InWorld);

	OnPlayerControllerChanged(PlayerController);

	return bResult;
}

void UQinLocalPlayer::InitOnlineSession()
{
	OnPlayerControllerChanged(PlayerController);

	Super::InitOnlineSession();
}

void UQinLocalPlayer::OnPlayerControllerChanged(APlayerController* NewController)
{
}

UQinSettingsLocal* UQinLocalPlayer::GetLocalSettings() const
{
	return UQinSettingsLocal::Get();
}

UQinSettingsShared* UQinLocalPlayer::GetSharedSettings() const
{
	if (!SharedSettings)
	{
		SharedSettings = UQinSettingsShared::LoadOrCreateSettings(this);
	}

	return SharedSettings;
}

void UQinLocalPlayer::OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId)
{
	FOnCompletedDeviceSwap DevicesSwappedCallback;
	DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedAudioDeviceSwap"));
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetWorld(), InAudioOutputDeviceId, DevicesSwappedCallback);
}

void UQinLocalPlayer::OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{
	if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Failure)
	{
	}
}
