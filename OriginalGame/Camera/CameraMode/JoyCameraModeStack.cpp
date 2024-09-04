// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyCameraModeStack.h"

#include "Gameplay/TimeDilation/JoyTimeDilationManageSubsystem.h"
#include "JoyCameraMode.h"
#include "JoyCameraMode_ThirdPerson.h"
#include "Kismet/GameplayStatics.h"

UJoyCameraModeStack::UJoyCameraModeStack()
{
	bIsActive = true;
}

void UJoyCameraModeStack::ActivateStack()
{
	if (!bIsActive)
	{
		bIsActive = true;

		// Notify camera modes that they are being activated.
		for (UJoyCameraMode* CameraMode : CameraModeStack)
		{
			check(CameraMode);
			CameraMode->OnActivation();
		}
	}
}

void UJoyCameraModeStack::UpdateCameraStack()
{
	/**
	 * 首先检查有哪些 CameraMode 需要被移除
	 * 针对 Lyra 设计的 CameraStack，处于栈内的 Camera Mode 会被自动移除，所以我们只负责处理栈顶的 Camera Mode，
	 * 同时，我们要保证 Camera Stack 内的 Camera Mode 数量要 >= 1，所以只处理当栈长度大于 1 的情况
	 */

	TArray<FCameraModeAction*> TempCMOperations;
	for (int i = 0; i < CameraModeOperations.Num(); i++)
	{
		const int NextID = i + 1;
		if (NextID < CameraModeOperations.Num() &&
			(CameraModeOperations[i].CameraModeClass == CameraModeOperations[NextID].CameraModeClass &&
				CameraModeOperations[i].OP == ECameraModePushPopOption::Push &&
				CameraModeOperations[NextID].OP == ECameraModePushPopOption::Pop))
		{
			i += 1;
			continue;
		}

		TempCMOperations.Add(&CameraModeOperations[i]);
	}

	for (FCameraModeAction* CM : TempCMOperations)
	{
		if (CM->OP == ECameraModePushPopOption::Push)
		{
			PushCameraMode(CM->CameraModeClass, CM->bBlending);
		}
		else if (GetCameraStackNum() >= 1)
		{
			const UJoyCameraMode* TopCameraMode = GetTopCameraMode();
			if (TopCameraMode->GetClass() == CM->CameraModeClass)
			{
				RemoveTopCameraMode();
			}
		}
	}

	// 检查 CameraMode 堆栈中是否为空
	if (GetCameraStackNum() == 0)
	{
		PushCameraMode(GetDefaultCameraModeClass(), true);
	}

	CameraModeOperations.Empty();
}

void UJoyCameraModeStack::DeactivateStack()
{
	if (bIsActive)
	{
		bIsActive = false;

		// Notify camera modes that they are being deactivated.
		for (UJoyCameraMode* CameraMode : CameraModeStack)
		{
			check(CameraMode);
			CameraMode->OnDeactivation();
		}
	}
}

int32 UJoyCameraModeStack::GetCameraStackNum()
{
	return CameraModeStack.Num();
}

UJoyCameraMode* UJoyCameraModeStack::GetTopCameraMode()
{
	if (GetCameraStackNum() > 0)
	{
		return CameraModeStack[0];
	}

	return nullptr;
}

void UJoyCameraModeStack::RemoveTopCameraMode()
{
	if (CameraModeStack.Num() > 0)
	{
		if (const auto CameraMode = CameraModeStack[0]; CameraMode.Get())
		{
			CameraMode->OnDeactivation();
		}

		CameraModeStack.RemoveAt(0);
	}
}

void UJoyCameraModeStack::PushCameraMode(TSubclassOf<UJoyCameraMode> CameraModeClass, bool bBlend)
{
	if (!CameraModeClass)
	{
		return;
	}

	UJoyCameraMode* CameraMode = GetCameraModeInstance(CameraModeClass);
	check(CameraMode);

	int32 StackSize = CameraModeStack.Num();

	if ((StackSize > 0) && (CameraModeStack[0] == CameraMode))
	{
		// Already top of stack.
		return;
	}

	// See if it's already in the stack and remove it.
	// Figure out how much it was contributing to the stack.
	int32 ExistingStackIndex = INDEX_NONE;
	float ExistingStackContribution = 1.0f;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		if (CameraModeStack[StackIndex] == CameraMode)
		{
			ExistingStackIndex = StackIndex;
			ExistingStackContribution *= CameraMode->GetBlendWeight();
			break;
		}
		else
		{
			ExistingStackContribution *= (1.0f - CameraModeStack[StackIndex]->GetBlendWeight());
		}
	}

	if (ExistingStackIndex != INDEX_NONE)
	{
		CameraModeStack.RemoveAt(ExistingStackIndex);
		StackSize--;
	}
	else
	{
		ExistingStackContribution = 0.0f;
	}

	// Decide what initial weight to start with.
	const bool bShouldBlend = ((CameraMode->GetBlendTime() > 0.0f) && (StackSize > 0) && bBlend);
	const float BlendWeight = (bShouldBlend ? ExistingStackContribution : 1.0f);

	CameraMode->SetBlendWeight(BlendWeight);

	// Add new entry to top of stack.
	CameraModeStack.Insert(CameraMode, 0);

	// Make sure stack bottom is always weighted 100%.
	CameraModeStack.Last()->SetBlendWeight(1.0f);

	// Let the camera mode know if it's being added to the stack.
	if (ExistingStackIndex == INDEX_NONE)
	{
		CameraMode->OnActivation();
	}
}

