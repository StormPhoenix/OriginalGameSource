#include "JoyCameraModifierController.h"

#include "Camera/JoyCameraComponent.h"
#include "Camera/JoyPlayerCameraManager.h"
#include "Character/JoyCharacter.h"
#include "Gameplay/JoyCharacterControlManageSubsystem.h"
#include "Gameplay/TimeDilation/JoyTimeDilationManageSubsystem.h"
#include "JoyGameBlueprintLibrary.h"
#include "JoyLogChannels.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/JoyPlayerController.h"
#include "Utils/JoyMathBlueprintLibrary.h"

constexpr int32 GModify_Small_Length = 1;

bool FloatInterpTo(float Current, float Target, float DeltaTime, float Speed, float& InterValue)
{
	InterValue = FMath::FInterpTo(Current, Target, DeltaTime, Speed);
	if (FMath::Abs(InterValue - Target) < GModify_Small_Length)
	{
		return false;
	}
	return true;
}

bool RotatorInterpTo(FRotator Current, FRotator Target, float DeltaTime, float Speed, FRotator& InterValue)
{
	InterValue = FRotator(FMath::QInterpTo(FQuat(Current), FQuat(Target), DeltaTime, Speed));
	if (InterValue.Equals(Target, GModify_Small_Length))
	{
		return false;
	}

	return true;
}

void PreProcessCameraModifiers(FCameraModifiers& Modifiers, const UJoyCameraModifierController* ModifierController)
{
	// 对 Modifiers 中的一些参数依据当前允许状态做修正
	if (ModifierController)
	{
		const AActor* ModifyTarget = ModifierController->GetModifyTarget();
		const AJoyPlayerCameraManager* PlayerCameraManager = ModifierController->GetPlayerCameraManager();
		if (ModifyTarget && PlayerCameraManager)
		{
			const FRotator& ActorRotation = ModifyTarget->GetActorRotation();
			const FMinimalViewInfo& CacheView = PlayerCameraManager->GetCameraCacheView();
			const FRotator LocalRotator =
				UKismetMathLibrary::ComposeRotators(CacheView.Rotation, ActorRotation.GetInverse());

			if (Modifiers.LocalRotationSettings.bModifyYaw &&
				Modifiers.LocalRotationSettings.RelativeYawAdaptiveOption.bAdaptiveOption)
			{
				const float AbsoluteYaw = FMath::Abs(Modifiers.LocalRotationSettings.Yaw);
				if (LocalRotator.Yaw >= 0)
				{
					// 镜头位于角色左侧
					Modifiers.LocalRotationSettings.Yaw =
						Modifiers.LocalRotationSettings.RelativeYawAdaptiveOption.bSync ? AbsoluteYaw : -AbsoluteYaw;
				}
				else
				{
					// 镜头位于角色右侧
					Modifiers.LocalRotationSettings.Yaw =
						Modifiers.LocalRotationSettings.RelativeYawAdaptiveOption.bSync ? -AbsoluteYaw : AbsoluteYaw;
				}
			}

			if (Modifiers.LocalRotationSettings.bModifyRoll &&
				Modifiers.LocalRotationSettings.RelativeRollAdaptiveOption.bAdaptiveOption)
			{
				const float AbsoluteRoll = FMath::Abs(Modifiers.LocalRotationSettings.Roll);
				if (LocalRotator.Yaw >= 0)
				{
					// 镜头位于角色左侧
					Modifiers.LocalRotationSettings.Roll =
						Modifiers.LocalRotationSettings.RelativeRollAdaptiveOption.bSync ? -AbsoluteRoll : AbsoluteRoll;
				}
				else
				{
					// 镜头位于角色右侧
					Modifiers.LocalRotationSettings.Roll =
						Modifiers.LocalRotationSettings.RelativeRollAdaptiveOption.bSync ? AbsoluteRoll : -AbsoluteRoll;
				}
			}

			if (Modifiers.LocalOffsetSettings.bModified &&
				Modifiers.LocalOffsetSettings.LocalArmOffsetYAdaptiveOption.bAdaptiveOption)
			{
				const float AbsoluteLocalY = FMath::Abs(Modifiers.LocalOffsetSettings.LocalArmOffset.Y);
				if (LocalRotator.Yaw >= 0)
				{
					// 镜头位于角色左侧
					Modifiers.LocalOffsetSettings.LocalArmOffset.Y =
						Modifiers.LocalOffsetSettings.LocalArmOffsetYAdaptiveOption.bSync ? AbsoluteLocalY
																						  : -AbsoluteLocalY;
				}
				else
				{
					// 镜头位于角色右侧
					Modifiers.LocalOffsetSettings.LocalArmOffset.Y =
						Modifiers.LocalOffsetSettings.LocalArmOffsetYAdaptiveOption.bSync ? -AbsoluteLocalY
																						  : AbsoluteLocalY;
				}
			}
		}
	}
}

UJoyCameraModifierController::UJoyCameraModifierController()
{
}

void UJoyCameraModifierController::SetModifyTarget(AActor* ModifyTarget)
{
	ModifiedViewTarget = ModifyTarget;
}

bool UJoyCameraModifierController::IsModifiedAndNeedUpdate() const
{
	if (bIsModified)
	{
		if (!bNeedManualBreakModify)
		{
			// 无需手动停止
			return true;
		}
		else
		{
			// 需要手动停止
			if (CurrentBlendState != EBlendState::Loop || bHasManualBreakModify == true)
			{
				return true;
			}
		}
	}
	else if (bNeedModifyFadeOut)
	{
		// 已经修改完毕，且处于 fade out 状态
		return true;
	}

	return false;
}

void UJoyCameraModifierController::UpdateInternal(float InDeltaSeconds)
{
	if (PCM == nullptr)
	{
		return;
	}

	const float DeltaTime = bIgnoreTimeDilation ? PCM->DeltaTimeThisFrame_IgnoreTimeDilation : PCM->DeltaTimeThisFrame;
	Super::UpdateInternal(DeltaTime);

	if (ModifiedViewTarget.Get() == nullptr)
	{
		return;
	}

	if (bIsModified)
	{
		// 当正在修改相机镜头时，需要更新依赖的外部相机数据
		ExternalDependencyCameraData.UpdateCameraData(ModifiedViewTarget->GetActorRotation().Quaternion());
	}

	if (PCM == nullptr)
	{
		return;
	}

	// 修改相机参数
	UpdateModifiers(DeltaTime);

	// 由于镜头被打断后，一部分参数可能尚未复原，所以需要在此处复原参数
	UpdateModifyFadeOut(DeltaTime);
}

