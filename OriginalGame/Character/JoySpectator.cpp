// Fill out your copyright notice in the Description page of Project Settings.

#include "JoySpectator.h"

#include "GameplayTagContainer.h"
#include "Input/JoyInputBlocker.h"
#include "Input/JoyInputReceiver.h"
#include "JoyCharacter.h"

AJoySpectator::AJoySpectator()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AJoySpectator::BeginPlay()
{
	Super::BeginPlay();
}

void AJoySpectator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AJoySpectator::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AJoySpectator::Input_Move(const FInputActionValue& InputActionValue)
{
	if (!UJoyInputBlocker::BlockMoveInput(InputBlockers, this, InputActionValue))
	{
		UJoyInputReceiver::ReceiveMoveInput(InputReceivers, this, InputActionValue);
	}
}

void AJoySpectator::Input_LookMove(const FInputActionValue& InputActionValue)
{
	if (!UJoyInputBlocker::BlockLookMoveInput(InputBlockers, this, InputActionValue))
	{
		UJoyInputReceiver::ReceiveLookMoveInput(InputReceivers, this, InputActionValue);
	}
}

void AJoySpectator::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (!UJoyInputBlocker::BlockAbilityTagPressInput(InputBlockers, this, InputTag))
	{
		UJoyInputReceiver::ReceiveAbilityTagPressInput(InputReceivers, this, InputTag);
	}
}

void AJoySpectator::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!UJoyInputBlocker::BlockAbilityTagReleaseInput(InputBlockers, this, InputTag))
	{
		UJoyInputReceiver::ReceiveAbilityTagReleaseInput(InputReceivers, this, InputTag);
	}
}

void AJoySpectator::RegisterInputReceiver(const TScriptInterface<IJoyInputReceiver> Receiver)
{
	if (Receiver.GetObject())
	{
		InputReceivers.AddUnique(Receiver.GetObject());
	}
}

void AJoySpectator::UnregisterInputReceiver(const TScriptInterface<IJoyInputReceiver> Receiver)
{
	if (Receiver.GetObject())
	{
		InputReceivers.Remove(Receiver.GetObject());
	}
}

void AJoySpectator::UnregisterInputBlocker(const TScriptInterface<IJoyInputBlocker> Blocker)
{
	if (Blocker.GetObject())
	{
		InputBlockers.Remove(Blocker.GetObject());
	}
}

void AJoySpectator::RegisterInputBlocker(const TScriptInterface<IJoyInputBlocker> Blocker)
{
	if (Blocker.GetObject())
	{
		InputBlockers.AddUnique(Blocker.GetObject());
	}
}
