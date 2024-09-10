// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/Controller/JoyCameraModifierController.h"
#include "Camera/JoyCameraData.h"
#include "CoreMinimal.h"

#include "JoyCameraBlueprintLibrary.generated.h"

class UCameraComponent;
class AJoyCharacter;
class AJoyPlayerCameraManager;

/**
 *
 */
UCLASS()
class ORIGINALGAME_API UJoyCameraBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 对附加到 OwnerActor 身上的相机参数进行修改
	 * @param OwnerActor 相机附加对象
	 * @param Tag
	 * @param Duration 被修改参数生效持续时间
	 * @param BlendInTime 参数从未修改状态过渡到目标值所需时间
	 * @param BlendOutTime 参数从修改后的值过渡到未修改值所需时间
	 * @param CameraModifiers 参数修改数据
	 * @param bNeedManualBreak 是否需要手动中断修改过程，当为 true 时代表你需要手动调用 @function{BreakCameraModifier}
	 * 使参数进入 BlendOut 状态
	 */
	UFUNCTION(BlueprintCallable, Category = Camera)
	static FCameraModifyHandle ApplyCameraSettings(AActor* OwnerActor, float Duration, float BlendInTime,
		float BlendOutTime, FCameraModifiers const& CameraModifiers, bool bNeedManualBreak = false);

	UFUNCTION(BlueprintCallable, Category = Camera)
	static void ApplyCameraSettings_Immediately(AActor* OwnerActor, FCameraModifiers const& CameraModifiers);

	UFUNCTION(BlueprintCallable, Category = Camera)
	static void ManualBreakCameraModifier(AActor* OwnerActor, FCameraModifyHandle ModifyHandler);

	UFUNCTION(BlueprintPure, Category = "Camera", DisplayName = "目标是否在屏幕内",
		meta = (WorldContext = "WorldContextObject"))
	static bool CheckTargetInsideScreen(const UObject* WorldContextObject, const AActor* Target);

	UFUNCTION(Blueprintable, Category = "Camera", meta = (WorldContext = "WorldContextObject"))
	static AJoyPlayerCameraManager* GetJoyPlayerCameraManager(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Camera", DisplayName = "获取目标本帧镜头数据")
	static void GetCameraViewFromTarget(
		AActor* Target, FMinimalViewInfo& OutView, bool bRealTime = false, float DeltaTime = 0.f);

	static void GetCameraConfigMap(
		UObject const* WorldContextObject, UPARAM(ref) TMap<FName, FJoyCameraConfigTable>& ConfigMapRef);	
};
