// Copyright Epic Games, Inc. All Rights Reserved.
#include "JoyPlayerCameraManager.h"

#include "Camera/CameraModifier.h"
#include "Character/JoyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameDelegates.h"
#include "GameFramework/PlayerController.h"
#include "Gameplay/Gravity/JoyGravityManageSubsystem.h"
#include "Gameplay/JoyCharacterControlManageSubsystem.h"
#include "Gameplay/TimeDilation/JoyTimeDilationManageSubsystem.h"
#include "JoyCameraComponent.h"
#include "JoyGameBlueprintLibrary.h"
#include "Controller/JoyCameraConfigController.h"
#include "Controller/JoyCameraInputController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Memory/MemoryView.h"
#include "Player/JoyPlayerController.h"

class AJoyHeroCharacter;
DECLARE_CYCLE_STAT(TEXT("Camera ProcessViewRotation"), STAT_Camera_ProcessViewRotation, STATGROUP_Game);

FVirtualCamera& FVirtualCamera::operator=(const FVirtualCamera& Other)
{
	this->CopyCamera(Other);
	return *this;
}

void FVirtualCamera::CopyCamera(const FVirtualCamera& Other)
{
	ArmCenterOffset = Other.ArmCenterOffset;
	ArmLength = Other.ArmLength;
	MinArmLength = Other.MinArmLength;
	MaxArmLength = Other.MaxArmLength;
	LocalArmCenterOffset = Other.LocalArmCenterOffset;
	WorldArmOffsetAdditional = Other.WorldArmOffsetAdditional;
	Fov = Other.Fov;
	ArmCenterRotation = Other.ArmCenterRotation;
}

void FVirtualCamera::SetArmCenterOffset(const FVector& NewArmCenterOffset)
{
	ArmCenterOffset = NewArmCenterOffset;
}

void FVirtualCamera::SetArmCenterRotation(const FRotator& NewArmCenterRotation)
{
	ArmCenterRotation = NewArmCenterRotation;
}

void AJoyPlayerCameraManager::BlendViewInfo(FMinimalViewInfo& A, FMinimalViewInfo& B, float T)
{
	switch (BlendViewType)
	{
		case EJoyCameraBlendType::Default:
			BlendViewFunc_Default(A, B, T);
			break;
		case EJoyCameraBlendType::LockTarget:
			BlendViewFunc_ArcCurve(A, B, T);
			break;
		case EJoyCameraBlendType::KeepDirection:
			BlendViewFunc_KeepViewDirection(A, B, T);
			break;
	}
}

AJoyPlayerCameraManager::AJoyPlayerCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AJoyPlayerCameraManager::BeginPlay()
{
	UJoyGameBlueprintLibrary::RegisterInputBlocker(GetWorld(), this);
	Super::BeginPlay();
}

void AJoyPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);
}

float AJoyPlayerCameraManager::ComputeTimeDilation(float DeltaTime) const
{
	if (const auto* TimeSys = UJoyTimeDilationManageSubsystem::Get(GetWorld()))
	{
		const float Dilation = TimeSys->GetGlobalTimeDilation();
		return DeltaTime * (1.0 / Dilation);
	}
	else
	{
		return DeltaTime;
	}
}

void AJoyPlayerCameraManager::EndCurrentCameraFadingProcess()
{
	CameraConfigFadingDescription.bDuringFading = false;
	if (CameraConfigFadingDescription.bOverrideCameraInput)
	{
		SetArmPitchInputEnabled(true);
		SetArmYawInputEnabled(true);
	}

	CameraConfigFadingDescription.bOverrideCameraInput = false;
}

void AJoyPlayerCameraManager::UpdateCameraConfigs(UJoyCameraComponent* CameraComponent)
{
	if (CameraComponent == nullptr)
	{
		return;
	}

	if (CameraComponent->IsCameraConfigDirty())
	{
		const TArray<FName> SubCameraIDs = CameraComponent->GetCameraIDs();
		if (SubCameraIDs.Num() < CameraConfigDescription.CameraStack.Num() && CameraConfigDescription.CameraStack.Num()
		    > 0)
		{
			CameraConfigDescription.FadeOutCamera = CameraConfigDescription.CameraStack[
				CameraConfigDescription.CameraStack.Num() - 1];
		}

		CameraConfigDescription.CameraStack = SubCameraIDs;
		CameraComponent->RefreshCameraConfig();
		if (CameraConfigController != nullptr)
		{
			CameraConfigController->MarkDirty();
		}

		CameraConfigDescription.bForceUpdateCameraConfigSet = false;
	}
	else if (CameraConfigDescription.bForceUpdateCameraConfigSet)
	{
		const auto& CameraIDs = CameraComponent->GetCameraIDs();
		bool bCameraConfigEqual = (CameraConfigDescription.CameraStack.Num() == CameraIDs.Num());
		if (bCameraConfigEqual)
		{
			for (int i = 0; i < CameraIDs.Num(); i++)
			{
				if (CameraConfigDescription.CameraStack[i] != CameraIDs[i])
				{
					bCameraConfigEqual = false;
					break;
				}
			}
		}

		if (!bCameraConfigEqual)
		{
			CameraConfigDescription.CameraStack = CameraIDs;
			if (CameraConfigController != nullptr)
			{
				CameraConfigController->MarkDirty();
			}
		}

		CameraConfigDescription.bForceUpdateCameraConfigSet = false;
	}
}

void AJoyPlayerCameraManager::UpdateCameraControllers(float DeltaTime)
{
	if (CameraConfigController != nullptr)
	{
		const AActor* TargetCameraOwner = PendingViewTarget.Target;
		if (TargetCameraOwner == nullptr || !TargetCameraOwner->IsA<AJoyHeroCharacter>() ||
		    UJoyCameraComponent::FindCameraComponent(TargetCameraOwner) == nullptr)
		{
			TargetCameraOwner = ViewTarget.Target;
		}

		if (auto* Camera = UJoyCameraComponent::FindCameraComponent(TargetCameraOwner))
		{
			UpdateCameraConfigs(Camera);
		}

		CameraConfigController->Update(DeltaTime);
	}

	if (CameraInputController != nullptr)
	{
		CameraInputController->Update(DeltaTime);
	}

	for (auto ItViewTarget = MultiViewTargetCameraManager.ViewTargetCameraInfos.CreateConstIterator(); ItViewTarget;
	     ++ItViewTarget)
	{
		TWeakObjectPtr<AActor> NewViewTarget = ItViewTarget.Key();
		const FViewTargetCameraInfo& CameraInfo = ItViewTarget.Value();

		if (NewViewTarget.Get() == nullptr)
		{
			continue;
		}

		if (!NeedUpdateViewTarget(NewViewTarget.Get(), CameraInfo))
		{
			continue;
		}

		if (CameraInfo.CameraModifierController != nullptr)
		{
			CameraInfo.CameraModifierController->Update(DeltaTime);
		}
	}
}

