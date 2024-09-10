// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyCameraInputController.h"

#include "JoyLogChannels.h"
#include "Camera/JoyPlayerCameraManager.h"
#include "Character/JoyCharacter.h"
#include "Gameplay/JoyCharacterControlManageSubsystem.h"
#include "Player/JoyPlayerController.h"

UJoyCameraInputController::UJoyCameraInputController()
{
}

void UJoyCameraInputController::UpdateInternal(float DeltaSeconds)
{
	Super::UpdateInternal(DeltaSeconds);

	UpdateRotationInput(DeltaSeconds);
	UpdateZoomInput(DeltaSeconds);

	// 重置设备输入
	ResetDeviceInput();
}

void UJoyCameraInputController::UpdateRotationInput(float DeltaSeconds)
{
	if (CameraManager == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("按键输入错误：Player Camera Manager 为空"));
		return;
	}

	auto* PlayerCtrl = Cast<AJoyPlayerController>(CameraManager->GetOwningPlayerController());
	if (PlayerCtrl == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("按键输入错误：Player Controller 为空"));
		return;
	}

	// 当前的 view target 并不一定是玩家控制的 actor
	const auto* ControlSystem = UJoyCharacterControlManageSubsystem::Get(GetWorld());
	if (!ControlSystem)
	{
		return;
	}
	
	AJoyCharacter* CurrentViewTarget = ControlSystem->GetCurrentControlCharacter();
	if (CurrentViewTarget == nullptr)
	{
		return;
	}

	FViewTargetCameraInfo* ViewTargetCamera = CameraManager->GetViewTargetCameraInfo(CurrentViewTarget);
	if (ViewTargetCamera == nullptr)
	{
		return;
	}

	// 开始缓冲
	const float DeltaTime = FMath::Clamp(DeltaSeconds, 0, 0.05);
	// 鼠标实际输入值 yaw
	const float MoveYaw = YawInputValue * SensitivityYaw * DeltaTime;
	// @TODO 此处乘以 DeltaTime 后可能会有抖动
	// const float MoveYaw = YawInputValue * SensitivityYaw;
	// 鼠标实际输入值 pitch
	const float MovePitch = PitchInputValue * SensitivityPitch * DeltaTime;
	// @TODO 此处乘以 DeltaTime 后可能会有抖动
	// const float MovePitch = PitchInputValue * SensitivityPitch;

	// 每帧的镜头输入需要插值缓动
	CachedAccYawInput = CachedAccYawInput * (1.0 - RotationInputSmoothFactor) + MoveYaw * RotationInputSmoothFactor;
	CachedAccPitchInput = CachedAccPitchInput * (1.0 - RotationInputSmoothFactor) + MovePitch * RotationInputSmoothFactor;

	// 限制镜头最大输入速率
	CachedAccYawInput = FMath::Clamp(CachedAccYawInput, -MaxRotationInputSpeed, MaxRotationInputSpeed);
	CachedAccPitchInput = FMath::Clamp(CachedAccPitchInput, -MaxRotationInputSpeed, MaxRotationInputSpeed);

	// 如果 yaw 输入方向改变了，则将缓存变为 0
	if (CachedAccYawInput * MoveYaw < 0)
	{
		CachedAccYawInput = 0.f;
	}

	if (LockArmRotationYawStack <= 0)
	{
		// 限制镜头最小输入速率
		if (!FMath::IsNearlyZero(CachedAccYawInput, MinRotationInputSpeed))
		{
			// 移动输入下一帧生效
			PlayerCtrl->AddYawInput(CachedAccYawInput);
			ViewTargetCamera->bArmYaw_HasModified = true;
		}
	}

	if (LockArmRotationPitchStack <= 0 && bPitchInput)
	{
		// 限制镜头最小输入速率
		if (!FMath::IsNearlyZero(CachedAccPitchInput, MinRotationInputSpeed))
		{
			// 移动输入下一帧生效
			PlayerCtrl->AddPitchInput(CachedAccPitchInput);
			ViewTargetCamera->bArmPitch_HasModified = true;
		}
	}

	/**
	 * 首先说明为什么要让镜头方向与 player controller 同步。
	 * 第三人称模式下，镜头方向要与 player controller 方向同步，这是因为第三人称需要依赖 player controller
	 * 方向计算按键输入方向，确保角色移动正确。而角色在画面中移动正确这一点，又让镜头方向与 player controller
	 * 方向需要保持同步。
	 *
	 * 其次，当镜头 attch 到非 player controller 控制的 actor 上时，就不能让 player controller 影响到
	 * 当前的 actor 的镜头了，所以在前段代码中，我们不能取 ViewTarget.Target，而是要从 SquadSystem 获取当前
	 * PlayerController 操纵的 actor.
	 *
	 * 第三，当控制角色是 PendingViewTarget 时候，业务逻辑会希望在镜头混合过程中，修改 PendingViewTarget 的
	 * 值，此时不能用 player controller 的方向覆盖 PendingViewTarget 的值。
	 */

	if (CurrentViewTarget != CameraManager->PendingViewTarget.Target)
	{
		ViewTargetCamera->DesiredCamera.ArmCenterRotation = PlayerCtrl->GetControlRotation();
	}
}