void UJoyCameraModifierController::UpdateModifyFadeOut(float DeltaSeconds)
{
	if (PCM == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("UpdateModifyFadeOut: PCM is null"));
		return;
	}

	if (ModifiedViewTarget.Get() == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("UpdateModifyFadeOut: ModifiedViewTarget is null"));
		return;
	}

	if (!bNeedModifyFadeOut)
	{
		return;
	}

	FViewTargetCameraInfo& TargetCameraInfo = PCM->MultiViewTargetCameraManager[ModifiedViewTarget.Get()];
	if (ModifyFadeOutData.bModifyArmLength)
	{
		// 如果修改了弹簧臂
		if (TargetCameraInfo.bCameraArmLength_HasModified)
		{
			// 当 ArmLength 又被修改时，则不再进行 FadeOut 处理
			ModifyFadeOutData.bModifyArmLength = false;
		}
		else
		{
			ModifyFadeOutData.bModifyArmLength =
				FloatInterpTo(TargetCameraInfo.CurrentCamera.ArmLength, ModifyFadeOutData.ArmLength, DeltaSeconds,
					ModifyArmLengthLagSpeed, TargetCameraInfo.DesiredCamera.ArmLength);
			TargetCameraInfo.bCameraArmLength_HasModified = true;
		}
	}

	/*
	if (ModifyFadeOutData.bModifyArmRotation)
	{
		if (TargetCameraInfo.bArmYaw_HasModified || TargetCameraInfo.bArmPitch_HasModified)
		{
			// 当 ArmRotation 又被修改时，则不再进行 FadeOut 处理
			ModifyFadeOutData.bModifyArmRotation = false;
		}
		else
		{
			ModifyFadeOutData.bModifyArmRotation =
				RotatorInterpTo(TargetCameraInfo.CurrentCamera.ArmCenterRotation, ModifyFadeOutData.ArmRotation,
					DeltaSeconds, ModifyArmRotationLagSpeed, TargetCameraInfo.DesiredCamera.ArmCenterRotation);

			TargetCameraInfo.bArmYaw_HasModified = true;
			TargetCameraInfo.bArmPitch_HasModified = true;
		}
	}
	*/

	if (ModifyFadeOutData.bModifyArmPitch)
	{
		if (TargetCameraInfo.bArmPitch_HasModified)
		{
			// 当 ArmRotation 又被修改时，则不再进行 FadeOut 处理
			ModifyFadeOutData.bModifyArmPitch = false;
		}
		else
		{
			float ArmPitch{0.f};
			ModifyFadeOutData.bModifyArmPitch = FloatInterpTo(TargetCameraInfo.CurrentCamera.ArmCenterRotation.Pitch,
				ModifyFadeOutData.ArmRotation.Pitch, DeltaSeconds, ModifyArmRotationLagSpeed, ArmPitch);
			TargetCameraInfo.DesiredCamera.ArmCenterRotation.Pitch = ArmPitch;
			TargetCameraInfo.bArmPitch_HasModified = true;
		}
	}

	if (ModifyFadeOutData.bModifyArmYaw)
	{
		if (TargetCameraInfo.bArmYaw_HasModified)
		{
			// 当 ArmRotation 又被修改时，则不再进行 FadeOut 处理
			ModifyFadeOutData.bModifyArmYaw = false;
		}
		else
		{
			float ArmYaw{0.f};
			ModifyFadeOutData.bModifyArmYaw = FloatInterpTo(TargetCameraInfo.CurrentCamera.ArmCenterRotation.Yaw,
				ModifyFadeOutData.ArmRotation.Yaw, DeltaSeconds, ModifyArmRotationLagSpeed, ArmYaw);
			TargetCameraInfo.DesiredCamera.ArmCenterRotation.Yaw = ArmYaw;
			TargetCameraInfo.bArmYaw_HasModified = true;
		}
	}

	if (ModifyFadeOutData.bModifyArmRoll)
	{
		if (TargetCameraInfo.bArmRoll_HasModified)
		{
			// 当 ArmRotation 又被修改时，则不再进行 FadeOut 处理
			ModifyFadeOutData.bModifyArmRoll = false;
		}
		else
		{
			float ArmRoll{0.f};
			ModifyFadeOutData.bModifyArmRoll = FloatInterpTo(TargetCameraInfo.CurrentCamera.ArmCenterRotation.Roll,
				ModifyFadeOutData.ArmRotation.Roll, DeltaSeconds, ModifyArmRotationLagSpeed, ArmRoll);
			TargetCameraInfo.DesiredCamera.ArmCenterRotation.Roll = ArmRoll;
			TargetCameraInfo.bArmRoll_HasModified = true;
		}
	}

	if (ModifyFadeOutData.bModifyFov)
	{
		if (TargetCameraInfo.bCameraFov_HasModified)
		{
			ModifyFadeOutData.bModifyFov = false;
		}
		else
		{
			ModifyFadeOutData.bModifyFov = FloatInterpTo(TargetCameraInfo.CurrentCamera.Fov, ModifyFadeOutData.Fov,
				DeltaSeconds, ModifyArmLengthLagSpeed, TargetCameraInfo.DesiredCamera.Fov);
			TargetCameraInfo.bCameraFov_HasModified = true;
		}
	}

	bNeedModifyFadeOut = ModifyFadeOutData.bModifyArmLength || ModifyFadeOutData.bModifyArmOffset ||
						 ModifyFadeOutData.bModifyArmPitch || ModifyFadeOutData.bModifyArmYaw ||
						 ModifyFadeOutData.bModifyArmRoll || ModifyFadeOutData.bModifyFov;
}