void AJoyPlayerCameraManager::UpdateViewTargetPose()
{
	const auto* CharacterControlManager = UJoyCharacterControlManageSubsystem::Get(GetWorld());
	for (TMap<TWeakObjectPtr<AActor>, FViewTargetCameraInfo>::TIterator ItViewTarget =
		     MultiViewTargetCameraManager.ViewTargetCameraInfos.CreateIterator();
	     ItViewTarget; ++ItViewTarget)
	{
		TWeakObjectPtr<AActor> NewViewTarget = ItViewTarget.Key();
		FViewTargetCameraInfo& CameraInfo = ItViewTarget.Value();

		if (NewViewTarget.Get() == nullptr)
		{
			continue;
		}

		bool bViewTargetFound = false;
		if (CharacterControlManager)
		{
			if (NewViewTarget == CharacterControlManager->GetCurrentControlCharacter())
			{
				bViewTargetFound = true;
			}
		}

		if (bViewTargetFound || NewViewTarget == ViewTarget.Target || NewViewTarget == PendingViewTarget.Target)
		{
			// 此处做一些对 DesiredCamera 数据的重置操作
			CameraInfo.DesiredCamera.ArmCenterRotation = CameraInfo.CurrentCamera.ArmCenterRotation;
		}
	}
}

bool AJoyPlayerCameraManager::NeedUpdateViewTarget(AActor* InViewTarget, const FViewTargetCameraInfo& CameraInfo) const
{
	/** ******************************************************************************** **
	 * (1) ViewTarget 处于 PlayerCameraManager
	 *的控制中(ViewTarget、PendingViewTarget) (2) CameraModifierController
	 *正在修改 ViewTarget
	 ** ******************************************************************************** **/
	if (InViewTarget != nullptr)
	{
		if (ViewTarget.Target == InViewTarget || PendingViewTarget.Target == InViewTarget)
		{
			return true;
		}

		if (CameraInfo.CameraModifierController.Get() != nullptr &&
		    CameraInfo.CameraModifierController.Get()->IsModifiedAndNeedUpdate())
		{
			return true;
		}
	}

	return false;
}

void AJoyPlayerCameraManager::ClearUnusedViewTargets()
{
	for (TMap<TWeakObjectPtr<AActor>, FViewTargetCameraInfo>::TIterator ItViewTarget =
		     MultiViewTargetCameraManager.ViewTargetCameraInfos.CreateIterator();
	     ItViewTarget; ++ItViewTarget)
	{
		TWeakObjectPtr<AActor> NewViewTarget = ItViewTarget.Key();
		if (NewViewTarget.Get() == nullptr)
		{
			ItViewTarget.RemoveCurrent();
		}
	}
}

void AJoyPlayerCameraManager::InternalUpdateCamera(float DeltaTime)
{
	/**
	 * 设计思路：每个 View Target 用三个变量存储相机相关信息
	 * 1. LastCamera: 存储 Fading、Modifiy
	 *执行之前的相机状态，用于执行过程中的插值
	 * 2. DesiredCamera: 存储每帧计算出的期望相机状态
	 * 3. CurrentCamera: 存储每帧计算出的最终相机状态。它通常在 DesiredCamera
	 *的基础上计算出来， 例如在 Fading 过程中，CurrentCamera 由 LastCamera 和
	 *DesiredCamera 之间插值计算 得到。CurrentCamera
	 *最终提供给外部，用于决定相机位置朝向。
	 */

	// 清理无用 View Targets
	ClearUnusedViewTargets();

	/*
	 * 预备修改相机参数前，设置 DesiredCamera 的 ArmRotation 相关参数
	 * ArmRotation -> 重置为 PlayerController 的方向
	 */
	UpdateViewTargetPose();

	/*
	 * 一部分相机参数修改后，会记录到标志位，于是在每帧修改之前，需把这些标志位置为
	 * false
	 */
	ResetModifiedMarkers();

	// 调整镜头位姿
	UpdateCameraControllers(DeltaTime);

	// 还没实现，目前看也没必要
	UpdateArmLocation(DeltaTime);

	/*
	 * 上述代码修改过后的参数保留在 DesiredCamera 中，SyncDesireCameraData 将
	 * DesiredCamera 数据同步到 CurrentCamera 中
	 */
	SyncDesireCameraData(DeltaTimeThisFrame_IgnoreTimeDilation);

	/*
	 * CurrentCamera 计算好之后做的一些扫尾工作
	 */
	UpdateActorTransform(DeltaTime);

	bMoveInput = false;
}

void AJoyPlayerCameraManager::SyncDesireCameraData(float DeltaTime)
{
	CameraConfigFadingDescription.ElapseTime += DeltaTime;
	if (CameraConfigFadingDescription.bDuringFading && CameraConfigFadingDescription.ElapseTime >=
	    CameraConfigFadingDescription.Duration)
	{
		EndCurrentCameraFadingProcess();
	}

	for (TMap<TWeakObjectPtr<AActor>, FViewTargetCameraInfo>::TIterator ItViewTarget =
		     MultiViewTargetCameraManager.ViewTargetCameraInfos.CreateIterator();
	     ItViewTarget; ++ItViewTarget)
	{
		TWeakObjectPtr<AActor> NewViewTarget = ItViewTarget.Key();
		FViewTargetCameraInfo& CameraInfo = ItViewTarget.Value();

		if (NewViewTarget.Get() == nullptr)
		{
			continue;
		}

		if (!NeedUpdateViewTarget(NewViewTarget.Get(), CameraInfo))
		{
			continue;
		}

		if (!CameraConfigFadingDescription.bDuringFading)
		{
			CameraInfo.CurrentCamera.CopyCamera(CameraInfo.DesiredCamera);
			continue;
		}

		if (!CameraInfo.bNeedFading)
		{
			CameraInfo.CurrentCamera.CopyCamera(CameraInfo.DesiredCamera);
			continue;
		}

		const float BlendAlpha = FMath::Clamp(CameraConfigFadingDescription.ElapseTime / CameraConfigFadingDescription.Duration, 0, 1);
		if (CameraInfo.bFadeArmLength && !CameraInfo.bCameraArmLength_HasModified)
		{
			CameraInfo.CurrentCamera.ArmLength =
				FMath::Lerp(CameraInfo.LastCamera.ArmLength, CameraInfo.DesiredCamera.ArmLength, BlendAlpha);
		}
		else
		{
			CameraInfo.bFadeArmLength = false;
			CameraInfo.CurrentCamera.ArmLength = CameraInfo.DesiredCamera.ArmLength;
		}

		if (CameraInfo.bFadeArmLengthRange)
		{
			CameraInfo.CurrentCamera.MinArmLength =
				FMath::Lerp(CameraInfo.LastCamera.MinArmLength, CameraInfo.DesiredCamera.MinArmLength, BlendAlpha);
			CameraInfo.CurrentCamera.MaxArmLength =
				FMath::Lerp(CameraInfo.LastCamera.MaxArmLength, CameraInfo.DesiredCamera.MaxArmLength, BlendAlpha);
		}
		else
		{
			CameraInfo.CurrentCamera.MinArmLength = CameraInfo.DesiredCamera.MinArmLength;
			CameraInfo.CurrentCamera.MaxArmLength = CameraInfo.DesiredCamera.MaxArmLength;
		}

		if (CameraInfo.bFadeLocalArmCenterOffset && !CameraInfo.bCameraOffset_HasModified)
		{
			CameraInfo.CurrentCamera.LocalArmCenterOffset = FMath::Lerp(
				CameraInfo.LastCamera.LocalArmCenterOffset, CameraInfo.DesiredCamera.LocalArmCenterOffset, BlendAlpha);

			CameraInfo.CurrentCamera.WorldArmOffsetAdditional =
				FMath::Lerp(CameraInfo.LastCamera.WorldArmOffsetAdditional,
					CameraInfo.DesiredCamera.WorldArmOffsetAdditional, BlendAlpha);
		}
		else
		{
			CameraInfo.bFadeLocalArmCenterOffset = false;
			CameraInfo.CurrentCamera.LocalArmCenterOffset = CameraInfo.DesiredCamera.LocalArmCenterOffset;
			CameraInfo.CurrentCamera.WorldArmOffsetAdditional = CameraInfo.DesiredCamera.WorldArmOffsetAdditional;
		}

		if (CameraInfo.bFadeArmPitch && !CameraInfo.bArmPitch_HasModified)
		{
			CameraInfo.CurrentCamera.ArmCenterRotation.Pitch =
				FMath::Lerp(CameraInfo.LastCamera.ArmCenterRotation.Pitch, FadingTarget_ArmPitch, BlendAlpha);
		}
		else
		{
			CameraInfo.bFadeArmPitch = false;
			CameraInfo.CurrentCamera.ArmCenterRotation.Pitch = CameraInfo.DesiredCamera.ArmCenterRotation.Pitch;
		}

		if (CameraInfo.bFadeArmYaw && !CameraInfo.bArmYaw_HasModified)
		{
			CameraInfo.CurrentCamera.ArmCenterRotation.Yaw =
				FMath::Lerp(CameraInfo.LastCamera.ArmCenterRotation.Yaw, FadingTarget_ArmYaw, BlendAlpha);
		}
		else
		{
			CameraInfo.bFadeArmYaw = false;
			CameraInfo.CurrentCamera.ArmCenterRotation.Yaw = CameraInfo.DesiredCamera.ArmCenterRotation.Yaw;
		}

		if (CameraInfo.bFadeCameraFov && !CameraInfo.bCameraFov_HasModified)
		{
			CameraInfo.CurrentCamera.Fov =
				FMath::Lerp(CameraInfo.LastCamera.Fov, CameraInfo.DesiredCamera.Fov, BlendAlpha);
		}
		else
		{
			CameraInfo.bFadeCameraFov = false;
			CameraInfo.CurrentCamera.Fov = CameraInfo.DesiredCamera.Fov;
		}	
	}
}

