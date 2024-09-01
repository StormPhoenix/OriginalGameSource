// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "InputActionValue.h"
#include "JoySpectatorBase.generated.h"

class UEnhancedInputLocalPlayerSubsystem;

UCLASS()
class ORIGINALGAME_API AJoySpectatorBase : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	static const FName Name_BindInputsNow;
	static const FName Name_ActorFeatureName;

	AJoySpectatorBase();

protected:
	virtual void BeginPlay() override;

	virtual void BindDefaultInputMappings(UEnhancedInputLocalPlayerSubsystem* EnhanceInput,
	                                      UInputComponent* InInputComponent);

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	void Input_LookStick(const FInputActionValue& InputActionValue);
};