void UJoyCameraModifierController::UpdateModifiers(float DeltaSeconds)
{
	if (!bIsModified)
	{
		return;
	}

	ModifyElapsedTime += DeltaSeconds;

	// 更新淡入淡出状态，以及插值数值
	EBlendState BlendState = EBlendState::None;
	float BlendAlpha = 0;

	if (ModifyElapsedTime < ModifyBlendInTime)
	{
		// 淡入
		BlendState = EBlendState::BlendIn;

		CurrentRawBlendAlpha = ModifyBlendInTime > 0 ? this->ModifyElapsedTime / ModifyBlendInTime : 1;
		BlendAlpha = CurrentRawBlendAlpha;
	}
	else if (!bNeedManualBreakModify && ModifyElapsedTime < ModifyBlendInTime + ModifyDuration)
	{
		/** 无需手动中断，且还未到持续时长，则保持值不变，设置为 Loop */
		BlendState = EBlendState::Loop;
	}
	else if (bNeedManualBreakModify && !bHasManualBreakModify)
	{
		/** 需要手动中断修改过程，但尚未中断，则保持值不变，设置为 Loop */
		BlendState = EBlendState::Loop;
	}
	else
	{
		// 淡出
		BlendState = EBlendState::BlendOut;
		if (!bNeedManualBreakModify)
		{
			/** 如果是到达时间限制进入淡出，则统计 BlendIn + BlendDuration + BlendOut 阶段经过时间 */
			CurrentRawBlendAlpha = ModifyBlendOutTime > 0
									   ? (ModifyElapsedTime - ModifyBlendInTime - ModifyDuration) / ModifyBlendOutTime
									   : 1;
			BlendAlpha = CurrentRawBlendAlpha;
		}
		else
		{
			/** 如果是手动中断导致进入淡出，则统计 BlendOut 阶段经过时间 */
			ModifyBlendOutElapsedTime += DeltaSeconds;
			CurrentRawBlendAlpha = ModifyBlendOutTime > 0 ? ModifyBlendOutElapsedTime / ModifyBlendOutTime : 1;
			BlendAlpha = CurrentRawBlendAlpha;
		}
	}

	CurrentRawBlendAlpha = FMath::Clamp(CurrentRawBlendAlpha, 0, 1.);
	BlendAlpha = FMath::Clamp(BlendAlpha, 0, 1.);
	CurrentBlendState = BlendState;

	// 更新相机臂最终臂长
	UpdateArmLengthModifier(BlendState, BlendAlpha);
	// 更新角色坐标系下的相机臂中心偏移
	UpdateLocalArmCenterOffsetModifier(BlendState, BlendAlpha);
	// 更新世界坐标系下面的相机臂中心偏移
	UpdateWorldArmCenterOffsetModifier(BlendState, BlendAlpha);
	// 更新相机臂旋转
	UpdateArmRotationModifier(BlendState, BlendAlpha);
	UpdateFovModifier(BlendState, BlendAlpha);

	if (!bNeedManualBreakModify)
	{
		if (ModifyElapsedTime > ModifyBlendInTime + ModifyDuration + ModifyBlendOutTime)
		{
			EndModify();
			ResetViewTarget();
		}
	}
	else if (ModifyBlendOutElapsedTime >= ModifyBlendOutTime)
	{
		// 已经手动中断了修改过程，则结束修改
		EndModify();
		ResetViewTarget();
	}
}

void UJoyCameraModifierController::ApplyCameraModify_Immediately(const FCameraModifiers& InCameraModifiers)
{
	float TotalTime = 2;
	if (const auto* TimeSys = UJoyTimeDilationManageSubsystem::Get(GetWorld()))
	{
		const float Dilation = TimeSys->GetGlobalTimeDilation();
		TotalTime = TotalTime * (1.0 / Dilation);
	}

	ApplyCameraModify(TotalTime, 0, 0, InCameraModifiers);

	constexpr float ElapsedTime = 1.;
	UpdateInternal(ElapsedTime);
	if (PCM != nullptr)
	{
		// 强制将 DesiredCamera 同步到 CurrentCamera
		FViewTargetCameraInfo& TargetCameraInfo = PCM->MultiViewTargetCameraManager[ModifiedViewTarget.Get()];
		TargetCameraInfo.CurrentCamera.CopyCamera(TargetCameraInfo.DesiredCamera);
		PCM->UpdateActorTransform(1.);
	}
	EndModify();
	ResetViewTarget();
}

FCameraModifyHandle UJoyCameraModifierController::GetLastModifierHandle() const
{
	return LastModifierHandle;
}

