// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "InputActionValue.h"
#include "JoySpectatorBase.generated.h"

class UJoyInputComponent;
class UJoyInputConfig;
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

	void BindDefaultInputMappings(UEnhancedInputLocalPlayerSubsystem* EnhanceInput,
	                              UInputComponent* InInputComponent);

	virtual void BindDefaultInputMappings_Impl(UJoyInputComponent* JoyIC, const UJoyInputConfig* InputConfig);

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	virtual void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	virtual void Input_Move(const FInputActionValue& InputActionValue);
	virtual void Input_LookMouse(const FInputActionValue& InputActionValue);
	virtual void Input_LookStick(const FInputActionValue& InputActionValue);
};