void AJoyPlayerCameraManager::UpdateArmLocation(float DeltaTime)
{
	// @TODO
}

void AJoyPlayerCameraManager::UpdateActorTransform(float DeltaTime)
{
	const auto* GravityManager = UJoyGravityManageSubsystem::Get(GetWorld());
	for (TMap<TWeakObjectPtr<AActor>, FViewTargetCameraInfo>::TIterator ItViewTarget =
		     MultiViewTargetCameraManager.ViewTargetCameraInfos.CreateIterator();
	     ItViewTarget; ++ItViewTarget)
	{
		TWeakObjectPtr<AActor> NewViewTarget = ItViewTarget.Key();
		FViewTargetCameraInfo& CameraInfo = ItViewTarget.Value();

		if (NewViewTarget.Get() == nullptr)
		{
			continue;
		}

		if (!NeedUpdateViewTarget(NewViewTarget.Get(), CameraInfo))
		{
			continue;
		}

		// 对局部坐标下的 Rotator 做限制
		FRotator WorldRotator = CameraInfo.CurrentCamera.ArmCenterRotation;
		FRotator LocalRotator =
			GravityManager != nullptr ? GravityManager->WorldRotatorToLocal(WorldRotator) : WorldRotator;
		LocalRotator.Pitch = FMath::Clamp(LocalRotator.Pitch, MinArmPitch, MaxArmPitch);
		WorldRotator = GravityManager != nullptr ? GravityManager->LocalRotatorToWorld(LocalRotator) : LocalRotator;
		
		// 更新 Controller 的控制方向
		SetRotationInternal(NewViewTarget.Get(), WorldRotator);

		// 计算 Camera Offset 对相机臂的偏移影响
		const FRotator ViewRotator = GetViewTargetViewRotation(NewViewTarget.Get());
		const FRotator CameraSpace = FRotator(ViewRotator.Pitch, ViewRotator.Yaw, ViewRotator.Roll);

		const FVector ForwardVec = CameraSpace.RotateVector(FVector(1.0, 0.0, 0.0));
		const FVector RightVec = CameraSpace.RotateVector(FVector(0.0, 1.0, 0.0));
		const FVector UpVec = CameraSpace.RotateVector(FVector(0.0, 0.0, 1.0));

		CameraInfo.CurrentCamera.SetArmCenterOffset(CameraInfo.CurrentCamera.LocalArmCenterOffset.X * ForwardVec +
		                                            CameraInfo.CurrentCamera.LocalArmCenterOffset.Y * RightVec +
		                                            CameraInfo.CurrentCamera.LocalArmCenterOffset.Z * UpVec);

		CameraInfo.CurrentCamera.SetArmCenterOffset(
			CameraInfo.CurrentCamera.ArmCenterOffset + CameraInfo.CurrentCamera.WorldArmOffsetAdditional);
	}
}

void AJoyPlayerCameraManager::SetRotationInternal(const AActor* InViewTarget, FRotator Rotator)
{
	if (InViewTarget == nullptr)
	{
		return;
	}

	if (PCOwner != nullptr)
	{
		const AActor* CurViewTarget = ViewTarget.Target;
		// 如果 ViewTarget 与当前操控的角色一致，才会设置控制器方向
		if (InViewTarget == CurViewTarget)
		{
			PCOwner->SetControlRotation(Rotator);
		}
	}
}