FCameraModifyHandle UJoyCameraModifierController::ApplyCameraModify(float Duration, float BlendInTime,
	float BlendOutTime, FCameraModifiers const& InCameraModifiers, bool bNeedManualBreak)
{
	if (PCM == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("ApplyCameraModify: PCM is null"));
		return FCameraModifyHandle(0);
	}

	if (ModifiedViewTarget.Get() == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("ApplyCameraModify: ModifiedViewTarget is null"));
		return FCameraModifyHandle(0);
	}

	// 如果存在状态混合，则终止
	EndModify();
	CurrentCameraModifySpec.Clear();

	FViewTargetCameraInfo& TargetCameraInfo = PCM->MultiViewTargetCameraManager[ModifiedViewTarget.Get()];
	TargetCameraInfo.LastCamera.CopyCamera(TargetCameraInfo.CurrentCamera);
	TargetCameraInfo.RestoreCamera.CopyCamera(TargetCameraInfo.CurrentCamera);
	MakeRestoreCameraData(TargetCameraInfo.RestoreCamera);

	ModifyDuration = Duration >= 0 ? Duration : 0;
	ModifyBlendInTime = BlendInTime;
	ModifyBlendOutTime = BlendOutTime;
	ModifyElapsedTime = 0.;
	CurrentCameraModifySpec.CameraModifiers = InCameraModifiers;
	PreProcessCameraModifiers(CurrentCameraModifySpec.CameraModifiers, this);

	bNeedManualBreakModify = bNeedManualBreak;
	bHasManualBreakModify = false;

	bNeedModifyAdditionalArmLength =
		!FMath::IsNearlyEqual(CurrentCameraModifySpec.CameraModifiers.ArmLengthSettings.ArmLengthAdditional, 0);
	bNeedModifyAdditionalLocalCameraOffset =
		!CurrentCameraModifySpec.CameraModifiers.LocalOffsetSettings.LocalArmOffsetAdditional.IsNearlyZero();
	bNeedModifyAdditionalCameraRotation =
		!CurrentCameraModifySpec.CameraModifiers.WorldRotationSettings.ArmRotationAdditional.IsNearlyZero();

	bIsModified = true;
	bResetViewTarget = CurrentCameraModifySpec.CameraModifiers.bResetViewTarget;
	TimeToResetViewTarget = CurrentCameraModifySpec.CameraModifiers.TimeToResetViewTarget;
	bNeedModifyFadeOut = false;
	bIgnoreTimeDilation = CurrentCameraModifySpec.CameraModifiers.bIgnoreTimeDilation;

	if (CurrentCameraModifySpec.CameraModifiers.bOverrideCameraInput)
	{
		if (IsCameraRotationModified())
		{
			// 锁定镜头旋转输入
			PCM->SetArmPitchInputEnabled(false);
			PCM->SetArmYawInputEnabled(false);
		}

		if (bNeedModifyAdditionalArmLength || CurrentCameraModifySpec.CameraModifiers.ArmLengthSettings.bModified)
		{
			// 锁定相机臂长修改
			PCM->SetArmLengthInputEnabled(false);
		}

		// 锁定角色切换
		if (auto* CharacterControlManager = UJoyCharacterControlManageSubsystem::Get(GetWorld()))
		{
			CharacterControlManager->SetCharacterSwitchEnabled(false);
		}
	}

	bNeedModifyFov = CurrentCameraModifySpec.CameraModifiers.CameraFovSettings.bModified;

	SequenceNumber++;
	LastModifierHandle = FCameraModifyHandle(SequenceNumber);

	return LastModifierHandle;
}

bool UJoyCameraModifierController::IsCameraOffsetModified() const
{
	return bNeedModifyAdditionalLocalCameraOffset ||
		   CurrentCameraModifySpec.CameraModifiers.WorldOffsetAdditionalSettings.bModified ||
		   CurrentCameraModifySpec.CameraModifiers.LocalOffsetSettings.bModified;
}

bool UJoyCameraModifierController::IsCameraRotationModified() const
{
	return bNeedModifyAdditionalCameraRotation ||
		   CurrentCameraModifySpec.CameraModifiers.WorldRotationSettings.bModified ||
		   CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyPitch ||
		   CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyYaw ||
		   CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyRoll;
}

void UJoyCameraModifierController::StartModifyFadeOut()
{
	// 检查是否需要复位 ArmLength
	ModifyFadeOutData.bModifyArmLength =
		CurrentCameraModifySpec.CameraModifiers.ArmLengthSettings.bModified || bNeedModifyAdditionalArmLength;
	if (ModifyFadeOutData.bModifyArmLength)
	{
		ModifyFadeOutData.ArmLength = GetFinalArmLength();
	}

	// 检查是否需要复位 ArmRotation
	ModifyFadeOutData.bModifyArmPitch = CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyPitch ||
										CurrentCameraModifySpec.CameraModifiers.WorldRotationSettings.bModified ||
										bNeedModifyAdditionalCameraRotation;

	ModifyFadeOutData.bModifyArmYaw = CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyYaw ||
									  CurrentCameraModifySpec.CameraModifiers.WorldRotationSettings.bModified ||
									  bNeedModifyAdditionalCameraRotation;

	ModifyFadeOutData.bModifyArmRoll = CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyRoll ||
									   CurrentCameraModifySpec.CameraModifiers.WorldRotationSettings.bModified ||
									   bNeedModifyAdditionalCameraRotation;

	if (ModifyFadeOutData.bModifyArmPitch || ModifyFadeOutData.bModifyArmYaw || ModifyFadeOutData.bModifyArmRoll)
	{
		ModifyFadeOutData.ArmRotation = GetFinalArmRotation();
	}

	// 检查是否需要复位 Fov
	ModifyFadeOutData.bModifyFov = bNeedModifyFov;
	if (ModifyFadeOutData.bModifyFov)
	{
		ModifyFadeOutData.Fov = PCM->GetBaseCameraFov();
	}

	ModifyFadeOutData.bModifyArmOffset =
		CurrentCameraModifySpec.CameraModifiers.WorldOffsetAdditionalSettings.bModified ||
		bNeedModifyAdditionalLocalCameraOffset;
	if (ModifyFadeOutData.bModifyArmOffset)
	{
		// 恢复世界坐标位移
		ModifyFadeOutData.WorldCameraOffsetAdditional = GetFinalWorldArmOffset();
	}

	bNeedModifyFadeOut = ModifyFadeOutData.bModifyArmLength | ModifyFadeOutData.bModifyArmPitch |
						 ModifyFadeOutData.bModifyArmYaw | ModifyFadeOutData.bModifyArmRoll |
						 ModifyFadeOutData.bModifyFov;
}

void UJoyCameraModifierController::MakeRestoreCameraData(FVirtualCamera& RestoreCamera) const
{
	// 不处理 Arm Length 恢复，因为它会默认恢复到基础臂长

	// 处理 Arm Rotation 恢复
	if (ModifyFadeOutData.bModifyArmPitch || ModifyFadeOutData.bModifyArmYaw || ModifyFadeOutData.bModifyArmRoll)
	{
		RestoreCamera.SetArmCenterRotation(ModifyFadeOutData.ArmRotation);
	}

	// 不处理 Arm Fov 恢复，因为它会默认恢复到基础 Fov

	// 不处理 Arm Local Offset 恢复，因为它会默认恢复基础局部偏移

	// 处理 Arm World Offset
	if (ModifyFadeOutData.bModifyArmOffset)
	{
		RestoreCamera.WorldArmOffsetAdditional = ModifyFadeOutData.WorldCameraOffsetAdditional;
	}
}

