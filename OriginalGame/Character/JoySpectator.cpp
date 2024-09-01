// Fill out your copyright notice in the Description page of Project Settings.


#include "JoySpectator.h"

#include "GameplayTagContainer.h"
#include "Input/JoyInputReceiver.h"


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
	UJoyInputReceiver::ReceiveMoveInput(InputReceivers, this, InputActionValue);
}

void AJoySpectator::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	UJoyInputReceiver::ReceiveLookMouseInput(InputReceivers, this, InputActionValue);
}

void AJoySpectator::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	UJoyInputReceiver::ReceiveAbilityTagPressInput(InputReceivers, this, InputTag);
}

void AJoySpectator::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	UJoyInputReceiver::ReceiveAbilityTagReleaseInput(InputReceivers, this, InputTag);
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
