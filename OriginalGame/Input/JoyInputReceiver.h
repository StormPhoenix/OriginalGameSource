// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "JoyInputReceiver.generated.h"

class IJoyInputReceiver;
struct FGameplayTag;
struct FInputActionValue;

UINTERFACE(Blueprintable, MinimalAPI)
class UJoyInputReceiver : public UInterface
{
	GENERATED_BODY()

public:
	static void ReceiveMoveInput(TConstArrayView<TObjectPtr<UObject>> const& Receivers, UObject* InputReceiver,
		FInputActionValue const& InputActionValue);

	static void ReceiveAbilityTagPressInput(
		TConstArrayView<TObjectPtr<UObject>> const& Receivers, UObject* InputReceiver, FGameplayTag const& InputTag);

	static void ReceiveAbilityTagReleaseInput(
		TConstArrayView<TObjectPtr<UObject>> const& Receivers, UObject* InputReceiver, FGameplayTag const& InputTag);

	static void ReceiveLookMoveInput(TConstArrayView<TObjectPtr<UObject>> const& Receivers, UObject* InputReceiver,
		const FInputActionValue& InputActionValue);

	static void ReceiveMouseScrollInput(TConstArrayView<TObjectPtr<UObject>> const& Receivers, UObject* InputReceiver,
		const FInputActionValue& InputActionValue);
};

class ORIGINALGAME_API IJoyInputReceiver
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Joy|Input")
	void ReceiveMoveInput(UObject* InputReceiver, const FInputActionValue& InputActionValue);
	virtual void ReceiveMoveInput_Implementation(UObject* InputReceiver, const FInputActionValue& InputActionValue);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Joy|Input")
	void ReceiveAbilityTagPressInput(UObject* InputReceiver, FGameplayTag const& InputTag);
	virtual void ReceiveAbilityTagPressInput_Implementation(UObject* InputReceiver, FGameplayTag const& InputTag);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Joy|Input")
	void ReceiveAbilityTagReleaseInput(UObject* InputReceiver, FGameplayTag const& InputTag);
	virtual void ReceiveAbilityTagReleaseInput_Implementation(UObject* InputReceiver, FGameplayTag const& InputTag);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Joy|Input")
	void ReceiveLookMoveInput(UObject* InputReceiver, const FInputActionValue& InputActionValue);
	virtual void ReceiveLookMoveInput_Implementation(
		UObject* InputReceiver, const FInputActionValue& InputActionValue);
};