void UJoyCameraModifierController::EndModify()
{
	if (!bIsModified)
	{
		return;
	}

	// 重新开启输入屏蔽
	if (CurrentCameraModifySpec.CameraModifiers.bOverrideCameraInput)
	{
		// 恢复输入输出的禁用
		if (IsCameraRotationModified())
		{
			PCM->SetArmPitchInputEnabled(true);
			PCM->SetArmYawInputEnabled(true);
		}

		if (bNeedModifyAdditionalArmLength || CurrentCameraModifySpec.CameraModifiers.ArmLengthSettings.bModified)
		{
			PCM->SetArmLengthInputEnabled(true);
		}

		// 恢复角色切换
		if (auto* CharacterControlManager = UJoyCharacterControlManageSubsystem::Get(GetWorld()))
		{
			CharacterControlManager->SetCharacterSwitchEnabled(true);
		}
	}

	StartModifyFadeOut();

	bNeedModifyAdditionalArmLength = false;
	bNeedModifyAdditionalLocalCameraOffset = false;

	if (/* CurrentCameraModifySpec.CameraModifiers.bIsModifiedArmRotation || */ bNeedModifyAdditionalCameraRotation)
	{
		bNeedModifyAdditionalCameraRotation = false;
	}

	bNeedModifyFov = false;

	ModifyBlendOutElapsedTime = 0;
	bNeedManualBreakModify = false;
	bHasManualBreakModify = false;

	bIsModified = false;
	bIgnoreTimeDilation = true;

	// 清空依赖数据
	ExternalDependencyCameraData.Clean();

	if (PCM)
	{
		PCM->OnCameraModifyEndDelegate.Broadcast(ModifiedViewTarget, LastModifierHandle);
	}
}

void UJoyCameraModifierController::BreakModifier(FCameraModifyHandle ModifyHandler)
{
	if (!ModifyHandler.IsValid() || ModifyHandler == LastModifierHandle)
	{
		if (bNeedManualBreakModify && !bHasManualBreakModify)
		{
			bHasManualBreakModify = true;
		}
	}
}

void UJoyCameraModifierController::UpdateFovModifier(EBlendState State, float Alpha)
{
	if (PCM == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("UpdateFovModifier: PCM is null"));
		return;
	}

	if (ModifiedViewTarget.Get() == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("UpdateFovModifier: ModifiedViewTarget is null"));
		return;
	}

	if (!bNeedModifyFov)
	{
		return;
	}

	FViewTargetCameraInfo& TargetCameraInfo = PCM->MultiViewTargetCameraManager[ModifiedViewTarget.Get()];

	if (CurrentCameraModifySpec.CameraModifiers.CameraFovSettings.bCameraFovCurveControl &&
		CurrentCameraModifySpec.CameraModifiers.CameraFovSettings.CameraFovCurve != nullptr)
	{
		if (State == EBlendState::BlendIn)
		{
			Alpha = UJoyMathBlueprintLibrary::GetNormalizeCurveFloat(
				CurrentCameraModifySpec.CameraModifiers.CameraFovSettings.CameraFovCurve, CurrentRawBlendAlpha);
		}
		else if (State == EBlendState::BlendOut)
		{
			Alpha = 1.0 - UJoyMathBlueprintLibrary::GetNormalizeCurveFloat(
							  CurrentCameraModifySpec.CameraModifiers.CameraFovSettings.CameraFovCurve,
							  1.0 - CurrentRawBlendAlpha);
		}
	}

	const float TargetFov = CurrentCameraModifySpec.CameraModifiers.CameraFovSettings.CameraFov;
	switch (State)
	{
		case EBlendState::BlendIn:
			TargetCameraInfo.DesiredCamera.Fov = FMath::Lerp(TargetCameraInfo.LastCamera.Fov, TargetFov, Alpha);
			break;
		case EBlendState::Loop:
			TargetCameraInfo.DesiredCamera.Fov = TargetFov;
			break;
		case EBlendState::BlendOut:
			// TargetCameraInfo.DesiredCamera.Fov = FMath::Lerp(TargetFov, PCM->GetBaseCameraFov(), Alpha);
			TargetCameraInfo.DesiredCamera.Fov = FMath::Lerp(TargetFov, GetFinalFov(), Alpha);
			break;
		default:
			break;
	}

	TargetCameraInfo.bCameraFov_HasModified = true;
}

