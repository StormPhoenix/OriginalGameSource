// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "JoyCharacterControlManageSubsystem.generated.h"

class AJoyPlayerController;
class AJoyCharacter;
struct FJoyCharacterSwitchExtraParam;

USTRUCT()
struct FCharacterControlState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AJoyCharacter> CurrentControlCharacter{nullptr};

	UPROPERTY()
	TObjectPtr<AJoyCharacter> TargetCharacterSwitchTo{nullptr};

	UPROPERTY()
	TObjectPtr<AJoyCharacter> LastControlCharacter{nullptr};

	// @TODO 回调绑定要放到 PlayerController 的初始化流程中
	bool bPlayerControllerCallbackBound{false};
};

/**
 * 
 */
UCLASS()
class ORIGINALGAME_API UJoyCharacterControlManageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UJoyCharacterControlManageSubsystem* Get(const UWorld* World);
	static UJoyCharacterControlManageSubsystem* GetCharacterControlManageSubsystem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable)
	AJoyCharacter* SwitchToCharacter(AJoyCharacter* TargetCharacter, FJoyCharacterSwitchExtraParam SwitchParam);

	UFUNCTION(BlueprintCallable)
	AJoyCharacter* GetCurrentControlCharacter() const
	{
		return ControlState.CurrentControlCharacter;
	}

	AJoyCharacter* GetSwitchTargetCharacter() const
	{
		return ControlState.TargetCharacterSwitchTo;
	}

	UFUNCTION()
	void OnCharacterSwitchFinished(AJoyCharacter* PreviousCharacter, AJoyCharacter* TargetCharacter);

	void SetCharacterSwitchEnabled(bool bEnabled)
	{
		bAllowSwitchCharacter = bEnabled;
	}

private:
	bool AllowCharacterSwitching() const;

	void InitializePlayerController(AJoyPlayerController* PlayerController);

private:
	UPROPERTY()
	FCharacterControlState ControlState{};

	bool bAllowSwitchCharacter = true;
};
