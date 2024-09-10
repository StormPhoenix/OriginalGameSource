#include "JoyCameraComponent.h"

#include "CameraMode/JoyCameraMode.h"
#include "CameraMode/JoyCameraModeStack.h"

FJoyCameraIDHandle::FJoyCameraIDHandle(int64 Seq) : SequenceID(Seq)
{
}

UJoyCameraComponent::UJoyCameraComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UJoyCameraComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UJoyCameraComponent::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	check(CameraModeStack);
	CameraModeStack->GetBlendInfo(/*out*/ OutWeightOfTopLayer, /*out*/ OutTagOfTopLayer);
}

void UJoyCameraComponent::OnRegister()
{
	Super::OnRegister();

	if (!CameraModeStack)
	{
		CameraModeStack = NewObject<UJoyCameraModeStack>(this);
		check(CameraModeStack);
	}
}

void UJoyCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	UpdateCameraModes();

	if (!bCameraFrozen)
	{
		FJoyCameraModeView CameraModeView;
		CameraModeStack->EvaluateStack(DeltaTime, CameraModeView);
		SetWorldLocationAndRotation(CameraModeView.Location, CameraModeView.Rotation);
		FieldOfView = CameraModeView.FieldOfView;

		// Fill in desired view.
		DesiredView.Location = CameraModeView.Location;
		DesiredView.Rotation = CameraModeView.Rotation;
		DesiredView.FOV = CameraModeView.FieldOfView;
		DesiredView.OrthoWidth = OrthoWidth;
		DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
		DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;
		DesiredView.AspectRatio = AspectRatio;
		DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
		DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
		DesiredView.ProjectionMode = ProjectionMode;

		// See if the CameraActor wants to override the PostProcess settings used.
		DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
		if (PostProcessBlendWeight > 0.0f)
		{
			DesiredView.PostProcessSettings = PostProcessSettings;
		}

		FrozenCameraView = DesiredView;
	}
	else
	{
		DesiredView = FrozenCameraView;
		SetWorldLocationAndRotation(DesiredView.Location, DesiredView.Rotation);
	}

	if (IsXRHeadTrackedCamera())
	{
		// In XR much of the camera behavior above is irrellevant, but the post process settings are not.
		Super::GetCameraView(DeltaTime, DesiredView);
	}
}

void UJoyCameraComponent::UpdateCameraModes()
{
	check(CameraModeStack);

	if (CameraModeStack->IsStackActivate())
	{
		CameraModeStack->UpdateCameraStack();
	}
}

void UJoyCameraComponent::PushCameraMode(TSubclassOf<UJoyCameraMode> CameraModeClass, bool bBlend) const
{
	check(CameraModeStack);
	CameraModeStack->AddCameraMode(CameraModeClass, bBlend);
}

void UJoyCameraComponent::PopCameraMode(TSubclassOf<UJoyCameraMode> CameraModeClass, bool bBlend) const
{
	check(CameraModeStack);
	CameraModeStack->RemoveCameraMode(CameraModeClass, bBlend);
}

UJoyCameraMode* UJoyCameraComponent::GetCameraModeInstance(TSubclassOf<UJoyCameraMode> GameModeClass) const
{
	check(CameraModeStack);
	return CameraModeStack->GetCameraModeInstance(GameModeClass);
}

TSubclassOf<UJoyCameraMode> UJoyCameraComponent::GetTopCameraModeClass() const
{
	if (UJoyCameraMode* GM = CameraModeStack->GetTopCameraMode())
	{
		return GM->GetClass();
	}

	return UJoyCameraMode::StaticClass();
}

void UJoyCameraComponent::FrozeCamera()
{
	bCameraFrozen = true;
}

const FMinimalViewInfo& UJoyCameraComponent::GetFrozenView() const
{
	return FrozenCameraView;
}

FJoyCameraIDHandle UJoyCameraComponent::PushCameraConfig(FName CameraID)
{
	if (!CameraIDs.Contains(CameraID))
	{
		bIsCameraConfigDirty = true;
		CameraIDs.Add(CameraID);

		SequenceNumber++;
		FJoyAddCameraIDRequestCache& NewRequest = CameraIDRequestQueue.Emplace_GetRef();
		NewRequest.Handle = FJoyCameraIDHandle(SequenceNumber);
		NewRequest.CameraID = CameraID;
		return NewRequest.Handle;
	}

	return FJoyCameraIDHandle(0);
}

bool UJoyCameraComponent::RemoveCameraConfig(FJoyCameraIDHandle Handler)
{
	if (!Handler.IsValid())
	{
		return false;
	}

	const int32 Index = CameraIDRequestQueue.IndexOfByKey(Handler);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	const FName CameraID = CameraIDRequestQueue[Index].CameraID;
	bIsCameraConfigDirty = true;
	CameraIDs.Remove(CameraID);

	CameraIDRequestQueue.RemoveAt(Index);
	return true;
}

bool UJoyCameraComponent::RemoveCameraConfigByID(FName CameraID)
{
	if (CameraID != NAME_None && CameraIDs.Contains(CameraID))
	{
		int32 Index = 0;
		for (const auto& CacheRequest : CameraIDRequestQueue)
		{
			if (CacheRequest.CameraID == CameraID)
			{
				break;
			}

			Index++;
		}

		if (Index >= CameraIDRequestQueue.Num())
		{
			return false;
		}

		bIsCameraConfigDirty = true;
		CameraIDRequestQueue.RemoveAt(Index);
		CameraIDs.Remove(CameraID);

		return true;
	}

	return false;
}

TArray<FName> UJoyCameraComponent::GetCameraIDs() const
{
	TArray<FName> OutResult{};
	for (const auto& CacheRequest : CameraIDRequestQueue)
	{
		OutResult.Add(CacheRequest.CameraID);
	}

	return OutResult;
}

bool UJoyCameraComponent::IsCameraConfigDirty() const
{
	return bIsCameraConfigDirty;
}

void UJoyCameraComponent::RefreshCameraConfig()
{
	bIsCameraConfigDirty = false;
}