void UJoyCameraModifierController::UpdateArmRotationModifier(EBlendState State, float Alpha)
{
	if (PCM == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("UpdateArmRotationModifier: PCM is null"));
		return;
	}

	if (ModifiedViewTarget.Get() == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("UpdateArmRotationModifier: ModifiedViewTarget is null"));
		return;
	}

	FViewTargetCameraInfo& TargetCameraInfo = PCM->MultiViewTargetCameraManager[ModifiedViewTarget.Get()];
	if (TargetCameraInfo.bArmYaw_HasModified || TargetCameraInfo.bArmPitch_HasModified ||
		TargetCameraInfo.bArmRoll_HasModified)
	{
		CurrentCameraModifySpec.bArmRotationModifyInterrupted = true;
	}

	if (CurrentCameraModifySpec.bArmRotationModifyInterrupted)
	{
		return;
	}

	FRotator TargetArmRotator = FRotator::ZeroRotator;
	bool bArmRotatorModified = true;
	// 判断有没有修改相对 (Pitch, Yaw, Roll)
	if (CurrentCameraModifySpec.CameraModifiers.WorldRotationSettings.bModified)
	{
		// 修改相机臂旋转（世界坐标系）
		TargetArmRotator = CurrentCameraModifySpec.CameraModifiers.WorldRotationSettings.ArmRotation;
		TargetCameraInfo.bArmYaw_HasModified = true;
		TargetCameraInfo.bArmPitch_HasModified = true;
		TargetCameraInfo.bArmRoll_HasModified = true;

		if (CurrentCameraModifySpec.CameraModifiers.bArmRotationCurveControl &&
			CurrentCameraModifySpec.CameraModifiers.ArmRotationCurve != nullptr)
		{
			if (State == EBlendState::BlendIn)
			{
				Alpha = UJoyMathBlueprintLibrary::GetNormalizeCurveFloat(
					CurrentCameraModifySpec.CameraModifiers.ArmRotationCurve, CurrentRawBlendAlpha);
			}
			else if (State == EBlendState::BlendOut)
			{
				Alpha = 1.0 - UJoyMathBlueprintLibrary::GetNormalizeCurveFloat(
								  CurrentCameraModifySpec.CameraModifiers.ArmRotationCurve, 1.0 - CurrentRawBlendAlpha);
			}
		}
	}
	else if (CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyPitch ||
			 CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyYaw ||
			 CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyRoll)
	{
		/** 修改相机臂旋转（角色局部坐标系） */
		if (State == EBlendState::BlendOut || (PCM != nullptr && PCM->bMoveInput))
		{
			// 进入 Blend Out 状态时，锁定依赖的角色朝向数据
			ExternalDependencyCameraData.LockPawnFaceViewQuat();
		}

		const FQuat PawnFaceQuat =
			ExternalDependencyCameraData.IsValid()
				? ExternalDependencyCameraData.GetPawnFaceViewQuat()
				: (ModifiedViewTarget.Get() != nullptr ? ModifiedViewTarget->GetActorRotation().Quaternion()
													   : FRotator::ZeroRotator.Quaternion());

		const FQuat AdderQuat = FRotator(CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyPitch
											 ? CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.Pitch
											 : 0,
			CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyYaw
				? CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.Yaw
				: 0,
			CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyRoll
				? CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.Roll
				: 0)
									.Quaternion();
		TargetArmRotator = (PawnFaceQuat * AdderQuat).Rotator();

		if (!CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyPitch)
		{
			TargetArmRotator.Pitch = TargetCameraInfo.LastCamera.ArmCenterRotation.Pitch;
		}

		if (!CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyYaw)
		{
			TargetArmRotator.Yaw = TargetCameraInfo.LastCamera.ArmCenterRotation.Yaw;
		}

		if (!CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyRoll)
		{
			TargetArmRotator.Roll = TargetCameraInfo.LastCamera.ArmCenterRotation.Roll;
		}

		TargetCameraInfo.bArmPitch_HasModified |=
			CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyPitch;
		TargetCameraInfo.bArmYaw_HasModified |=
			CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyYaw;
		TargetCameraInfo.bArmRoll_HasModified |=
			CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyRoll;
	}
	else
	{
		// 不做改变
		bArmRotatorModified = false;
		TargetArmRotator = TargetCameraInfo.LastCamera.ArmCenterRotation;
	}

	// 额外 Rotation
	if (bNeedModifyAdditionalCameraRotation)
	{
		// 在当前相机臂旋转的基础上，做旋转递增修改
		TargetArmRotator += CurrentCameraModifySpec.CameraModifiers.WorldRotationSettings.ArmRotationAdditional;

		TargetCameraInfo.bArmPitch_HasModified |=
			(CurrentCameraModifySpec.CameraModifiers.WorldRotationSettings.ArmRotationAdditional.Pitch != 0);
		TargetCameraInfo.bArmYaw_HasModified |=
			(CurrentCameraModifySpec.CameraModifiers.WorldRotationSettings.ArmRotationAdditional.Yaw != 0);
		TargetCameraInfo.bArmRoll_HasModified |=
			(CurrentCameraModifySpec.CameraModifiers.WorldRotationSettings.ArmRotationAdditional.Roll != 0);
	}
	else if (!bArmRotatorModified)
	{
		// 完全没有修改，则直接返回
		return;
	}

	// 初始化为目标位置
	TargetCameraInfo.DesiredCamera.ArmCenterRotation = TargetArmRotator;
	switch (State)
	{
		case EBlendState::BlendIn:
			TargetCameraInfo.DesiredCamera.ArmCenterRotation = FQuat::Slerp(
				TargetCameraInfo.LastCamera.ArmCenterRotation.Quaternion(), TargetArmRotator.Quaternion(), Alpha)
																   .Rotator();
			break;
		case EBlendState::Loop:
			TargetCameraInfo.DesiredCamera.ArmCenterRotation = TargetArmRotator;
			break;
		case EBlendState::BlendOut:
			TargetCameraInfo.DesiredCamera.ArmCenterRotation =
				FQuat::Slerp(TargetArmRotator.Quaternion(), GetFinalArmRotation().Quaternion(), Alpha).Rotator();
			break;
		default:
			break;
	}
}

void UJoyCameraModifierController::UpdateLocalArmCenterOffsetModifier(EBlendState State, float Alpha) const
{
	if (PCM == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("UpdateCameraOffsetModifier: PCM is null"));
		return;
	}

	if (ModifiedViewTarget.Get() == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("UpdateCameraOffsetModifier: ModifiedViewTarget is null"));
		return;
	}

	if ((!CurrentCameraModifySpec.CameraModifiers.LocalOffsetSettings.bModified &&
			!bNeedModifyAdditionalLocalCameraOffset))
	{
		return;
	}

	FViewTargetCameraInfo& TargetCameraInfo = PCM->MultiViewTargetCameraManager[ModifiedViewTarget.Get()];
	FVector TargetCameraOffset = FVector::ZeroVector;
	if (CurrentCameraModifySpec.CameraModifiers.LocalOffsetSettings.bModified)
	{
		// 如果用户修改了 CameraOffset，则忽略当前的 CameraOffset 值，使用用户的 CameraOffset
		TargetCameraOffset = CurrentCameraModifySpec.CameraModifiers.LocalOffsetSettings.LocalArmOffset;
	}
	else
	{
		TargetCameraOffset = FVector(PCM->ArmCenterOffsetX, PCM->ArmCenterOffsetY, PCM->ArmCenterOffsetZ);
	}

	// 额外 CameraOffset
	if (bNeedModifyAdditionalLocalCameraOffset)
	{
		TargetCameraOffset += CurrentCameraModifySpec.CameraModifiers.LocalOffsetSettings.LocalArmOffsetAdditional;
	}

	switch (State)
	{
		case EBlendState::BlendIn:
			// 从 LastCamera 状态开始 Blend
			TargetCameraInfo.DesiredCamera.LocalArmCenterOffset =
				FMath::Lerp(TargetCameraInfo.LastCamera.LocalArmCenterOffset, TargetCameraOffset, Alpha);
			break;
		case EBlendState::Loop:
			TargetCameraInfo.DesiredCamera.LocalArmCenterOffset = TargetCameraOffset;
			break;
		case EBlendState::BlendOut:
			/** CameraOffset 默认还原到基础 Offset */
			TargetCameraInfo.DesiredCamera.LocalArmCenterOffset =
				FMath::Lerp(TargetCameraOffset, GetFinalLocalArmOffset(), Alpha);
			break;
		default:
			break;
	}

	TargetCameraInfo.bCameraOffset_HasModified = true;
}

