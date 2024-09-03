// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyInputReceiver.h"

void UJoyInputReceiver::ReceiveMoveInput(TConstArrayView<TObjectPtr<UObject>> const& Receivers, UObject* InputReceiver,
                                         FInputActionValue const& InputActionValue)
{
	for (UObject* Receiver : Receivers)
	{
		if (!Receiver)
		{
			continue;
		}

		IJoyInputReceiver::Execute_ReceiveMoveInput(Receiver, InputReceiver, InputActionValue);
	}
}

void UJoyInputReceiver::ReceiveAbilityTagPressInput(
	TConstArrayView<TObjectPtr<UObject>> const& Receivers,
	UObject* InputReceiver, FGameplayTag const& InputTag)
{
	for (UObject* Receiver : Receivers)
	{
		if (!Receiver)
		{
			continue;
		}

		IJoyInputReceiver::Execute_ReceiveAbilityTagPressInput(Receiver, InputReceiver, InputTag);
	}
}

void UJoyInputReceiver::ReceiveAbilityTagReleaseInput(
	TConstArrayView<TObjectPtr<UObject>> const& Receivers,
	UObject* InputReceiver, FGameplayTag const& InputTag)
{
	for (UObject* Receiver : Receivers)
	{
		if (!Receiver)
		{
			continue;
		}

		IJoyInputReceiver::Execute_ReceiveAbilityTagReleaseInput(Receiver, InputReceiver, InputTag);
	}
}

void UJoyInputReceiver::ReceiveLookMoveInput(
	TConstArrayView<TObjectPtr<UObject>> const& Receivers,
	UObject* InputReceiver, const FInputActionValue& InputActionValue)
{
	for (UObject* Receiver : Receivers)
	{
		if (!Receiver)
		{
			continue;
		}

		IJoyInputReceiver::Execute_ReceiveLookMouseInput(Receiver, InputReceiver, InputActionValue);
	}
}

void IJoyInputReceiver::ReceiveAbilityTagPressInput_Implementation(UObject* InputReceiver,
                                                                   FGameplayTag const& InputTag)
{
}

void IJoyInputReceiver::ReceiveAbilityTagReleaseInput_Implementation(UObject* InputReceiver,
                                                                     FGameplayTag const& InputTag)
{
}

void IJoyInputReceiver::ReceiveMoveInput_Implementation(UObject* InputReceiver,
                                                        const FInputActionValue& InputActionValue)
{
}

void IJoyInputReceiver::ReceiveLookMouseInput_Implementation(UObject* InputReceiver,
                                                             const FInputActionValue& InputActionValue)
{
}
