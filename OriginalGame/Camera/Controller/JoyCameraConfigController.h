// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JoyCameraControllerBase.h"
#include "UObject/Object.h"

#include "JoyCameraConfigController.generated.h"

struct FJoyCameraConfigTable;

UCLASS()
class ORIGINALGAME_API UCameraConfig : public UObject
{
	GENERATED_BODY()

public:
	// 更新至此配置需要的时间
	float FadeInTime{0.f};

	// 离开此配置需要的时间
	float FadeOutTime{0.f};

	bool bIgnoreTimeDilation{false};

	bool bFadeArmPitch{false};

	float FadeTargetArmPitch{0.};

	bool bOverrideCameraInput{true};

	bool bArmCenterLagEnable{false};

	// 镜头优先级
	int32 Priority{0};

	// 默认相机参数
	TMap<EJoyCameraBasic, float> BasicConfig;

	TMap<EJoyCameraInput, float> InputConfig;

	// 相机类型
	EJoyCameraType Type;

	FString Description{};

	UCameraConfig();

	void LoadProto(const FJoyCameraConfigTable& Config);
};

UCLASS()
class ORIGINALGAME_API UJoyCameraConfigController : public UJoyCameraControllerBase
{
	GENERATED_BODY()

	friend class AJoyPlayerCameraManager;

public:
	// 默认相机参数
	UPROPERTY()
	UCameraConfig* DefaultConfig = nullptr;

	UJoyCameraConfigController();

	virtual void InitializeFor(AJoyPlayerCameraManager* PCMgr) override;

	virtual void UpdateInternal(float DeltaSeconds) override;

	void MarkDirty();

private:
	float ConfigFadeDurationTime{0};

	bool bFadeArmPitch{false};

	bool bArmCenterLagEnable{false};

	float FadeTargetArmPitch{0.};

protected:
	void LoadConfig();

	void UpdateConfig(bool bForceUpdate = false);

private:
	void UpdateConfigData(const UCameraConfig* Config);

	UPROPERTY()
	TMap<FName, UCameraConfig*> CachedConfigMap;

	bool bConfigDirty = false;
};
