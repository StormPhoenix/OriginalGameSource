// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/Controller/JoyCameraMeta.h"
#include "CoreMinimal.h"
#include "JoyCameraControllerBase.h"

#include "JoyCameraInputController.generated.h"

/**
 * 负责更新设备输入控制与弹簧臂调整
 */
UCLASS()
class ORIGINALGAME_API UJoyCameraInputController : public UJoyCameraControllerBase
{
	GENERATED_BODY()

public:
	UJoyCameraInputController();
	
	virtual void UpdateInternal(float DeltaSeconds) override;

	void LockArmRotationYaw();

	void LockArmRotationPitch();

	void LockArmLength();

	void UnlockArmLength();

	void UnlockArmRotationYaw();

	void UnlockArmRotationPitch();

	bool IsArmRotationLocked() const;

	void AddYawInput(float Val);

	void AddPitchInput(float Val);

	void AddDeviceArmLengthInput(float Val);

	void SetConfigs(const TMap<EJoyCameraInput, float>& InputConfig);

	float GetZoomLagSpeed() const
	{
		return ArmZoomLagSpeed;
	}

private:
	void UpdateRotationInput(float DeltaSeconds);

	void UpdateZoomInput(float DeltaSeconds);

	bool FovInputReversed(float StartFov, float EndFov) const;

	bool ArmInputReversed() const;

	void ResetDeviceInput();

	// 当帧是否有镜头 yaw 输入
	bool bPitchInput = false;

	// 当帧镜头 yaw 输入值
	float YawInputValue = 0;
	float PitchInputValue = 0;

	// 缓存的累计镜头输入
	float CachedAccYawInput = 0;
	float CachedAccPitchInput = 0;

	// 每帧缓存的设备输入值
	bool bArmLengthInput{false};
	float ArmZoomValue{0};

	// Arm length 缓动配置
	float DesiredZoomArmLength{0.};
	float CurrentZoomArmLength{0.};

	// Fov 缓动配置
	float DesiredZoomFov{0.};
	float CurrentZoomFov{0.};

	// Arm length 是否处于 zooming 状态
	bool bArmLengthZooming{false};

	float FovOnHitFace{30.};

	/* ****************** Lock input begin **************** **/
	int8 LockArmLengthStack = {0};

	int8 LockArmRotationYawStack = {0};

	bool bLockArmRotationPitch = false;
	int8 LockArmRotationPitchStack = {0};
	/* ****************** Lock input end **************** **/

	// 设备滚轮速度
	float ArmZoomSpeed = 1.;
	float FovZoomSpeed = 1.;
	// 设备滚轮修改弹簧臂时的速度
	float ArmZoomLagSpeed = 1.;
	// 设备滚轮修改 Fov 时的速度
	float FovSpeedOnHitFace = 1.;
	// 镜头输入缓动系数
	float RotationInputSmoothFactor = 0.55;
	// 镜头方向灵敏度 Yaw
	float SensitivityYaw = 1.;
	// 镜头方向灵敏度 Pitch
	float SensitivityPitch = 1.;
	// 最小镜头方向输入速率
	float MinRotationInputSpeed = 0.01;
	// 最大镜头方向输入速率
	float MaxRotationInputSpeed = 6.;
	float MinRotationInputSpeed_LockTarget = 1.;
	// 检查角色是否处于移动时相机臂修正状态
	bool bInArmLengthCorrectionOnMove = false;

	bool bProcessFov{false};
};
