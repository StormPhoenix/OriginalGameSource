// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "JoyCameraMeta.generated.h"

/**
 * 相机基础参数
 */
UENUM(Blueprintable)
enum class EJoyCameraBasic : uint8
{
	NotUsed = 0 UMETA(DisplayName = "占位符，不使用"),
	BaseArmLength = 1 UMETA(DisplayName = "相机臂基础臂长"),
	MinArmLength = 2 UMETA(DisplayName = "相机臂最小臂长"),
	MaxArmLength = 3 UMETA(DisplayName = "相机臂最大臂长"),
	OverlayArmLength = 4 UMETA(DisplayName = "相机臂叠加臂长"),
	BaseFov = 5 UMETA(DisplayName = "基础 Fov"),

	ArmCenterOffsetX = 6 UMETA(DisplayName = "相机臂中心偏移 X"),
	ArmCenterOffsetY = 7 UMETA(DisplayName = "相机臂中心偏移 Y"),
	ArmCenterOffsetZ = 8 UMETA(DisplayName = "相机臂中心偏移 Z"),

	ArmCenterLagRecoverSpeed = 9 UMETA(DisplayName = "角色移动时相机臂中心滞后恢复速度"),
	ArmCenterLagMaxHorizontalDistance = 10 UMETA(DisplayName = "角色移动时相机臂中心滞后最大距离(水平)"),
	ArmCenterLagMaxVerticalDistance = 11 UMETA(DisplayName = "相机方向输入时相机旋转的滞后恢复速度"),

	MinArmPitch = 12 UMETA(DisplayName = "相机臂最小 Pitch"),
	MaxArmPitch = 13 UMETA(DisplayName = "相机臂最大 Pitch"),

	ArmRotationLagRecoverSpeed = 14 UMETA(DisplayName = "相机方向输入时相机旋转的滞后恢复速度"),

	ViewTargetSwitchTime = 15 UMETA(DisplayName = "相机臂中心切换过度时间"),
};

/**
 * 设备输入参数
 */
UENUM(Blueprintable)
enum class EJoyCameraInput : uint8
{
	NotUsed = 0 UMETA(DisplayName = "占位符"),
	ArmZoomSpeed = 1 UMETA(DisplayName = "滚轮轴影响相机臂弹性系数"),
	RotationInputSmoothFactor = 2 UMETA(DisplayName = "镜头方向输入缓动系数"),
	SensitivityYaw = 3 UMETA(DisplayName = "镜头方向灵敏度 Yaw"),
	SensitivityPitch = 4 UMETA(DisplayName = "镜头方向灵敏度 Pitch"),
	MinRotationInputSpeed = 5 UMETA(DisplayName = "最小镜头方向输入速率"),
	MaxRotationInputSpeed = 6 UMETA(DisplayName = "最大镜头方向输入速率"),
	ArmZoomLagSpeed = 7 UMETA(DisplayName = "滚轮轴影响相机臂长度的变化延迟速度"),
	FovZoomSpeed = 8 UMETA(DisplayName = "滚轮轴影响 Fov 弹性系数"),
	FovOnHitFace = 9 UMETA(DisplayName = "相机臂缩短到脸部最近距离时的 Fov 值"),
};

/**
 * 镜头类型
 */
UENUM(Blueprintable)
enum class EJoyCameraType : uint8
{
	Basic = 0 UMETA(DisplayName = "基础镜头"),
	Sub = 1 UMETA(DisplayName = "子镜头"),
	NumMax UMETA(Hidden),
};

USTRUCT(BlueprintType)
struct FJoyCameraConfigTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", DisplayName = "镜头描述")
	FString Description{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", DisplayName = "镜头优先级")
	int32 Priority{0};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", DisplayName = "淡入时间")
	float FadeInTime{0.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", DisplayName = "淡出时间")
	float FadeOutTime{0.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", DisplayName = "淡入淡出是否忽略时间膨胀")
	bool bIgnoreTimeDilation{true};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", DisplayName = "淡入时屏蔽镜头输入")
	bool bOverrideCameraInput{true};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", DisplayName = "移动时是否允许相机臂中心滞后")
	bool bArmCenterLagEnable{false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", DisplayName = "镜头基础参数")
	TMap<EJoyCameraBasic, float> Basic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", DisplayName = "设备输入参数")
	TMap<EJoyCameraInput, float> Input;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", DisplayName = "镜头类型")
	EJoyCameraType Type = EJoyCameraType::Basic;
};

#define UPDATE_INPUT_CONFIGS(Key)             \
	if (Config.Contains(EJoyCameraInput::Key)) \
	{                                         \
		Key = Config[EJoyCameraInput::Key];    \
	}

#define UPDATE_BASIC_CONFIGS(Key)             \
	if (Config.Contains(EJoyCameraBasic::Key)) \
	{                                         \
		Key = Config[EJoyCameraBasic::Key];    \
	}