void AJoyPlayerCameraManager::DoUpdateCamera(float InDeltaTime)
{
	InternalUpdateCamera(DeltaTimeThisFrame_IgnoreTimeDilation);

	FMinimalViewInfo NewPOV = ViewTarget.POV;

	// update color scale interpolation
	if (bEnableColorScaleInterp)
	{
		const float BlendPct =
			FMath::Clamp((GetWorld()->TimeSeconds - ColorScaleInterpStartTime) / ColorScaleInterpDuration, 0.f, 1.0f);
		ColorScale = FMath::Lerp(OriginalColorScale, DesiredColorScale, BlendPct);
		// if we've maxed
		if (BlendPct == 1.0f)
		{
			// disable further interpolation
			bEnableColorScaleInterp = false;
		}
	}

	// Don't update outgoing viewtarget during an interpolation when bLockOutgoing
	// is set.
	if ((PendingViewTarget.Target == NULL) || !BlendParams.bLockOutgoing)
	{
		// Update current view target
		ViewTarget.CheckViewTarget(PCOwner);
		UpdateViewTarget(ViewTarget, DeltaTimeThisFrame_IgnoreTimeDilation);
	}

	// our camera is now viewing there
	NewPOV = ViewTarget.POV;

	// if we have a pending view target, perform transition from one to another.
	if (PendingViewTarget.Target != NULL)
	{
		BlendTimeToGo -= DeltaTimeThisFrame_IgnoreTimeDilation;

		// Update pending view target
		PendingViewTarget.CheckViewTarget(PCOwner);
		UpdateViewTarget(PendingViewTarget, DeltaTimeThisFrame_IgnoreTimeDilation);

		// blend....
		if (BlendTimeToGo > 0)
		{
			const float DurationPct = (BlendParams.BlendTime - BlendTimeToGo) / BlendParams.BlendTime;
			float BlendPct = 0.f;
			if (!BlendViewCurve.Get())
			{
				switch (BlendParams.BlendFunction)
				{
					case VTBlend_Linear:
						BlendPct = FMath::Lerp(0.f, 1.f, DurationPct);
						break;
					case VTBlend_Cubic:
						BlendPct = FMath::CubicInterp(0.f, 0.f, 1.f, 0.f, DurationPct);
						break;
					case VTBlend_EaseIn:
						BlendPct = FMath::Lerp(0.f, 1.f, FMath::Pow(DurationPct, BlendParams.BlendExp));
						break;
					case VTBlend_EaseOut:
						BlendPct = FMath::Lerp(0.f, 1.f, FMath::Pow(DurationPct, 1.f / BlendParams.BlendExp));
						break;
					case VTBlend_EaseInOut:
						BlendPct = FMath::InterpEaseInOut(0.f, 1.f, DurationPct, BlendParams.BlendExp);
						break;
					case VTBlend_PreBlended:
						BlendPct = 1.0f;
						break;
					default:
						break;
				}
			}
			else
			{
				BlendPct = FMath::Clamp(BlendViewCurve->GetFloatValue(DurationPct), 0.f, 1.f);
			}

			// Update pending view target blend
			NewPOV = ViewTarget.POV;
			BlendViewInfo(NewPOV, PendingViewTarget.POV, BlendPct);
		}
		else
		{
			OnViewTargetBlendComplete.Broadcast(ViewTarget.Target, PendingViewTarget.Target);

			// we're done blending, set new view target
			ViewTarget = PendingViewTarget;

			// clear pending view target
			PendingViewTarget.Target = nullptr;

			BlendTimeToGo = 0;
			BlendViewCurve = nullptr;

			// our camera is now viewing there
			NewPOV = PendingViewTarget.POV;

			OnBlendComplete().Broadcast();
		}
	}

	if (bEnableFading)
	{
		if (bAutoAnimateFade)
		{
			FadeTimeRemaining = FMath::Max(FadeTimeRemaining - DeltaTimeThisFrame_IgnoreTimeDilation, 0.0f);
			if (FadeTime > 0.0f)
			{
				FadeAmount = FadeAlpha.X + ((1.f - FadeTimeRemaining / FadeTime) * (FadeAlpha.Y - FadeAlpha.X));
			}

			if ((bHoldFadeWhenFinished == false) && (FadeTimeRemaining <= 0.f))
			{
				// done
				StopCameraFade();
			}
		}

		if (bFadeAudio)
		{
			ApplyAudioFade();
		}
	}

	if (AllowPhotographyMode())
	{
		const bool bPhotographyCausedCameraCut = UpdatePhotographyCamera(NewPOV);
		bGameCameraCutThisFrame = bGameCameraCutThisFrame || bPhotographyCausedCameraCut;
	}

	// Cache results
	FillCameraCache(NewPOV);
}

void AJoyPlayerCameraManager::AddNewViewTarget(AActor* NewViewTarget)
{
	if (MultiViewTargetCameraManager.ContainsViewTarget(NewViewTarget))
	{
		return;
	}

	// Virtual camera 的默认值
	FVirtualCamera VirtualCamera{};
	VirtualCamera.ArmLength = BaseArmLength;
	VirtualCamera.MinArmLength = MinArmLength;
	VirtualCamera.MaxArmLength = MaxArmLength;
	VirtualCamera.Fov = BaseFov;
	VirtualCamera.LocalArmCenterOffset = GetBaseLocalArmOffset();
	VirtualCamera.WorldArmOffsetAdditional = FVector::ZeroVector;
	VirtualCamera.ArmCenterRotation = GetViewTargetViewRotation(NewViewTarget);
	// VirtualCamera.ArmCenterRotation = FRotator::ZeroRotator;
	MultiViewTargetCameraManager.AddViewTarget(this, NewViewTarget, VirtualCamera);
}

void AJoyPlayerCameraManager::InitializeFor(APlayerController* PC)
{
	// 初始化 CurrentCamera
	for (TPair<TWeakObjectPtr<AActor>, FViewTargetCameraInfo>& ModifierContainer :
	     MultiViewTargetCameraManager.ViewTargetCameraInfos)
	{
		auto NewViewTarget = ModifierContainer.Key;
		FViewTargetCameraInfo& CameraInfo = ModifierContainer.Value;
		if (!NeedUpdateViewTarget(NewViewTarget.Get(), CameraInfo))
		{
			continue;
		}

		// 初始化 CameraInfo.CurrentCamera
		CameraInfo.CurrentCamera.ArmLength = BaseArmLength;
		CameraInfo.CurrentCamera.MinArmLength = MinArmLength;
		CameraInfo.CurrentCamera.MaxArmLength = MaxArmLength;
		CameraInfo.CurrentCamera.Fov = BaseFov;
		CameraInfo.CurrentCamera.LocalArmCenterOffset = GetBaseLocalArmOffset();
		CameraInfo.CurrentCamera.WorldArmOffsetAdditional = FVector::ZeroVector;
		CameraInfo.CurrentCamera.ArmCenterRotation = FRotator::ZeroRotator;

		// 初始化 LastCamera、CurrentCamera、DesiredCamera
		CameraInfo.LastCamera.CopyCamera(CameraInfo.CurrentCamera);
		CameraInfo.DesiredCamera.CopyCamera(CameraInfo.CurrentCamera);

		CameraInfo.CameraModifierController = NewObject<UJoyCameraModifierController>(this);
		check(CameraInfo.CameraModifierController);
		CameraInfo.CameraModifierController->InitializeFor(this);
		CameraInfo.CameraModifierController->ModifiedViewTarget = NewViewTarget.Get();
	}

	CameraInputController = NewObject<UJoyCameraInputController>(this);
	check(CameraInputController);
	CameraInputController->InitializeFor(this);

	// CameraConfigController 要放在最后初始化，因为它要为其他 Controller 设置参数
	CameraConfigController = NewObject<UJoyCameraConfigController>(this);
	check(CameraConfigController);
	CameraConfigController->InitializeFor(this);

	Super::InitializeFor(PC);
	FGameDelegates::Get().GetViewTargetChangedDelegate().AddUObject(this, &ThisClass::OnViewTargetChanged);
}

