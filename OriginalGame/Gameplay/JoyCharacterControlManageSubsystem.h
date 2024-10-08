// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "JoyCharacterControlManageSubsystem.generated.h"

class AJoyPlayerController;
class AJoyCharacter;
struct FJoyCharacterSwitchExtraParam;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnControlCharacterChanged, AJoyCharacter*, FromCharacter, AJoyCharacter*, ToCharacter);

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

	UPROPERTY(BlueprintAssignable)
	FOnControlCharacterChanged OnControlCharacterChangedDelegate{};

	UFUNCTION()
	void OnCharacterSwitchFinished(AJoyCharacter* PreviousCharacter, AJoyCharacter* TargetCharacter);
	
	void SetCharacterSwitchEnabled(bool bEnabled)
	{
		bAllowSwitchCharacter = bEnabled;
	}

private:
	bool AllowCharacterSwitching() const;

private:
	UPROPERTY()
	FCharacterControlState ControlState{};

	bool bAllowSwitchCharacter = true;
};