bool UJoyCameraModeStack::EvaluateStack(float DeltaTime, FJoyCameraModeView& OutCameraModeView)
{
	if (!bIsActive)
	{
		return false;
	}

	// Ignore time dilation
	if (const auto* TimeDilationSystem = UJoyTimeDilationManageSubsystem::Get(GetWorld()))
	{
		DeltaTime = DeltaTime / TimeDilationSystem->GetGlobalTimeDilation();
	}

	const int RemoveCount = UpdateStack(DeltaTime);
	BlendStack(OutCameraModeView, RemoveCount);

	return true;
}

UJoyCameraMode* UJoyCameraModeStack::GetCameraModeInstance(TSubclassOf<UJoyCameraMode> CameraModeClass)
{
	check(CameraModeClass);

	// First see if we already created one.
	for (UJoyCameraMode* CameraMode : CameraModeInstances)
	{
		if ((CameraMode != nullptr) && (CameraMode->GetClass() == CameraModeClass))
		{
			return CameraMode;
		}
	}

	// Not found, so we need to create it.
	UJoyCameraMode* NewCameraMode = NewObject<UJoyCameraMode>(GetOuter(), CameraModeClass, NAME_None, RF_NoFlags);
	check(NewCameraMode);

	CameraModeInstances.Add(NewCameraMode);

	return NewCameraMode;
}

int UJoyCameraModeStack::UpdateStack(float DeltaTime)
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0)
	{
		return 0;
	}

	int32 RemoveCount = 0;
	int32 RemoveIndex = INDEX_NONE;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		UJoyCameraMode* CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		CameraMode->UpdateCameraMode(DeltaTime);

		if (CameraMode->GetBlendWeight() >= 1.0f)
		{
			// Everything below this mode is now irrelevant and can be removed.
			RemoveIndex = (StackIndex + 1);
			RemoveCount = (StackSize - RemoveIndex);
			break;
		}
	}

	return RemoveCount;
}

void UJoyCameraModeStack::BlendStack(FJoyCameraModeView& OutCameraModeView, const int RemoveCount) const
{
	const int32 StackSize = CameraModeStack.Num() - RemoveCount;
	if (StackSize <= 0)
	{
		return;
	}

	// Start at the bottom and blend up the stack
	const UJoyCameraMode* CameraMode = CameraModeStack[StackSize - 1];
	check(CameraMode);

	OutCameraModeView = CameraMode->GetCameraModeView();

	for (int32 StackIndex = (StackSize - 2); StackIndex >= 0; --StackIndex)
	{
		CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		OutCameraModeView.Blend(CameraMode->GetCameraModeView(), CameraMode->GetBlendWeight());
	}
}

void UJoyCameraModeStack::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	if (CameraModeStack.Num() == 0)
	{
		OutWeightOfTopLayer = 1.0f;
		OutTagOfTopLayer = FGameplayTag();
		return;
	}
	else
	{
		UJoyCameraMode* TopEntry = CameraModeStack.Last();
		check(TopEntry);
		OutWeightOfTopLayer = TopEntry->GetBlendWeight();
		OutTagOfTopLayer = TopEntry->GetCameraTypeTag();
	}
}

TSubclassOf<UJoyCameraMode> UJoyCameraModeStack::GetDefaultCameraModeClass()
{
	return UJoyCameraMode_ThirdPerson::StaticClass();
}

void UJoyCameraModeStack::AddCameraMode(TSubclassOf<UJoyCameraMode> CameraModeClass, bool bBlend)
{
	FCameraModeAction CameraModeEntry;
	CameraModeEntry.CameraModeClass = CameraModeClass;
	CameraModeEntry.bBlending = bBlend;
	CameraModeEntry.OP = ECameraModePushPopOption::Push;
	CameraModeOperations.Add(CameraModeEntry);
}

void UJoyCameraModeStack::RemoveCameraMode(TSubclassOf<UJoyCameraMode> CameraModeClass, bool bBlend)
{
	FCameraModeAction CameraModeEntry;
	CameraModeEntry.CameraModeClass = CameraModeClass;
	CameraModeEntry.bBlending = bBlend;
	CameraModeEntry.OP = ECameraModePushPopOption::Pop;
	CameraModeOperations.Add(CameraModeEntry);
}