bool UJoyCameraInputController::ArmInputReversed() const
{
	return (DesiredZoomArmLength - CurrentZoomArmLength) * ArmZoomValue < 0;
}

bool UJoyCameraInputController::FovInputReversed(float StartFov, float EndFov) const
{
	if ((EndFov - StartFov) > 0)
	{
		return (DesiredZoomFov - CurrentZoomFov) * ArmZoomValue > 0;
	}
	else
	{
		return (DesiredZoomFov - CurrentZoomFov) * ArmZoomValue < 0;
	}
}

void UJoyCameraInputController::UpdateZoomInput(float DeltaSeconds)
{
	if (CameraManager == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("按键输入错误：调用 UpdateZoomInput 时 Player Camera Manager 为空"));
		return;
	}

	const AActor* CurViewTarget = CameraManager->ViewTarget.Target;
	if (CurViewTarget == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("按键输入错误：调用 UpdateZoomInput 时 View Target 为空"));
		return;
	}

	FViewTargetCameraInfo* ViewTargetCamera = CameraManager->GetViewTargetCameraInfo(CurViewTarget);
	if (ViewTargetCamera == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("按键输入错误：调用 UpdateZoomInput 时 View Target 镜头信息为空"));
		return;
	}

	// 首先要限制范围鼠标可操作范围
	const float MinArmLength = CameraManager != nullptr ? CameraManager->GetBaseMinArmLength() : 50;
	const float MaxArmLength = CameraManager != nullptr ? CameraManager->GetBaseMaxArmLength() : 1000;

	// 拿到当前操作的 ViewTarget 的镜头信息，设置 ArmLength
	if (LockArmLengthStack <= 0 && bArmLengthInput)
	{
		// 优先处理 Fov
		if (bProcessFov)
		{
			if (FovInputReversed(CameraManager->BaseFov, FovOnHitFace))
			{
				// 输入方向与变化方向反向
				DesiredZoomFov = CurrentZoomFov;
			}
			else
			{
				const float OldZoomFov = CurrentZoomFov;
				DesiredZoomFov += ArmZoomValue * FovZoomSpeed;
				CurrentZoomFov = FMath::FInterpTo(CurrentZoomFov, DesiredZoomFov, DeltaSeconds, FovSpeedOnHitFace);

				const float MinFov = FovOnHitFace < CameraManager->BaseFov ? FovOnHitFace : CameraManager->BaseFov;
				const float MaxFov = FovOnHitFace > CameraManager->BaseFov ? FovOnHitFace : CameraManager->BaseFov;

				// 更新 Arm Fov
				const float FovZoomDelta = CurrentZoomFov - OldZoomFov;
				ViewTargetCamera->DesiredCamera.Fov += FovZoomDelta;
				ViewTargetCamera->DesiredCamera.Fov = FMath::Clamp(ViewTargetCamera->DesiredCamera.Fov, MinFov, MaxFov);
				ViewTargetCamera->bCameraFov_HasModified = true;

				if (ArmZoomValue > 0 && FMath::IsNearlyEqual(ViewTargetCamera->DesiredCamera.Fov, CameraManager->BaseFov))
				{
					bProcessFov = false;
				}
			}
		}
		else
		{
			if (ArmInputReversed())
			{
				// 输入方向与变化方向反向
				DesiredZoomArmLength = CurrentZoomArmLength;
			}
			else
			{
				const float OldZoomArmLength = CurrentZoomArmLength;
				DesiredZoomArmLength += ArmZoomValue * ArmZoomSpeed;
				CurrentZoomArmLength =
					FMath::FInterpTo(CurrentZoomArmLength, DesiredZoomArmLength, DeltaSeconds, ArmZoomLagSpeed);

				// 更新 Arm Length
				const float ArmLengthZoomDelta = CurrentZoomArmLength - OldZoomArmLength;
				ViewTargetCamera->DesiredCamera.ArmLength += ArmLengthZoomDelta;
				ViewTargetCamera->DesiredCamera.ArmLength =
					FMath::Clamp(ViewTargetCamera->DesiredCamera.ArmLength, MinArmLength, MaxArmLength);
				ViewTargetCamera->bCameraArmLength_HasModified = true;

				if (ArmZoomValue < 0 && FMath::IsNearlyEqual(ViewTargetCamera->DesiredCamera.ArmLength, MinArmLength) &&
					CameraManager->BaseFov != FovOnHitFace)
				{
					// 此时臂长已经缩短到了最小值，开启调整 Fov
					bProcessFov = true;
				}
			}
		}

		bArmLengthZooming = true;
	}
	else if (bArmLengthZooming)
	{
		if (bProcessFov)
		{
			const float MinFov = FovOnHitFace < CameraManager->BaseFov ? FovOnHitFace : CameraManager->BaseFov;
			const float MaxFov = FovOnHitFace > CameraManager->BaseFov ? FovOnHitFace : CameraManager->BaseFov;

			if (FMath::Abs(DesiredZoomFov - CurrentZoomFov) < UE_SMALL_NUMBER)
			{
				// Fov 修正完毕
				ViewTargetCamera->DesiredCamera.Fov += (DesiredZoomFov - CurrentZoomFov);
				ViewTargetCamera->DesiredCamera.Fov = FMath::Clamp(ViewTargetCamera->DesiredCamera.Fov, MinFov, MaxFov);
				ViewTargetCamera->bCameraFov_HasModified = true;

				// 清理 Fov 状态
				bArmLengthZooming = false;
				DesiredZoomFov = 0.f;
				CurrentZoomFov = 0.f;
			}
			else
			{
				const float OldZoomArmFov = CurrentZoomFov;
				CurrentZoomFov = FMath::FInterpTo(CurrentZoomFov, DesiredZoomFov, DeltaSeconds, FovSpeedOnHitFace);
				const float FovZoomDelta = CurrentZoomFov - OldZoomArmFov;
				ViewTargetCamera->DesiredCamera.Fov += FovZoomDelta;
				ViewTargetCamera->DesiredCamera.Fov = FMath::Clamp(ViewTargetCamera->DesiredCamera.Fov, MinFov, MaxFov);
				ViewTargetCamera->bCameraFov_HasModified = true;
			}
		}
		else
		{
			// 处理没有输入情况下的 ArmLength 淡入情况
			if (FMath::Abs(DesiredZoomArmLength - CurrentZoomArmLength) < UE_SMALL_NUMBER)
			{
				// 回弹完毕
				ViewTargetCamera->DesiredCamera.ArmLength += (DesiredZoomArmLength - CurrentZoomArmLength);
				ViewTargetCamera->DesiredCamera.ArmLength =
					FMath::Clamp(ViewTargetCamera->DesiredCamera.ArmLength, MinArmLength, MaxArmLength);
				ViewTargetCamera->bCameraArmLength_HasModified = true;

				// 清理状态
				bArmLengthZooming = false;
				CurrentZoomArmLength = 0.f;
				DesiredZoomArmLength = 0.f;
			}
			else
			{
				const float OldZoomArmLength = CurrentZoomArmLength;
				CurrentZoomArmLength =
					FMath::FInterpTo(CurrentZoomArmLength, DesiredZoomArmLength, DeltaSeconds, ArmZoomLagSpeed);
				const float ArmLengthZoomDelta = CurrentZoomArmLength - OldZoomArmLength;
				ViewTargetCamera->DesiredCamera.ArmLength += ArmLengthZoomDelta;
				ViewTargetCamera->DesiredCamera.ArmLength =
					FMath::Clamp(ViewTargetCamera->DesiredCamera.ArmLength, MinArmLength, MaxArmLength);
				ViewTargetCamera->bCameraArmLength_HasModified = true;
			}
		}
	}
}

