// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "JoyInputBlocker.generated.h"

class IJoyInputBlocker;
struct FGameplayTag;
struct FInputActionValue;

UINTERFACE(Blueprintable, MinimalAPI)
class UJoyInputBlocker : public UInterface
{
	GENERATED_BODY()

public:
	static bool BlockMoveInput(TConstArrayView<TObjectPtr<UObject>> const& Blockers, UObject* InputReceiver,
		FInputActionValue const& InputActionValue);

	static bool BlockAbilityTagPressInput(
		TConstArrayView<TObjectPtr<UObject>> const& Blockers, UObject* InputReceiver, FGameplayTag const& InputTag);

	static bool BlockAbilityTagReleaseInput(
		TConstArrayView<TObjectPtr<UObject>> const& Blockers, UObject* InputReceiver, FGameplayTag const& InputTag);

	static bool BlockLookMoveInput(TConstArrayView<TObjectPtr<UObject>> const& Blockers, UObject* InputReceiver,
		const FInputActionValue& InputActionValue);

	static bool BlockMouseScrollInput(TConstArrayView<TObjectPtr<UObject>> const& Blockers, UObject* InputReceiver,
		const FInputActionValue& InputActionValue);
};

/**
 * 输入拦截接口
 */
class ORIGINALGAME_API IJoyInputBlocker
{
	GENERATED_BODY()

public:
	/**
	 * 是否拦截移动输入事件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Joy|Input")
	bool BlockMoveInput(UObject* InputReceiver, const FInputActionValue& InputActionValue);
	virtual bool BlockMoveInput_Implementation(UObject* InputReceiver, const FInputActionValue& InputActionValue);

	/**
	 * 是否拦截使用指定能力事件，这个逻辑会比 BlockSkillIDInput 更早触发。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Joy|Input")
	bool BlockAbilityTagPressInput(UObject* InputReceiver, FGameplayTag const& InputTag);
	virtual bool BlockAbilityTagPressInput_Implementation(UObject* InputReceiver, FGameplayTag const& InputTag);

	/**
	 * 是否拦截使用指定能力事件，这个逻辑会比 BlockSkillIDInput 更早触发。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Joy|Input")
	bool BlockAbilityTagReleaseInput(UObject* InputReceiver, FGameplayTag const& InputTag);
	virtual bool BlockAbilityTagReleaseInput_Implementation(UObject* InputReceiver, FGameplayTag const& InputTag);

	/**
	 * 是否拦截视角移动事件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Joy|Input")
	bool BlockLookMoveInput(UObject* InputReceiver, const FInputActionValue& InputActionValue);
	virtual bool BlockLookMoveInput_Implementation(UObject* InputReceiver, const FInputActionValue& InputActionValue);

	/**
	 * 是否拦截鼠标滚动事件。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Joy|Input")
	bool BlockMouseScrollInput(UObject* InputReceiver, const FInputActionValue& InputActionValue);
	virtual bool BlockMouseScrollInput_Implementation(UObject* InputReceiver, const FInputActionValue& InputActionValue);
};