void AJoyPlayerCameraManager::ResetModifiedMarkers()
{
	for (TMap<TWeakObjectPtr<AActor>, FViewTargetCameraInfo>::TIterator ItViewTarget =
		     MultiViewTargetCameraManager.ViewTargetCameraInfos.CreateIterator();
	     ItViewTarget; ++ItViewTarget)
	{
		TWeakObjectPtr<AActor> NewViewTarget = ItViewTarget.Key();
		FViewTargetCameraInfo& CameraInfo = ItViewTarget.Value();

		if (NewViewTarget.Get() == nullptr)
		{
			continue;
		}

		CameraInfo.bCameraOffset_HasModified = false;
		CameraInfo.bCameraArmLength_HasModified = false;
		CameraInfo.bArmPitch_HasModified = false;
		CameraInfo.bArmYaw_HasModified = false;
		CameraInfo.bArmRoll_HasModified = false;
	}
}

FVector AJoyPlayerCameraManager::GetBaseLocalArmOffset() const
{
	return FVector(ArmCenterOffsetX, ArmCenterOffsetY, ArmCenterOffsetZ);
}

float AJoyPlayerCameraManager::GetBaseMaxArmLength() const
{
	return MaxArmLength;
}

float AJoyPlayerCameraManager::GetBaseMinArmLength() const
{
	return MinArmLength;
}

float AJoyPlayerCameraManager::GetBaseArmLength() const
{
	return BaseArmLength;
}

float AJoyPlayerCameraManager::GetBaseCameraFov() const
{
	return BaseFov;
}

FRotator AJoyPlayerCameraManager::GetViewTargetViewRotation(const AActor* InViewTarget)
{
	if (auto* PawnTarget = Cast<APawn>(InViewTarget))
	{
		return PawnTarget->GetViewRotation();
	}

	return FRotator::ZeroRotator;
}

FVector AJoyPlayerCameraManager::GetNegativeGravityNormal() const
{
	const auto* GravityManager = UJoyGravityManageSubsystem::Get(GetWorld());
	return GravityManager != nullptr ? GravityManager->GetGravitySpaceZ() : FVector(0., 0., 1.f);
}

FVector AJoyPlayerCameraManager::GetCharacterHeadLocation(const AActor* Target) const
{
	if (Target == nullptr)
	{
		return FVector::ZeroVector;
	}

	const AActor* TargetActor = Target;
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		// Height adjustments for characters to account for crouching.
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
		{
			// 获取相机顶部位置
			const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
			check(TargetCharacterCDO);

			const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
			check(CapsuleComp);

			const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
			check(CapsuleCompCDO);

			const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
			const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
			const float HeightAdjustment = (DefaultHalfHeight - ActualHalfHeight) + TargetCharacterCDO->BaseEyeHeight;

			return TargetCharacter->GetActorLocation() + (GetNegativeGravityNormal() * HeightAdjustment);
		}

		return TargetPawn->GetPawnViewLocation();
	}

	return TargetActor->GetActorLocation();
}

FVector AJoyPlayerCameraManager::GetCharacterFaceBase(const AActor* Target)
{
	if (Target == nullptr)
	{
		return FVector::ZeroVector;
	}

	const AActor* TargetActor = Target;
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		if (const auto* SkeletalComp = TargetPawn->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (const USkeletalMesh* SkeletalMeshAsset = SkeletalComp->GetSkeletalMeshAsset())
			{
				if (const USkeleton* Skeleton = SkeletalMeshAsset->GetSkeleton())
				{
					if (Skeleton->FindSocket(FName("FacePoint")) != nullptr)
					{
						return SkeletalComp->GetSocketLocation(FName("FacePoint"));
					}
				}
			}
		}
	}

	return GetCharacterHeadLocation(Target);
}

FVector AJoyPlayerCameraManager::GetCharacterCameraBase(const AActor* Target) const
{
	if (Target == nullptr)
	{
		return FVector::ZeroVector;
	}

	const AActor* TargetActor = Target;
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		if (const auto* SkeletalComp = TargetPawn->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (const USkeletalMesh* SkeletalMeshAsset = SkeletalComp->GetSkeletalMeshAsset())
			{
				if (const USkeleton* Skeleton = SkeletalMeshAsset->GetSkeleton())
				{
					if (Skeleton->FindSocket(FName("CameraPoint")) != nullptr)
					{
						return SkeletalComp->GetSocketLocation(FName("CameraPoint"));
					}
				}
			}
		}
	}

	return GetCharacterHeadLocation(Target);
}

FRotator AJoyPlayerCameraManager::GetCurrentCameraArmCenterRotation(AActor* InViewTarget) const
{
	if (InViewTarget != nullptr && MultiViewTargetCameraManager.ContainsViewTarget(InViewTarget))
	{
		return MultiViewTargetCameraManager[InViewTarget].CurrentCamera.ArmCenterRotation;
	}

	return FRotator::ZeroRotator;
}

float AJoyPlayerCameraManager::GetCurrentArmLength(AActor* InViewTarget) const
{
	if (InViewTarget != nullptr && MultiViewTargetCameraManager.ContainsViewTarget(InViewTarget))
	{
		return MultiViewTargetCameraManager[InViewTarget].CurrentCamera.ArmLength;
	}

	return 0.;
}

FViewTargetCameraInfo* AJoyPlayerCameraManager::GetViewTargetCameraInfo(const AActor* InViewTarget)
{
	if (InViewTarget != nullptr && MultiViewTargetCameraManager.ContainsViewTarget(InViewTarget))
	{
		return &MultiViewTargetCameraManager[InViewTarget];
	}

	return nullptr;
}

FVector AJoyPlayerCameraManager::GetBaseArmCenterLocalOffset() const
{
	return FVector(ArmCenterOffsetX, ArmCenterOffsetY, ArmCenterOffsetZ);
}

FVector AJoyPlayerCameraManager::GetCurrentArmCenterOffset(AActor* InViewTarget) const
{
	if (InViewTarget != nullptr && MultiViewTargetCameraManager.ContainsViewTarget(InViewTarget))
	{
		return MultiViewTargetCameraManager[InViewTarget].CurrentCamera.ArmCenterOffset;
	}

	return FVector::ZeroVector;
}

float AJoyPlayerCameraManager::GetCurrentCameraFov(AActor* InViewTarget) const
{
	if (InViewTarget != nullptr && MultiViewTargetCameraManager.ContainsViewTarget(InViewTarget))
	{
		return MultiViewTargetCameraManager[InViewTarget].CurrentCamera.Fov;
	}

	return 0.f;
}

bool AJoyPlayerCameraManager::IsDuringMemberSwitching() const
{
	if (const auto* PlayerController = UJoyGameBlueprintLibrary::GetJoyPlayerController(this))
	{
		return PlayerController->CheckDuringCharacterSwitching();
	}

	return false;
}

void AJoyPlayerCameraManager::SetViewTarget(AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams)
{
	if (NewViewTarget)
	{
		TInlineComponentArray<UJoyCameraComponent*> CameraComponents;
		NewViewTarget->GetComponents(CameraComponents);

		for (auto* Camera : CameraComponents)
		{
			if (Camera->IsActive())
			{
				Camera->CameraDataThisFrame.Clean();
			}
		}

		// 新 ViewTarget 纳入镜头管理
		if (auto* NewPawn = Cast<APawn>(NewViewTarget); NewPawn != nullptr)
		{
			AddNewViewTarget(NewPawn);
		}
	}

	Super::SetViewTarget(NewViewTarget, TransitionParams);
}