void UJoyCameraInputController::AddYawInput(float Val)
{
	YawInputValue = Val;
}

void UJoyCameraInputController::AddPitchInput(float Val)
{
	bPitchInput = true;
	PitchInputValue = Val;
}

void UJoyCameraInputController::AddDeviceArmLengthInput(float Val)
{
	bArmLengthInput = true;
	ArmZoomValue = Val;
}

void UJoyCameraInputController::ResetDeviceInput()
{
	bPitchInput = false;
	PitchInputValue = 0.f;

	YawInputValue = 0.f;

	bArmLengthInput = false;
	ArmZoomValue = 0.f;
}

void UJoyCameraInputController::LockArmLength()
{
	LockArmLengthStack++;
}

void UJoyCameraInputController::LockArmRotationPitch()
{
	LockArmRotationPitchStack++;
}

void UJoyCameraInputController::LockArmRotationYaw()
{
	LockArmRotationYawStack++;
}

void UJoyCameraInputController::UnlockArmLength()
{
	LockArmLengthStack--;
}

void UJoyCameraInputController::UnlockArmRotationPitch()
{
	LockArmRotationPitchStack--;
}

bool UJoyCameraInputController::IsArmRotationLocked() const
{
	return LockArmRotationPitchStack > 0 || LockArmRotationYawStack > 0;
}

void UJoyCameraInputController::UnlockArmRotationYaw()
{
	LockArmRotationYawStack--;
}

void UJoyCameraInputController::SetConfigs(const TMap<EJoyCameraInput, float>& Config)
{
	UPDATE_INPUT_CONFIGS(ArmZoomSpeed);
	UPDATE_INPUT_CONFIGS(RotationInputSmoothFactor);
	UPDATE_INPUT_CONFIGS(SensitivityYaw);
	UPDATE_INPUT_CONFIGS(SensitivityPitch);
	UPDATE_INPUT_CONFIGS(MinRotationInputSpeed);
	UPDATE_INPUT_CONFIGS(MaxRotationInputSpeed);
	UPDATE_INPUT_CONFIGS(ArmZoomLagSpeed);
	UPDATE_INPUT_CONFIGS(FovZoomSpeed);
	UPDATE_INPUT_CONFIGS(FovOnHitFace);
}
