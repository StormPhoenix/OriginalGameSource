// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyCameraMode_PlayerSwitching.h"

#include "Character/JoyCharacter.h"
#include "Gameplay/JoyCharacterControlManageSubsystem.h"
#include "JoyGameBlueprintLibrary.h"
#include "Player/JoyPlayerController.h"

UJoyCameraMode_PlayerSwitching::UJoyCameraMode_PlayerSwitching()
{
}

float GetAngleDiff(float A, float B)
{
	// 计算两个角度之间的差值
	float AngleDiff = FMath::Abs(A - B);
	if (AngleDiff > 180.0f)
	{
		AngleDiff = 360.0f - AngleDiff;
	}
	return AngleDiff;
}

float AddAngleBy(float A, float Delta)
{
	// 计算两个角度相加的值
	float Angle = A + Delta;
	if (Angle > 360.0f)
	{
		Angle -= 360.0f;
	}
	else if (Angle < 0.0f)
	{
		Angle += 360.0f;
	}
	return Angle;
}

FRotator ComputeNearestRotation(FVector CenterDirection, float WindowTolerance, FRotator RefRotation)
{
	if (WindowTolerance < 0)
	{
		WindowTolerance = 0;
	}

	FRotator CenterR = CenterDirection.Rotation();
	float CenterR_Yaw = CenterR.Yaw;
	float LeftWindowEdge_Yaw = AddAngleBy(CenterR_Yaw, WindowTolerance * 0.5);
	float RightWindowEdge_Yaw = AddAngleBy(CenterR_Yaw, -WindowTolerance * 0.5);

	float RefR_Yaw = RefRotation.Yaw;
	float DistEdge_Yaw = 0;
	if (GetAngleDiff(LeftWindowEdge_Yaw, RefR_Yaw) < GetAngleDiff(RightWindowEdge_Yaw, RefR_Yaw))
	{
		DistEdge_Yaw = LeftWindowEdge_Yaw;
	}
	else
	{
		DistEdge_Yaw = RightWindowEdge_Yaw;
	}

	CenterR.Yaw = DistEdge_Yaw;
	CenterR.Pitch = RefRotation.Pitch;
	return CenterR;
}

FRotator UJoyCameraMode_PlayerSwitching::CalcPivotRotation() const
{
	const auto* JoyPlayerController = UJoyGameBlueprintLibrary::GetJoyPlayerController(GetWorld());
	const AJoyCharacter* TargetCharacter = Cast<AJoyCharacter>(GetTargetActor());
	const auto* ControlManageSubsystem = UJoyCharacterControlManageSubsystem::Get(GetWorld());
	if (TargetCharacter == nullptr || ControlManageSubsystem == nullptr || JoyPlayerController == nullptr)
	{
		return Super::CalcPivotRotation();
	}

	if (ControlManageSubsystem->GetSwitchTargetCharacter() == TargetCharacter)
	{
		return JoyPlayerController->GetControlRotation();
	}

	return Super::CalcPivotRotation();
}

void UJoyCameraMode_PlayerSwitching::UpdateView(float DeltaTime)
{
	Super::UpdateView(DeltaTime);
}

void UJoyCameraMode_PlayerSwitching::PreventCameraPenetration(AActor const& ViewTarget, FVector const& SafeLoc,
	FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly)
{
	Super::PreventCameraPenetration(ViewTarget, SafeLoc, CameraLoc, DeltaTime, DistBlockedPct, bSingleRayOnly);
}

void UJoyCameraMode_PlayerSwitching::OnActivation()
{
	Super::OnActivation();
}