void AJoyPlayerCameraManager::SetViewTargetWithCurveBlend(AActor* NewViewTarget, TObjectPtr<UCurveFloat> BlendCurve,
	bool bEnableUpdateCameraConfig, FViewTargetTransitionParams TransitionParams)
{
	if (NewViewTarget)
	{
		TInlineComponentArray<UJoyCameraComponent*> TargetCameras;
		NewViewTarget->GetComponents(TargetCameras);
		for (auto* Camera : TargetCameras)
		{
			if (Camera->IsActive())
			{
				Camera->CameraDataThisFrame.Clean();
			}
		}

		// 新 ViewTarget 纳入镜头管理
		if (auto* NewPawn = Cast<APawn>(NewViewTarget); NewPawn != nullptr)
		{
			AddNewViewTarget(NewPawn);
		}

		BlendViewCurve = nullptr;
		if (BlendCurve)
		{
			BlendViewCurve = BlendCurve;
		}
	}

	Super::SetViewTarget(NewViewTarget, TransitionParams);
}

bool AJoyPlayerCameraManager::IsCurrentViewTarget(const AActor* TestViewTarget) const
{
	return ViewTarget.Target.Get() == TestViewTarget;
}

bool AJoyPlayerCameraManager::IsPendingViewTarget(const AActor* TestViewTarget) const
{
	return PendingViewTarget.Target.Get() == TestViewTarget;
}

void AJoyPlayerCameraManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	DeltaTimeThisFrame = DeltaSeconds;
	DeltaTimeThisFrame_IgnoreTimeDilation = ComputeTimeDilation(DeltaSeconds);
}

void AJoyPlayerCameraManager::ResetCameraToPlayer(float InBlendTime) const
{
	if (const auto* CharacterControl = UJoyCharacterControlManageSubsystem::Get(GetWorld()))
	{
		AJoyCharacter* CurrentControlCharacter = CharacterControl->GetCurrentControlCharacter();
		const AActor* CurrentViewTarget = GetViewTarget();
		if (CurrentControlCharacter != CurrentViewTarget && PCOwner != nullptr)
		{
			const float BlendTime = InBlendTime >= 0 ? InBlendTime : 0;
			PCOwner->SetViewTargetWithBlend(CurrentControlCharacter, BlendTime);
		}
	}
}

void AJoyPlayerCameraManager::BlendViewFunc_Default(FMinimalViewInfo& A, FMinimalViewInfo& B, float T)
{
	A.BlendViewInfo(B, T);
}

void AJoyPlayerCameraManager::OnViewTargetChanged(APlayerController* PC, AActor* OldViewTarget, AActor* NewViewTarget)
{
	if (OldViewTarget != nullptr)
	{
		TObjectPtr<class UJoyCameraModifierController> CurCameraModifier =
			MultiViewTargetCameraManager.GetCameraModifier(OldViewTarget);
		if (CurCameraModifier.Get())
		{
			CurCameraModifier->BreakModifier();
		}
	}
}

void AJoyPlayerCameraManager::BlendViewFunc_ArcCurve(FMinimalViewInfo& A, FMinimalViewInfo& B, float T)
{
	const float L = FVector::DistXY(A.Location, B.Location);
	FVector MoveCurveCenter = (A.Location + B.Location) / 2.0;
	{
		FVector A_Direction = A.Rotation.Vector();
		FVector B_Direction = B.Rotation.Vector();

		float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(A_Direction, B_Direction)));
		if (Angle >= ThresholdWhenOffsetBegin)
		{
			// 计算 A 点到 B 点方向的垂直方向向量
			FVector NormalDirection = B.Location - A.Location;
			{
				NormalDirection.Z = 0;
				FVector UpVector(0., 0., 1.);
				NormalDirection = NormalDirection.Cross(UpVector).GetSafeNormal();
			}

			// 判断向量 A B 方向合并向量和向量 NormalDirection 是否同向，如果否则反转
			// NormalDirection 方向
			if (FVector::DotProduct((A_Direction + B_Direction) * 0.5, NormalDirection) > 0)
			{
				NormalDirection = -NormalDirection;
			}

			// float XYOffsetScalar = FMath::Min(MaxXYOffsetParam_A * Angle +
			// MaxXYOffsetParam_B, MaxXYOffsetParam_C);
			float XYOffsetScalar = FMath::Min(MaxXYOffsetParam_A * L + MaxXYOffsetParam_B, MaxXYOffsetParam_C);
			FVector XYOffset = XYOffsetScalar * NormalDirection;
			MoveCurveCenter += XYOffset;
		}
	}

	// 能够上升的最大高度
	float MaxHeightDelta = FMath::Min(MaxHeightParam_A * L + MaxHeightParam_B, MaxHeightParam_C);
	MoveCurveCenter.Z += MaxHeightDelta;

	// 计算 Bezier 曲线上移动到的位置
	FVector Bezier_Loc =
		FMath::Square(1. - T) * A.Location + 2. * T * (1 - T) * MoveCurveCenter + FMath::Square(T) * B.Location;
	A.Location = Bezier_Loc;

	FRotator CurRotator = (B.Rotation - A.Rotation).GetNormalized() * T;
	A.Rotation = A.Rotation + CurRotator;

	{
		// Bezier 插值计算当前 pitch
		const float MaxPitchDegree = FMath::Max(-(PitchParam_A * MaxHeightDelta + PitchParam_B), -85);
		const float CurPitch = FMath::Square(1. - T) * A.Rotation.Pitch + 2. * T * (1 - T) * MaxPitchDegree +
		                       FMath::Square(T) * B.Rotation.Pitch;
		A.Rotation.Pitch = CurPitch;
	}

	A.FOV = FMath::Lerp(A.FOV, B.FOV, T);
	A.OrthoWidth = FMath::Lerp(A.OrthoWidth, B.OrthoWidth, T);
	A.OrthoNearClipPlane = FMath::Lerp(A.OrthoNearClipPlane, B.OrthoNearClipPlane, T);
	A.OrthoFarClipPlane = FMath::Lerp(A.OrthoFarClipPlane, B.OrthoFarClipPlane, T);
	A.PerspectiveNearClipPlane = FMath::Lerp(A.PerspectiveNearClipPlane, B.PerspectiveNearClipPlane, T);
	A.OffCenterProjectionOffset = FMath::Lerp(A.OffCenterProjectionOffset, B.OffCenterProjectionOffset, T);

	A.AspectRatio = FMath::Lerp(A.AspectRatio, B.AspectRatio, T);
	A.bConstrainAspectRatio |= B.bConstrainAspectRatio;
	A.bUseFieldOfViewForLOD |= B.bUseFieldOfViewForLOD;
}

