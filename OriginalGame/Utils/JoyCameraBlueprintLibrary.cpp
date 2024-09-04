// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyCameraBlueprintLibrary.h"

#include "JoyGameBlueprintLibrary.h"
#include "Camera/JoyCameraComponent.h"
#include "Camera/JoyPlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Player/JoyPlayerController.h"


void UJoyCameraBlueprintLibrary::ApplyCameraSettings_Immediately(
	AActor* OwnerActor, FCameraModifiers const& CameraModifiers)
{
	if (OwnerActor == nullptr)
	{
		return;
	}

	UJoyCameraModifierController* Modifier = nullptr;
	if (const auto* PlayerController = UJoyGameBlueprintLibrary::GetJoyPlayerController(OwnerActor->GetWorld()))
	{
		if (auto* PlayerCameraManager = Cast<AJoyPlayerCameraManager>(PlayerController->PlayerCameraManager))
		{
			PlayerCameraManager->AddNewViewTarget(OwnerActor);
			Modifier = PlayerCameraManager->GetCameraModifier(OwnerActor);
		}
	}

	if (Modifier == nullptr)
	{
		return;
	}

	Modifier->ApplyCameraModify_Immediately(CameraModifiers);
}

FCameraModifyHandle UJoyCameraBlueprintLibrary::ApplyCameraSettings(AActor* OwnerActor, float Duration,
                                                                    float BlendInTime, float BlendOutTime,
                                                                    FCameraModifiers const& CameraModifiers,
                                                                    bool bNeedManualBreak)
{
	if (OwnerActor == nullptr)
	{
		return FCameraModifyHandle(0);
	}

	UJoyCameraModifierController* Modifier = nullptr;
	const UWorld* OwnerWorld = OwnerActor->GetWorld();
	if (const auto* PlayerController = Cast<AJoyPlayerController>(OwnerWorld->GetFirstPlayerController()))
	{
		// 获取相机管理器
		if (auto* PlayerCameraManager = Cast<AJoyPlayerCameraManager>(PlayerController->PlayerCameraManager))
		{
			PlayerCameraManager->AddNewViewTarget(OwnerActor);
			Modifier = PlayerCameraManager->GetCameraModifier(OwnerActor);
		}
	}

	if (Modifier == nullptr)
	{
		return FCameraModifyHandle(0);
	}

	return Modifier->ApplyCameraModify(Duration, BlendInTime, BlendOutTime, CameraModifiers, bNeedManualBreak);
}

void UJoyCameraBlueprintLibrary::ManualBreakCameraModifier(AActor* OwnerActor, FCameraModifyHandle ModifyHandler)
{
	if (OwnerActor == nullptr)
	{
		return;
	}

	UJoyCameraModifierController* Modifier = nullptr;
	if (AJoyPlayerCameraManager* CameraManager = GetJoyPlayerCameraManager(OwnerActor->GetWorld()))
	{
		Modifier = CameraManager->GetCameraModifier(OwnerActor);
	}

	if (Modifier == nullptr)
	{
		return;
	}

	Modifier->BreakModifier(ModifyHandler);
}

bool UJoyCameraBlueprintLibrary::CheckTargetInsideScreen(const UObject* WorldContextObject, const AActor* Target)
{
	if (Target == nullptr)
	{
		return false;
	}

	const auto* ThisWorld = WorldContextObject->GetWorld();
	if (ThisWorld == nullptr)
	{
		return false;
	}

	if (const APlayerController* PlayerController = ThisWorld->GetFirstPlayerController())
	{
		int32 ViewportSizeX, ViewportSizeY;
		PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

		FVector2D TargetScreenPosition;
		if (UGameplayStatics::ProjectWorldToScreen(PlayerController, Target->GetActorLocation(), TargetScreenPosition))
		{
			return TargetScreenPosition.X >= 0 && TargetScreenPosition.X <= ViewportSizeX &&
				TargetScreenPosition.Y >= 0 && TargetScreenPosition.Y <= ViewportSizeY;
		}
	}

	return false;
}

void UJoyCameraBlueprintLibrary::GetCameraViewFromTarget(
	AActor* Target, FMinimalViewInfo& OutView, bool bRealTime, float DeltaTime)
{
	if (Target)
	{
		if (const auto* JoyCamera = Target->GetComponentByClass<UJoyCameraComponent>())
		{
			if (bRealTime)
			{
				Target->CalcCamera(DeltaTime, OutView);
			}
			else
			{
				OutView = JoyCamera->GetFrozenView();
			}
		}
		else
		{
			if (bRealTime)
			{
				Target->CalcCamera(DeltaTime, OutView);
			}
			else
			{
				Target->CalcCamera(0, OutView);
			}
		}
	}
}

AJoyPlayerCameraManager* UJoyCameraBlueprintLibrary::GetJoyPlayerCameraManager(const UObject* WorldContextObject)
{
	if (const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		return Cast<AJoyPlayerCameraManager>(PlayerController->PlayerCameraManager);
	}

	return nullptr;
}
