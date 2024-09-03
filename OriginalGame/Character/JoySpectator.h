// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JoySpectatorBase.h"
#include "Input/JoyInputBlocker.h"
#include "Input/JoyInputReceiver.h"
#include "JoySpectator.generated.h"

UCLASS()
class ORIGINALGAME_API AJoySpectator : public AJoySpectatorBase
{
	GENERATED_BODY()

public:
	AJoySpectator();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Input_AbilityInputTagPressed(FGameplayTag InputTag) override;
	virtual void Input_AbilityInputTagReleased(FGameplayTag InputTag) override;

	virtual void Input_Move(const FInputActionValue& InputActionValue) override;
	virtual void Input_LookMove(const FInputActionValue& InputActionValue) override;

	void RegisterInputReceiver(const TScriptInterface<IJoyInputReceiver> Receiver);

	void UnregisterInputReceiver(const TScriptInterface<IJoyInputReceiver> Receiver);
	
	void RegisterInputBlocker(const TScriptInterface<IJoyInputBlocker> Blocker);

	void UnregisterInputBlocker(const TScriptInterface<IJoyInputBlocker> Blocker);

private:
	UPROPERTY()
	TArray<TObjectPtr<UObject>> InputReceivers{};

	UPROPERTY()
	TArray<TObjectPtr<UObject>> InputBlockers{};
};
