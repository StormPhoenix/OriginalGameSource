// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyMathBlueprintLibrary.h"

#include "Components/CapsuleComponent.h"

float UJoyMathBlueprintLibrary::Sin2D(const FVector& VecA, const FVector& VecB)
{
	FVector A = VecA.GetSafeNormal2D();
	FVector B = VecB.GetSafeNormal2D();
	return A[0] * B[1] - A[1] * B[0];
}

float UJoyMathBlueprintLibrary::LerpSin(const float A, const float B, const float BlendAlpha)
{
	const float NewBlendAlpha = FMath::Sin(UE_PI * 0.5 * BlendAlpha);
	return A * (1. - NewBlendAlpha) + B * NewBlendAlpha;
}

bool UJoyMathBlueprintLibrary::CheckLeftSide2D(const FVector& VecA, const FVector& VecB)
{
	return Sin2D(VecA, VecB) < 0;
}

bool UJoyMathBlueprintLibrary::CheckRightSide2D(const FVector& VecA, const FVector& VecB)
{
	return Sin2D(VecA, VecB) > 0;
}

float UJoyMathBlueprintLibrary::CalculateFrustumRotationPitch_ForReachTarget(const float Alpha,
	const float VertexCenterDistance, const FVector& RotateCenter, const FVector& ReachTarget,
	const FVector& OriginForward)
{
	const FVector CenterToTarget = ReachTarget - RotateCenter;
	const FVector ViewForward_XY = OriginForward.GetSafeNormal2D();

	const float DistanceX = FVector::DotProduct(ViewForward_XY, CenterToTarget);
	const float DistanceZ = CenterToTarget.Z;
	const FVector CenterToTarget_Proj = ViewForward_XY * DistanceX + FVector::UpVector * DistanceZ;
	const float Gamma = FMath::RadiansToDegrees(
		FMath::Acos(FVector::DotProduct(CenterToTarget_Proj.GetSafeNormal(), -OriginForward.GetSafeNormal())));

	const float Beta = FMath::RadiansToDegrees(
		FMath::Asin(FMath::Sin(FMath::DegreesToRadians(Alpha)) * VertexCenterDistance / CenterToTarget_Proj.Size()));

	const float ModifyPitch = 180. - (Beta + Alpha) - Gamma;
	if (CenterToTarget_Proj.GetSafeNormal().Z > OriginForward.Z)
	{
		return ModifyPitch;
	}
	else
	{
		return -ModifyPitch;
	}
}

float UJoyMathBlueprintLibrary::CalculateFrustumRotationYaw_ForReachTarget(const float FrustumHalfAngle,
	const float VertexCenterDistance, const FVector& RotateCenter, const FVector& ReachTarget,
	const FVector& OriginForward)
{
	const FVector CenterToTarget = ReachTarget - RotateCenter;
	const FVector CenterToTarget_XY = {CenterToTarget.X, CenterToTarget.Y, 0};
	const FVector ViewForward_XY = OriginForward.GetSafeNormal2D();
	const float Gamma =
		FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CenterToTarget_XY.GetSafeNormal(), -ViewForward_XY)));

	const float Beta = FMath::RadiansToDegrees(FMath::Asin(
		FMath::Sin(FMath::DegreesToRadians(FrustumHalfAngle)) * VertexCenterDistance / CenterToTarget.Size2D()));

	const float ModifyYaw = 180. - (Beta + FrustumHalfAngle) - Gamma;

	const FVector View = RotateCenter - ViewForward_XY * VertexCenterDistance;
	const FVector ViewToTarget = ReachTarget - View;

	if (CheckRightSide2D(ViewForward_XY, ViewToTarget))
	{
		return ModifyYaw;
	}
	else
	{
		return -ModifyYaw;
	}
}

