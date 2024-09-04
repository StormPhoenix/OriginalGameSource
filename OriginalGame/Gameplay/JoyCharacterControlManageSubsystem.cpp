// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyCharacterControlManageSubsystem.h"

#include "Character/JoyCharacter.h"
#include "JoyGameBlueprintLibrary.h"
#include "JoyLogChannels.h"
#include "Player/JoyPlayerController.h"

UJoyCharacterControlManageSubsystem* UJoyCharacterControlManageSubsystem::Get(const UWorld* World)
{
	if (World)
	{
		return UGameInstance::GetSubsystem<UJoyCharacterControlManageSubsystem>(World->GetGameInstance());
	}

	return nullptr;
}

UJoyCharacterControlManageSubsystem* UJoyCharacterControlManageSubsystem::GetCharacterControlManageSubsystem(
	const UObject* WorldContextObject)
{
	if (UWorld const* World =
		GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return UJoyCharacterControlManageSubsystem::Get(World);
	}

	return nullptr;
}

AJoyCharacter* UJoyCharacterControlManageSubsystem::SwitchToCharacter(
	AJoyCharacter* TargetCharacter, FJoyCharacterSwitchExtraParam ExtraParam)
{
	if (!AllowCharacterSwitching())
	{
		return nullptr;
	}

	if (TargetCharacter != nullptr && TargetCharacter != ControlState.CurrentControlCharacter)
	{
		if (AJoyPlayerController* PlayerController = UJoyGameBlueprintLibrary::GetJoyPlayerController(TargetCharacter))
		{
			ControlState.TargetCharacterSwitchTo = TargetCharacter;
			PlayerController->SwitchCharacter(ControlState.CurrentControlCharacter, TargetCharacter, ExtraParam);
			return ControlState.CurrentControlCharacter;
		}
	}

	return nullptr;
}

void UJoyCharacterControlManageSubsystem::OnCharacterSwitchFinished(
	AJoyCharacter* PreviousCharacter, AJoyCharacter* TargetCharacter)
{
	if (PreviousCharacter == ControlState.CurrentControlCharacter &&
	    TargetCharacter == ControlState.TargetCharacterSwitchTo)
	{
		ControlState.LastControlCharacter = ControlState.CurrentControlCharacter;
		ControlState.CurrentControlCharacter = ControlState.TargetCharacterSwitchTo;
		ControlState.TargetCharacterSwitchTo = nullptr;
	}
	else
	{
		UE_LOG(LogJoy, Error,
			TEXT("角色控制权切换过程出错：切换结束时回调接口返回的切换对象与 UJoyCharacterControlManageSubsystem 中保存的不一致。"));
	}
}

bool UJoyCharacterControlManageSubsystem::AllowCharacterSwitching() const
{
	if (const AJoyPlayerController* PlayerController = UJoyGameBlueprintLibrary::GetJoyPlayerController(GetWorld()))
	{
		return !PlayerController->CheckDuringCharacterSwitching() && bAllowSwitchCharacter;
	}

	return bAllowSwitchCharacter;
}