void AJoyPlayerCameraManager::BlendViewFunc_KeepViewDirection(FMinimalViewInfo& A, FMinimalViewInfo& B, float T)
{
	A.Location = FMath::Lerp(A.Location, B.Location, T);

	A.FOV = FMath::Lerp(A.FOV, B.FOV, T);
	A.OrthoWidth = FMath::Lerp(A.OrthoWidth, B.OrthoWidth, T);
	A.OrthoNearClipPlane = FMath::Lerp(A.OrthoNearClipPlane, B.OrthoNearClipPlane, T);
	A.OrthoFarClipPlane = FMath::Lerp(A.OrthoFarClipPlane, B.OrthoFarClipPlane, T);
	A.PerspectiveNearClipPlane = FMath::Lerp(A.PerspectiveNearClipPlane, B.PerspectiveNearClipPlane, T);
	A.OffCenterProjectionOffset = FMath::Lerp(A.OffCenterProjectionOffset, B.OffCenterProjectionOffset, T);

	A.AspectRatio = FMath::Lerp(A.AspectRatio, B.AspectRatio, T);
	A.bConstrainAspectRatio |= B.bConstrainAspectRatio;
	A.bUseFieldOfViewForLOD |= B.bUseFieldOfViewForLOD;
}

EJoyCameraBlendType AJoyPlayerCameraManager::GetBlendViewType() const
{
	return BlendViewType;
}

const FCameraConfigDescription& AJoyPlayerCameraManager::GetCameraConfigDescription() const
{
	return CameraConfigDescription;
}

void AJoyPlayerCameraManager::SetBlendViewType(EJoyCameraBlendType InBlendViewWay)
{
	BlendViewType = InBlendViewWay;
}

void AJoyPlayerCameraManager::ResetBlendViewWay()
{
	BlendViewType = EJoyCameraBlendType::Default;
}

void AJoyPlayerCameraManager::RemoveBlendViewWay(EJoyCameraBlendType InBlendViewWay)
{
	if (BlendViewType == InBlendViewWay)
	{
		ResetBlendViewWay();
	}
}

void AJoyPlayerCameraManager::ProcessViewRotation(float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot)
{
	/**
	 * @param OutViewRotation: 世界坐标系下的 Control Rotation
	 * @param OutDelta: 增量
	 */

	SCOPE_CYCLE_COUNTER(STAT_Camera_ProcessViewRotation);
	const FRotator OldViewRotation = OutViewRotation;

	for (int32 ModifierIdx = 0; ModifierIdx < ModifierList.Num(); ModifierIdx++)
	{
		if (ModifierList[ModifierIdx] != NULL && !ModifierList[ModifierIdx]->IsDisabled())
		{
			if (ModifierList[ModifierIdx]->ProcessViewRotation(
				ViewTarget.Target, DeltaTime, OutViewRotation, OutDeltaRot))
			{
				break;
			}
		}
	}

	const auto* GravityManager = UJoyGravityManageSubsystem::Get(GetWorld());
	const FVector ViewPlaneZ = GravityManager != nullptr ? GravityManager->GetGravitySpaceZ() : FVector(0.f, 0.f, 1.0f);

	if (!OutDeltaRot.IsZero() && GravityManager)
	{
		const FRotator LocalViewRotation =
			UKismetMathLibrary::ComposeRotators(OutViewRotation, GravityManager->GetInverseGravitySpaceTransform());
		OutViewRotation = UKismetMathLibrary::ComposeRotators(
			LocalViewRotation + OutDeltaRot, GravityManager->GetGravitySpaceTransform());
	}

	if (OutViewRotation != OldViewRotation)
	{
		if (!ViewPlaneZ.IsZero())
		{
			// Limit the player's view pitch only

			// Obtain current view orthonormal axes
			FVector ViewRotationX, ViewRotationY, ViewRotationZ;
			FRotationMatrix(OutViewRotation).GetUnitAxes(ViewRotationX, ViewRotationY, ViewRotationZ);

			// Obtain angle (with sign) between current view Z vector and plane normal
			float PitchAngle = FMath::RadiansToDegrees(FMath::Acos(ViewRotationZ | ViewPlaneZ));
			if ((ViewRotationX | ViewPlaneZ) < 0.0f)
			{
				PitchAngle *= -1.0f;
			}

			if (PitchAngle > ViewPitchMax)
			{
				// Make quaternion from zero pitch
				FQuat ViewRotation(FRotationMatrix::MakeFromZY(ViewPlaneZ, ViewRotationY));

				// Rotate 'up' with maximum pitch
				ViewRotation = FQuat(ViewRotationY, FMath::DegreesToRadians(-ViewPitchMax)) * ViewRotation;

				OutViewRotation = ViewRotation.Rotator();
			}
			else if (PitchAngle < ViewPitchMin)
			{
				// Make quaternion from zero pitch
				FQuat ViewRotation(FRotationMatrix::MakeFromZY(ViewPlaneZ, ViewRotationY));

				// Rotate 'down' with minimum pitch
				ViewRotation = FQuat(ViewRotationY, FMath::DegreesToRadians(-ViewPitchMin)) * ViewRotation;

				OutViewRotation = ViewRotation.Rotator();
			}

			if (bLimitYawAngle)
			{
				LimitViewYaw(OutViewRotation, ViewYawMin, ViewYawMax);
			}
		}
		else
		{
			// Limit player view axes
			LimitViewPitch(OutViewRotation, ViewPitchMin, ViewPitchMax);
			LimitViewYaw(OutViewRotation, ViewYawMin, ViewYawMax);
			LimitViewRoll(OutViewRotation, ViewRollMin, ViewRollMax);
		}
	}
}

UJoyCameraModifierController* AJoyPlayerCameraManager::GetCameraModifier(AActor* InActor)
{
	return MultiViewTargetCameraManager.GetCameraModifier(InActor);
}

TObjectPtr<class UJoyCameraModifierController> FMultiViewTargetCameraManager::GetCameraModifier(AActor* InViewTarget)
{
	if (InViewTarget != nullptr && ViewTargetCameraInfos.Contains(InViewTarget))
	{
		return ViewTargetCameraInfos[InViewTarget].CameraModifierController;
	}

	return nullptr;
}

bool FMultiViewTargetCameraManager::ContainsViewTarget(const AActor* InViewTarget) const
{
	return InViewTarget != nullptr && ViewTargetCameraInfos.Contains(InViewTarget);
}

void FMultiViewTargetCameraManager::AddViewTarget(
	class AJoyPlayerCameraManager* CameraManager, AActor* InViewTarget, const FVirtualCamera& VirtualCamera)
{
	if (InViewTarget == nullptr || ContainsViewTarget(InViewTarget) || CameraManager == nullptr)
	{
		return;
	}

	ViewTargetCameraInfos.Add(InViewTarget, FViewTargetCameraInfo());
	FViewTargetCameraInfo& CameraInfo = ViewTargetCameraInfos[InViewTarget];

	// 初始化 CameraInfo.CurrentCamera
	CameraInfo.CurrentCamera.CopyCamera(VirtualCamera);
	// 初始化 LastCamera、CurrentCamera、DesiredCamera
	CameraInfo.LastCamera.CopyCamera(VirtualCamera);
	CameraInfo.DesiredCamera.CopyCamera(VirtualCamera);
	CameraInfo.CameraModifierController = NewObject<UJoyCameraModifierController>(CameraManager);
	check(CameraInfo.CameraModifierController);
	CameraInfo.CameraModifierController->InitializeFor(CameraManager);
	CameraInfo.CameraModifierController->SetModifyTarget(InViewTarget);
}