float UJoyMathBlueprintLibrary::CalculateTangentRotationYaw_ForReachTarget(const FVector& RotateCenter,
	const FVector& LocalOffsetToCenter, const FVector& ReachTarget, const FVector& ViewLocation,
	const FRotator& ViewRotator)
{
	// 计算 XY 平面的偏移半径
	const float Rxy = FMath::Abs(
		FMath::Sin(FMath::Acos(FVector::DotProduct(-LocalOffsetToCenter.GetSafeNormal2D(), FVector::ForwardVector))) *
		LocalOffsetToCenter.Size2D());

	const FVector CenterToTarget = ReachTarget - RotateCenter;
	const float DistanceBetweenCenterAndTargetXY = CenterToTarget.Size2D();

	const float YawDelta = FMath::RadiansToDegrees(FMath::Asin(Rxy / DistanceBetweenCenterAndTargetXY));

	const FVector ViewToCenter = RotateCenter - ViewLocation;
	const FVector ViewForward = ViewRotator.RotateVector(FVector::ForwardVector);

	FRotator TargetToCenterRotator = (-CenterToTarget.GetSafeNormal2D()).Rotation();
	if (CheckLeftSide2D(ViewForward.GetSafeNormal2D(), ViewToCenter.GetSafeNormal2D()))
	{
		// 逆时针
		TargetToCenterRotator.Yaw -= YawDelta;
	}
	else
	{
		// 顺时针
		TargetToCenterRotator.Yaw += YawDelta;
	}

	const float AlphaYaw = FMath::Abs(FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(
		-(TargetToCenterRotator.RotateVector(FVector::ForwardVector)), ViewForward.GetSafeNormal2D()))));

	const FVector ViewToTarget = ReachTarget - ViewLocation;
	if (CheckRightSide2D(ViewForward.GetSafeNormal2D(), ViewToTarget.GetSafeNormal2D()))
	{
		// 顺时针
		return AlphaYaw;
	}
	else
	{
		// 逆时针
		return -AlphaYaw;
	}
}

float UJoyMathBlueprintLibrary::CalculateTangentRotationPitch_ForReachTarget(const FVector& RotateCenter,
	const FVector& LocalOffsetToCenter, const FVector& ReachTarget, const FVector& ViewLocation,
	const FRotator& ViewRotator)
{
	// 计算 ViewForward 平面的偏移半径
	const float Rz = FMath::Abs(LocalOffsetToCenter.Z);

	const FVector ViewForward = ViewRotator.RotateVector(FVector::ForwardVector).GetSafeNormal();
	const FVector ViewForwardXY = ViewRotator.RotateVector(FVector::ForwardVector).GetSafeNormal2D();

	const FVector CenterToTarget = ReachTarget - RotateCenter;
	const float DistanceBetweenCenterAndTargetZPanel =
		FMath::Sqrt(FMath::Square(FVector::DotProduct(ViewForwardXY, CenterToTarget)) +
					FMath::Square(FMath::Abs(CenterToTarget.Z)));

	const float PitchDelta = FMath::RadiansToDegrees(FMath::Asin(Rz / DistanceBetweenCenterAndTargetZPanel));

	const float TempPitchDegree =
		FMath::RadiansToDegrees(FMath::Asin(FMath::Abs(CenterToTarget.Z) / DistanceBetweenCenterAndTargetZPanel));
	float TargetToCenterPitch = CenterToTarget.Z < 0 ? TempPitchDegree : -TempPitchDegree;

	if (LocalOffsetToCenter.Z > 0)
	{
		TargetToCenterPitch += PitchDelta;
	}
	else
	{
		TargetToCenterPitch -= PitchDelta;
	}

	const FRotator NegativeViewForwardRotatorXY = (-ViewForwardXY).Rotation();
	const FRotator TargetToCenterRotator = {
		TargetToCenterPitch,
		NegativeViewForwardRotatorXY.Yaw,
		NegativeViewForwardRotatorXY.Roll,
	};

	const float AlphaPitch = FMath::Abs(FMath::RadiansToDegrees(
		FMath::Acos(FVector::DotProduct(-(TargetToCenterRotator.RotateVector(FVector::ForwardVector)), ViewForward))));

	const FVector ViewToTargetNormal = (ReachTarget - ViewLocation).GetSafeNormal();
	if (ViewToTargetNormal.Z < ViewForward.Z)
	{
		return -AlphaPitch;
	}
	else
	{
		return AlphaPitch;
	}
}

float UJoyMathBlueprintLibrary::GetNormalizeCurveFloat(
	const TObjectPtr<UCurveFloat> Curve, const float Alpha, const float StartX, const float EndX)
{
	if (Curve == nullptr)
	{
		return 0.0f;
	}

	const float VarX = FMath::Clamp(Alpha, StartX, EndX);
	const float StartY = Curve->GetFloatValue(StartX);
	const float EndY = Curve->GetFloatValue(EndX);
	if (FMath::IsNearlyEqual(StartY, EndY))
	{
		return StartY;
	}

	return (Curve->GetFloatValue(VarX) - StartY) / (EndY - StartY);
}
