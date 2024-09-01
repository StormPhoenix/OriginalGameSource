// Fill out your copyright notice in the Description page of Project Settings.


#include "JoyGameBlueprintLibrary.h"

#include "Character/JoySpectator.h"
#include "Kismet/GameplayStatics.h"
#include "Player/JoyPlayerController.h"

AJoyPlayerController* UJoyGameBlueprintLibrary::GetJoyPlayerController(const UObject* WorldContextObject)
{
	return Cast<AJoyPlayerController>(UGameplayStatics::GetPlayerController(WorldContextObject, 0));
}

void UJoyGameBlueprintLibrary::RegisterInputReceiver(const UObject* WorldContextObject, UObject* Receiver)
{
	if (const auto* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (auto* JoySpectator = Cast<AJoySpectator>(PC->GetSpectatorPawn()))
		{
			JoySpectator->RegisterInputReceiver(Receiver);
		}
	}
}

void UJoyGameBlueprintLibrary::UnregisterInputReceiver(const UObject* WorldContextObject, UObject* Receiver)
{
	if (const auto* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (auto* JoySpectator = Cast<AJoySpectator>(PC->GetSpectatorPawn()))
		{
			JoySpectator->UnregisterInputReceiver(Receiver);
		}
	}
}
