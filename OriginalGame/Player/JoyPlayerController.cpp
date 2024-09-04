// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyPlayerController.h"

#include "Camera/CameraMode/JoyCameraMode_PlayerSwitching.h"
#include "Camera/JoyCameraComponent.h"
#include "Camera/JoyPlayerCameraManager.h"
#include "Character/JoyCharacter.h"
#include "JoyPlayerBotController.h"
#include "JoyPlayerState.h"
#include "Utils/JoyCameraBlueprintLibrary.h"

AJoyPlayerController::AJoyPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

AJoyPlayerState* AJoyPlayerController::GetJoyPlayerState() const
{
	return CastChecked<AJoyPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

void AJoyPlayerController::SwitchCharacter(
	AJoyCharacter* PreviousCharacter, AJoyCharacter* TargetCharacter, FJoyCharacterSwitchExtraParam ExtraParam)
{
	if (CheckDuringCharacterSwitching())
	{
		return;
	}

	AController* PreviousController = PreviousCharacter != nullptr ? PreviousCharacter->GetController() : nullptr;
	AController* NextController = TargetCharacter != nullptr ? TargetCharacter->GetController() : nullptr;

	bDuringPlayerSwitching = true;
	ApplyTimeDilation(TimeDilationOfCharacterSwitching);

	CharacterSwitchSpec.From = PreviousCharacter;
	CharacterSwitchSpec.FromController = PreviousController;
	CharacterSwitchSpec.To = TargetCharacter;
	CharacterSwitchSpec.ToController = NextController;
	CharacterSwitchSpec.ExtraParam = ExtraParam;

	DoSwitchCharacter();
}

void AJoyPlayerController::ApplyTimeDilation(float TimeDilation)
{
	RemoveTimeDilation();
	if (auto* TimeSys = UJoyTimeDilationManageSubsystem::GetTimeDilationManageSubsystem(this))
	{
		TimeDilationHandle =
			TimeSys->AddGlobalTimeDilation(TimeDilation, TEXT("AJoyPlayerController::ApplyTimeDilation"));
	}
}

void AJoyPlayerController::RemoveTimeDilation()
{
	if (!TimeDilationHandle.IsValid())
	{
		return;
	}

	if (auto* TimeSys = UJoyTimeDilationManageSubsystem::GetTimeDilationManageSubsystem(this))
	{
		TimeSys->RemoveGlobalTimeDilation(TimeDilationHandle);
		TimeDilationHandle = {};
	}
}

void AJoyPlayerController::DoSwitchCharacter()
{
	if (CharacterSwitchSpec.From != nullptr)
	{
		CharacterSwitchSpec.From->bUseControllerRotationYaw = false;
		if (const auto* FromCamera = UJoyCameraComponent::FindCameraComponent(CharacterSwitchSpec.From.Get()))
		{
			FromCamera->PushCameraMode(UJoyCameraMode_PlayerSwitching::StaticClass(), true);
		}
	}

	if (CharacterSwitchSpec.ToController != nullptr && CharacterSwitchSpec.To != nullptr)
	{
		CharacterSwitchSpec.To->bUseControllerRotationYaw = false;
		const auto* ToCamera = UJoyCameraComponent::FindCameraComponent(CharacterSwitchSpec.To.Get());
		if (ToCamera && CharacterSwitchSpec.ExtraParam.BlendType != EJoyCameraBlendType::Default)
		{
			ToCamera->PushCameraMode(UJoyCameraMode_PlayerSwitching::StaticClass(), true);
		}
	}

	/**
	 * 必须等相机移动完毕之后才能通知 AI Controller 角色控制权变更了，
	 * 如果立即广播控制权变更消息：那么相机移动过程中视角会发生一次瞬间切换，这是因为当控制权
	 * 变更后，PlayerCameraManager 控制的 ViewTarget 的 CameraComponent 的 Rotaion 会被
	 * AI Controller 设置，而 AI Controller 与 Player Controller 的 Rotation 是不一样的，导致发生一次
	 * 瞬间切换
	 *
	 * 要延迟修改主控角色控制权，且必须现在(当前帧)设置 Timer，否则控制权的延迟修改时间会受到世界静止的影响
	 */

	if (auto* JoyCameraManager = Cast<AJoyPlayerCameraManager>(PlayerCameraManager))
	{
		if (CharacterSwitchSpec.To != nullptr)
		{
			if (CharacterSwitchSpec.ExtraParam.bImmediately)
			{
				SetViewTarget(CharacterSwitchSpec.To.Get());
				OnCharacterSwitchFinished(CharacterSwitchSpec.From.Get(), CharacterSwitchSpec.To.Get());
			}
			else
			{
				JoyCameraManager->SetBlendViewType(CharacterSwitchSpec.ExtraParam.BlendType);
				JoyCameraManager->OnViewTargetBlendComplete.AddUObject(
					this, &AJoyPlayerController::OnCharacterSwitchFinished);
				SetViewTargetWithBlend(CharacterSwitchSpec.To.Get(), TargetSwitchTime, VTBlend_Cubic);
			}
		}
	}
	else
	{
		CharacterSwitchSpec.Reset();
	}
}

void AJoyPlayerController::OnCharacterSwitchFinished(AActor* ViewTarget, AActor* PendingViewTarget)
{
	if (auto* JoyCameraManager = UJoyCameraBlueprintLibrary::GetJoyPlayerCameraManager(this))
	{
		JoyCameraManager->OnViewTargetBlendComplete.RemoveAll(this);
		JoyCameraManager->SetBlendViewType(EJoyCameraBlendType::Default);
	}

	if (ViewTarget == CharacterSwitchSpec.From && PendingViewTarget == CharacterSwitchSpec.To)
	{
		// @TODO 此处没有考虑角色切换中途，其它地方被设置了新的 PendingViewTarget 的情况
		OnPlayerTargetSwitchFinishedDelegate.Broadcast(CharacterSwitchSpec.From.Get(), CharacterSwitchSpec.To.Get());
	}

	CharacterSwitchSpec.Reset();
	RemoveTimeDilation();
}
