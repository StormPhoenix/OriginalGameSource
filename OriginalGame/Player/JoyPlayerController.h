// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerController.h"
#include "Camera/JoyPlayerCameraManager.h"
#include "Gameplay/TimeDilation/JoyTimeDilationManageSubsystem.h"
#include "JoyPlayerController.generated.h"

class AJoyCharacter;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayerTargetSwitchFinished, AJoyCharacter* FromCharacter,
                                     AJoyCharacter* ToCharacter);

USTRUCT(BlueprintType)
struct FJoyCharacterSwitchExtraParam
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, DisplayName = "摄像机切换参数")
	EJoyCameraBlendType BlendType{EJoyCameraBlendType::LockTarget};
};

USTRUCT()
struct FCharacterSwitchSpec
{
	GENERATED_BODY()

	UPROPERTY()
	FJoyCharacterSwitchExtraParam ExtraParam;

	UPROPERTY()
	TWeakObjectPtr<AJoyCharacter> From{nullptr};

	UPROPERTY()
	TWeakObjectPtr<AController> FromController{nullptr};

	UPROPERTY()
	TWeakObjectPtr<AJoyCharacter> To{nullptr};

	UPROPERTY()
	TWeakObjectPtr<AController> ToController{nullptr};

	void Reset()
	{
		From = nullptr;
		FromController = nullptr;
		To = nullptr;
		ToController = nullptr;
	}
};

/**
 * 
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class ORIGINALGAME_API AJoyPlayerController : public AModularPlayerController
{
	GENERATED_BODY()

public:
	AJoyPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Joy|PlayerController")
	AJoyPlayerState* GetJoyPlayerState() const;

	bool CheckDuringCharacterSwitching() const
	{
		return bDuringPlayerSwitching;
	}

	void SwitchCharacter(AJoyCharacter* PreviousCharacter, AJoyCharacter* TargetCharacter,
	                     FJoyCharacterSwitchExtraParam ExtraParam);

	UFUNCTION()
	void OnCharacterSwitchFinished(AActor* ViewTarget, AActor* PendingViewTarget);

	FOnPlayerTargetSwitchFinished OnPlayerTargetSwitchFinishedDelegate;

private:
	void DoSwitchCharacter();

	void ApplyTimeDilation(float TimeDilation);

	void ResetTimeDilation();

private:
	// @TODO 挪动到资产配置
	UPROPERTY(EditDefaultsOnly)
	float TimeDilationOfCharacterSwitching{0.1};

	// @TODO 挪动到资产配置
	UPROPERTY(EditDefaultsOnly)
	float TargetSwitchTime{1.0};

private:
	bool bDuringPlayerSwitching{false};

	FCharacterSwitchSpec CharacterSwitchSpec{};

	UPROPERTY()
	FJoyTimeDilationHandle TimeDilationHandle{};
};