void UJoyCameraModifierController::UpdateWorldArmCenterOffsetModifier(EBlendState State, float Alpha) const
{
	if (PCM == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("UpdateWorldArmCenterOffsetModifier: PCM is null"));
		return;
	}

	if (ModifiedViewTarget.Get() == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("UpdateWorldArmCenterOffsetModifier: ModifiedViewTarget is null"));
		return;
	}

	if (!CurrentCameraModifySpec.CameraModifiers.WorldOffsetAdditionalSettings.bModified)
	{
		return;
	}

	FViewTargetCameraInfo& TargetCameraInfo = PCM->MultiViewTargetCameraManager[ModifiedViewTarget.Get()];
	const FVector TargetWorldArmOffsetAdditional =
		CurrentCameraModifySpec.CameraModifiers.WorldOffsetAdditionalSettings.WorldArmOffsetAdditional;
	switch (State)
	{
		case EBlendState::BlendIn:
			// 从 LastCamera 状态开始 Blend
			TargetCameraInfo.DesiredCamera.WorldArmOffsetAdditional = FMath::Lerp(
				TargetCameraInfo.LastCamera.WorldArmOffsetAdditional, TargetWorldArmOffsetAdditional, Alpha);
			break;
		case EBlendState::Loop:
			TargetCameraInfo.DesiredCamera.WorldArmOffsetAdditional = TargetWorldArmOffsetAdditional;
			break;
		case EBlendState::BlendOut:
			/** World Camera Offset 默认还原到 0 */
			TargetCameraInfo.DesiredCamera.WorldArmOffsetAdditional =
				FMath::Lerp(TargetWorldArmOffsetAdditional, GetFinalWorldArmOffset(), Alpha);
			break;
		default:
			break;
	}

	TargetCameraInfo.bCameraOffset_HasModified = true;
}

void UJoyCameraModifierController::UpdateArmLengthModifier(EBlendState State, float Alpha)
{
	if (PCM == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("UpdateArmLengthModifier: PCM is null"));
		return;
	}

	if (ModifiedViewTarget.Get() == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("UpdateArmLengthModifier: ModifiedViewTarget is null"));
		return;
	}

	if ((!CurrentCameraModifySpec.CameraModifiers.ArmLengthSettings.bModified && !bNeedModifyAdditionalArmLength))
	{
		return;
	}

	FViewTargetCameraInfo& TargetCameraInfo = PCM->MultiViewTargetCameraManager[ModifiedViewTarget.Get()];
	float TargetArmLength = 0;
	if (CurrentCameraModifySpec.CameraModifiers.ArmLengthSettings.bModified)
	{
		// 改动 ArmLength 的基础长度
		TargetArmLength = CurrentCameraModifySpec.CameraModifiers.ArmLengthSettings.ArmLength;
	}
	else
	{
		// 目标基础臂长设置为 Blend 启动之前的基础臂长
		TargetArmLength = TargetCameraInfo.LastCamera.ArmLength;
	}

	if (bNeedModifyAdditionalArmLength)
	{
		// 检查是否要在基础比场上递增/递减
		TargetArmLength += CurrentCameraModifySpec.CameraModifiers.ArmLengthSettings.ArmLengthAdditional;
	}

	if (CurrentCameraModifySpec.CameraModifiers.ArmLengthSettings.bArmLengthCurveControl &&
		CurrentCameraModifySpec.CameraModifiers.ArmLengthSettings.ArmLengthCurve != nullptr)
	{
		if (State == EBlendState::BlendIn)
		{
			Alpha = UJoyMathBlueprintLibrary::GetNormalizeCurveFloat(
				CurrentCameraModifySpec.CameraModifiers.ArmLengthSettings.ArmLengthCurve, CurrentRawBlendAlpha);
		}
		else if (State == EBlendState::BlendOut)
		{
			Alpha = 1.0 - UJoyMathBlueprintLibrary::GetNormalizeCurveFloat(
							  CurrentCameraModifySpec.CameraModifiers.ArmLengthSettings.ArmLengthCurve,
							  1.0 - CurrentRawBlendAlpha);
		}
	}

	TargetArmLength = FMath::Clamp(TargetArmLength, PCM->MinArmLength, PCM->MaxArmLength);
	switch (State)
	{
		case EBlendState::BlendIn:
			// 在 Blend 开始之前的相机基础臂长上进行插值
			TargetCameraInfo.DesiredCamera.ArmLength =
				FMath::Lerp(TargetCameraInfo.LastCamera.ArmLength, TargetArmLength, Alpha);
			break;
		case EBlendState::Loop:
			// 位于 Duration 期间，无需插值
			TargetCameraInfo.DesiredCamera.ArmLength = TargetArmLength;
			break;
		case EBlendState::BlendOut:
			// 在目标臂长和 Blend 完毕之后的相机基础臂长之间进行插值
			TargetCameraInfo.DesiredCamera.ArmLength = FMath::Lerp(TargetArmLength, GetFinalArmLength(), Alpha);
			break;
		default:
			break;
	}

	TargetCameraInfo.bCameraArmLength_HasModified = true;
}

float UJoyCameraModifierController::GetFinalFov() const
{
	if (PCM == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("GetFinalFov: PCM is null"));
		return 90;
	}

	if (CurrentCameraModifySpec.CameraModifiers.CameraFovSettings.bReset)
	{
		return PCM->GetBaseCameraFov();
	}
	else
	{
		const FViewTargetCameraInfo& TargetCameraInfo = PCM->MultiViewTargetCameraManager[ModifiedViewTarget.Get()];
		return TargetCameraInfo.CurrentCamera.Fov;
	}
}

