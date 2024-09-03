// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Camera/CameraMode/JoyCameraMode.h"
#include "JoyCameraModeStack.generated.h"

class UJoyCameraMode;

UENUM()
enum class ECameraModePushPopOption : uint8
{
	Push = 0,
	Pop = 1,
};

struct FCameraModeAction
{
	TSubclassOf<UJoyCameraMode> CameraModeClass;
	bool bBlending;
	ECameraModePushPopOption OP;
};

/**
 * UJoyCameraModeStack
 *
 *	Stack used for blending camera modes.
 */
UCLASS()
class ORIGINALGAME_API UJoyCameraModeStack : public UObject
{
	GENERATED_BODY()

	friend class UJoyCameraComponent;

public:
	UJoyCameraModeStack();

	void ActivateStack();
	void DeactivateStack();

	bool IsStackActivate() const
	{
		return bIsActive;
	}

	void UpdateCameraStack();

	bool EvaluateStack(float DeltaTime, FJoyCameraModeView& OutCameraModeView);

	// Gets the tag associated with the top layer and the blend weight of it
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

	UJoyCameraMode* GetCameraModeInstance(TSubclassOf<UJoyCameraMode> CameraModeClass);

	void PushCameraMode(TSubclassOf<UJoyCameraMode> CameraModeClass, bool bBlend = true);

	void RemoveTopCameraMode();

	int32 GetCameraStackNum();

	UJoyCameraMode* GetTopCameraMode();

	TSubclassOf<UJoyCameraMode> GetDefaultCameraModeClass();

	void AddCameraMode(TSubclassOf<UJoyCameraMode> CameraModeClass, bool bBlend = true);

	void RemoveCameraMode(TSubclassOf<UJoyCameraMode> CameraModeClass, bool bBlend = true);

protected:
	int UpdateStack(float DeltaTime);

	void BlendStack(FJoyCameraModeView& OutCameraModeView, const int RemoveCount) const;

private:
	bool bIsActive;

	UPROPERTY()
	TArray<TObjectPtr<UJoyCameraMode>> CameraModeInstances;

	UPROPERTY()
	TArray<TObjectPtr<UJoyCameraMode>> CameraModeStack;

	TArray<FCameraModeAction> CameraModeOperations;
};
