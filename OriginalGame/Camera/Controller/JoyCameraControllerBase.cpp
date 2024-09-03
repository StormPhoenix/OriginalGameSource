// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyCameraControllerBase.h"

#include "JoyLogChannels.h"

void UJoyCameraControllerBase::UpdateInternal(float DeltaSeconds)
{
}

void UJoyCameraControllerBase::UpdateDeactivateInternal(float DeltaSeconds)
{
}

void UJoyCameraControllerBase::Update(float DeltaSeconds)
{
	if (bIsActive)
	{
		UpdateInternal(DeltaSeconds);
	}
	else
	{
		UpdateDeactivateInternal(DeltaSeconds);
	}
}

void UJoyCameraControllerBase::InitializeFor(AJoyPlayerCameraManager* PCMgr)
{
	if (PCMgr == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("%s ::InitializeFor() failed, PCM is nullptr !!!"), *GetName());
		return;
	}

	PCM = PCMgr;
}

void UJoyCameraControllerBase::Lock(UJoyCameraControllerBase* Other)
{
	OnDisable();
}

void UJoyCameraControllerBase::Unlock(UJoyCameraControllerBase* Other)
{
	OnEnable();
}

AJoyPlayerCameraManager* UJoyCameraControllerBase::GetPlayerCameraManager() const
{
	return PCM;
}

void UJoyCameraControllerBase::OnEnable()
{
	if (!bIsActive)
	{
		// @Comment 加上条件判断避免多次触发
		bIsActive = true;
	}
}

void UJoyCameraControllerBase::OnDisable()
{
	if (bIsActive)
	{
		// @Comment 加上条件判断避免多次触发
		bIsActive = false;
	}
}