FRotator UJoyCameraModifierController::GetFinalArmRotation()
{
	if (PCM == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("GetFinalArmRotation: PCM is null"));
		return FRotator::ZeroRotator;
	}

	if (ModifiedViewTarget.Get() == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("GetFinalArmRotation: ModifiedViewTarget is null"));
		return FRotator::ZeroRotator;
	}

	FViewTargetCameraInfo& TargetCameraInfo = PCM->MultiViewTargetCameraManager[ModifiedViewTarget.Get()];
	if (CurrentCameraModifySpec.CameraModifiers.WorldRotationSettings.bModified)
	{
		// 修改了世界坐标系旋转，检查是否需要复位
		if (CurrentCameraModifySpec.CameraModifiers.WorldRotationSettings.bReset)
		{
			// 是否要还原相机弹簧臂方向
			return TargetCameraInfo.RestoreCamera.ArmCenterRotation;
		}
	}
	else if (CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyPitch ||
			 CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyYaw ||
			 CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bModifyRoll)
	{
		// 修改了角色坐标系旋转，检查要复位哪一项 (Pitch, Yaw, Roll ?)
		FRotator FinalRotator = TargetCameraInfo.DesiredCamera.ArmCenterRotation;
		if (CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bResetPitch)
		{
			// 复位 Pitch
			FinalRotator.Pitch = TargetCameraInfo.RestoreCamera.ArmCenterRotation.Pitch;
		}

		if (CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bResetYaw)
		{
			// 复位 Yaw
			FinalRotator.Yaw = TargetCameraInfo.RestoreCamera.ArmCenterRotation.Yaw;
		}

		if (CurrentCameraModifySpec.CameraModifiers.LocalRotationSettings.bResetRoll)
		{
			// 复位 Roll，且要复位到 0
			// FinalRotator.Roll = 0;
			FinalRotator.Roll = TargetCameraInfo.RestoreCamera.ArmCenterRotation.Roll;
		}

		return FinalRotator;
	}

	// 不还原相机臂，直接采用当前相机臂方向
	return TargetCameraInfo.DesiredCamera.ArmCenterRotation;
}

FVector UJoyCameraModifierController::GetFinalWorldArmOffset() const
{
	if (PCM == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("GetFinalLocalArmOffset: PCM is null"));
		return FVector::ZeroVector;
	}

	if (ModifiedViewTarget.Get() == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("GetFinalLocalArmOffset: ModifiedViewTarget is null"));
		return FVector::ZeroVector;
	}

	if (CurrentCameraModifySpec.CameraModifiers.WorldOffsetAdditionalSettings.bReset)
	{
		return FVector::ZeroVector;
	}

	// 不还原相机臂中心偏移
	const FViewTargetCameraInfo& TargetCameraInfo = PCM->MultiViewTargetCameraManager[ModifiedViewTarget.Get()];
	return TargetCameraInfo.CurrentCamera.WorldArmOffsetAdditional;
}

FVector UJoyCameraModifierController::GetFinalLocalArmOffset() const
{
	if (PCM == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("GetFinalLocalArmOffset: PCM is null"));
		return FVector::ZeroVector;
	}

	if (ModifiedViewTarget.Get() == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("GetFinalLocalArmOffset: ModifiedViewTarget is null"));
		return FVector::ZeroVector;
	}

	if (CurrentCameraModifySpec.CameraModifiers.LocalOffsetSettings.bReset)
	{
		return PCM->GetBaseLocalArmOffset();
	}

	// 不还原相机偏移
	const FViewTargetCameraInfo& TargetCameraInfo = PCM->MultiViewTargetCameraManager[ModifiedViewTarget.Get()];
	return TargetCameraInfo.CurrentCamera.LocalArmCenterOffset;
}

float UJoyCameraModifierController::GetFinalArmLength() const
{
	if (PCM == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("GetFinalArmLength: PCM is null"));
		return 0;
	}

	if (ModifiedViewTarget.Get() == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("GetFinalArmLength: ModifiedViewTarget is null"));
		return 0;
	}

	if (CurrentCameraModifySpec.CameraModifiers.ArmLengthSettings.bReset)
	{
		// 如果要还原 ArmLength，则还原到基础臂长
		return PCM->GetBaseArmLength();
	}

	// 不还原相机臂长
	const FViewTargetCameraInfo& TargetCameraInfo = PCM->MultiViewTargetCameraManager[ModifiedViewTarget.Get()];
	return TargetCameraInfo.CurrentCamera.ArmLength;
}

void UJoyCameraModifierController::ResetViewTarget()
{
	if (bResetViewTarget && PCM != nullptr)
	{
		if (const auto* ControlManager = UJoyCharacterControlManageSubsystem::Get(GetWorld()))
		{
			const AJoyCharacter* CurrentControlCharacter = ControlManager->GetCurrentControlCharacter();
			const AActor* CurrentViewTarget = PCM->GetViewTarget();
			if (CurrentViewTarget != CurrentControlCharacter)
			{
				// 需要复原 ViewTarget，为了保证复原过程中方向不变，需要重置 Controller 方向
				auto* JoyPlayerController = UJoyGameBlueprintLibrary::GetJoyPlayerController(GetWorld());
				auto* JoyCamera = UJoyCameraComponent::FindCameraComponent(CurrentViewTarget);
				if (JoyPlayerController && JoyCamera)
				{
					// 在进行 SetViewTarget 之前冻结相机，不允许修改镜头，避免切换过程中出现抖动
					JoyCamera->FrozeCamera();
					const FMinimalViewInfo& FrozenView = JoyCamera->GetFrozenView();

					JoyPlayerController->SetControlRotation(FrozenView.Rotation);
				}

				PCM->ResetCameraToPlayer(TimeToResetViewTarget);
			}
		}

		bResetViewTarget = false;
	}
}

FCameraModifyHandle::FCameraModifyHandle(int64 Seq) : SequenceID(Seq)
{
}