void AJoyPlayerCameraManager::SetArmPitchInputEnabled(bool bEnabled)
{
	InputOverrideDescription.BlockArmPitchCounter += (bEnabled ? -1 : 1);
}

void AJoyPlayerCameraManager::SetArmYawInputEnabled(bool bEnabled)
{
	InputOverrideDescription.BlockArmYawCounter += (bEnabled ? -1 : 1);
}

void AJoyPlayerCameraManager::SetArmLengthInputEnabled(bool bEnabled)
{
	InputOverrideDescription.BlockArmLengthCounter += (bEnabled ? -1 : 1);
}

bool AJoyPlayerCameraManager::BlockLookMoveInput_Implementation(
	UObject* InputReceiver, const FInputActionValue& InputActionValue)
{
	return InputOverrideDescription.BlockArmPitchCounter > 0 || InputOverrideDescription.BlockArmYawCounter > 0;
}

bool AJoyPlayerCameraManager::BlockMouseScrollInput_Implementation(UObject* InputReceiver,
	const FInputActionValue& InputActionValue)
{
	return InputOverrideDescription.BlockArmLengthCounter > 0;
}

void AJoyPlayerCameraManager::SetConfigs(const TMap<EJoyCameraBasic, float>& Config)
{
	// 基础镜头设置
	UPDATE_BASIC_CONFIGS(BaseArmLength);
	UPDATE_BASIC_CONFIGS(MinArmLength);
	UPDATE_BASIC_CONFIGS(MaxArmLength);
	UPDATE_BASIC_CONFIGS(OverlayArmLength);
	UPDATE_BASIC_CONFIGS(BaseFov);
	UPDATE_BASIC_CONFIGS(ArmCenterOffsetX);
	UPDATE_BASIC_CONFIGS(ArmCenterOffsetY);
	UPDATE_BASIC_CONFIGS(ArmCenterOffsetZ);
	UPDATE_BASIC_CONFIGS(ViewTargetSwitchTime);
	UPDATE_BASIC_CONFIGS(ArmCenterLagRecoverSpeed);
	UPDATE_BASIC_CONFIGS(ArmCenterLagMaxHorizontalDistance);
	UPDATE_BASIC_CONFIGS(ArmCenterLagMaxVerticalDistance);

	UPDATE_BASIC_CONFIGS(MinArmPitch);
	ViewPitchMin = MinArmPitch;
	UPDATE_BASIC_CONFIGS(MaxArmPitch);
	ViewPitchMax = MaxArmPitch;

	UPDATE_BASIC_CONFIGS(ArmRotationLagRecoverSpeed);
}

void AJoyPlayerCameraManager::ApplyConfig()
{
	if (CameraConfigController)
	{
		bEnableCameraLag = CameraConfigController->bArmCenterLagEnable;
		FadingTarget_ArmPitch = CameraConfigController->FadeTargetArmPitch;
	}

	for (TMap<TWeakObjectPtr<AActor>, FViewTargetCameraInfo>::TIterator ItViewTarget =
		     MultiViewTargetCameraManager.ViewTargetCameraInfos.CreateIterator();
	     ItViewTarget; ++ItViewTarget)
	{
		TWeakObjectPtr<AActor> CachedViewTarget = ItViewTarget.Key();
		FViewTargetCameraInfo& CachedCameraInfo = ItViewTarget.Value();

		if (CachedViewTarget.Get() == nullptr)
		{
			ItViewTarget.RemoveCurrent();
			continue;
		}

		// 将当前基础相机参数更新到缓存的 ViewTarget 上
		CachedCameraInfo.DesiredCamera.ArmLength = BaseArmLength + OverlayArmLength;
		CachedCameraInfo.DesiredCamera.MinArmLength = MinArmLength;
		CachedCameraInfo.DesiredCamera.MaxArmLength = MaxArmLength;
		CachedCameraInfo.DesiredCamera.SetArmCenterOffset(
			FVector(ArmCenterOffsetX, ArmCenterOffsetY, ArmCenterOffsetZ));
		CachedCameraInfo.DesiredCamera.LocalArmCenterOffset =
			FVector(ArmCenterOffsetX, ArmCenterOffsetY, ArmCenterOffsetZ);
		CachedCameraInfo.DesiredCamera.WorldArmOffsetAdditional = FVector::ZeroVector;
		CachedCameraInfo.DesiredCamera.Fov = BaseFov;
	}
}

void AJoyPlayerCameraManager::StartFade(float InFadeDuration, bool bFadeArmLengthRange, bool bInFadeArmLength,
	bool bInFadeArmRotationPitch, bool bInFadeArmRotationYaw, bool bInFadeLocalArmCenterOffset, bool bInFadeFov,
	bool bIgnoreTimeDilationDuringFading, bool bOverrideCameraInput)
{
	for (TMap<TWeakObjectPtr<AActor>, FViewTargetCameraInfo>::TIterator ItViewTarget =
		     MultiViewTargetCameraManager.ViewTargetCameraInfos.CreateIterator();
	     ItViewTarget; ++ItViewTarget)
	{
		TWeakObjectPtr<AActor> FadeViewTarget = ItViewTarget.Key();
		FViewTargetCameraInfo& FadeCameraInfo = ItViewTarget.Value();

		if (FadeViewTarget.Get() == nullptr)
		{
			ItViewTarget.RemoveCurrent();
			continue;
		}

		// @TODO 如果在 Fade 期间同时进行了 CameraModifier，表现会异常，因为这两套机制同时写入了 LastCamera 变量
		FadeCameraInfo.LastCamera.CopyCamera(FadeCameraInfo.CurrentCamera);
		FadeCameraInfo.bFadeArmLengthRange = bFadeArmLengthRange;
		FadeCameraInfo.bFadeArmLength = bInFadeArmLength;
		FadeCameraInfo.bFadeArmPitch = bInFadeArmRotationPitch;
		FadeCameraInfo.bFadeArmYaw = bInFadeArmRotationYaw;
		FadeCameraInfo.bFadeLocalArmCenterOffset = bInFadeLocalArmCenterOffset;

		FadeCameraInfo.bNeedFading = true;
		FadeCameraInfo.bFadeCameraFov = bInFadeFov;
	}

	if (CameraConfigFadingDescription.bDuringFading)
	{
		// 中断上一段 Fading 过程
		EndCurrentCameraFadingProcess();
	}

	CameraConfigFadingDescription.Duration = InFadeDuration;
	CameraConfigFadingDescription.ElapseTime = 0;
	CameraConfigFadingDescription.bDuringFading = true;
	CameraConfigFadingDescription.bIgnoreTimeDilation = bIgnoreTimeDilationDuringFading;
	CameraConfigFadingDescription.bOverrideCameraInput = bOverrideCameraInput;

	if (CameraConfigFadingDescription.bOverrideCameraInput)
	{
		SetArmPitchInputEnabled(false);
		SetArmYawInputEnabled(false);
	}
}
