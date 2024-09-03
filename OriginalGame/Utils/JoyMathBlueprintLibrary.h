// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "JoyMathBlueprintLibrary.generated.h"

/**
 *
 */
UCLASS()
class ORIGINALGAME_API UJoyMathBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static float GetNormalizeCurveFloat(
		const TObjectPtr<UCurveFloat> Curve, float Alpha, float StartX = 0.f, float EndX = 1.f);

	static float Sin2D(const FVector& VecA, const FVector& VecB);

	static float LerpSin(const float A, const float B, const float BlendAlpha);

	/**
	 * VecB 在 VecA 的左侧
	 * @param VecA
	 * @param VecB
	 * @return
	 */
	UFUNCTION(BlueprintPure, Category = "Joy|Math", DisplayName = "VecB 在 VecA 的左侧")
	static bool CheckLeftSide2D(const FVector& VecA, const FVector& VecB);

	/**
	 * VecB 在 VecA 的右侧
	 * @param VecA
	 * @param VecB
	 * @return
	 */
	UFUNCTION(BlueprintPure, Category = "Joy|Math", DisplayName = "VecB 在 VecA 的右侧")
	static bool CheckRightSide2D(const FVector& VecA, const FVector& VecB);

	/**
	 * 计算视锥要旋转多少 yaw 才能恰好包含目标点 Target
	 * @return
	 */
	static float CalculateFrustumRotationYaw_ForReachTarget(const float FrustumHalfAngle,
		const float VertexCenterDistance, const FVector& RotateCenter, const FVector& ReachTarget,
		const FVector& OriginForward);

	/**
	 * 计算视锥要旋转多少 pitch 才能恰好包含目标点 Target
	 * @return
	 */
	static float CalculateFrustumRotationPitch_ForReachTarget(const float FrustumHalfAngle,
		const float VertexCenterDistance, const FVector& RotateCenter, const FVector& ReachTarget,
		const FVector& OriginForward);

	/**
	 * 计算绕中心点 RotateCenter 的切线在 XY 平面上旋转多少角度才能抵达目标 Target
	 * @param RotateCenter
	 * @param LocalOffsetToCenter
	 * @param ReachTarget
	 * @param ViewLocation
	 * @param ViewRotator
	 * @return
	 */
	static float CalculateTangentRotationYaw_ForReachTarget(const FVector& RotateCenter,
		const FVector& LocalOffsetToCenter, const FVector& ReachTarget, const FVector& ViewLocation,
		const FRotator& ViewRotator);

	static float CalculateTangentRotationPitch_ForReachTarget(const FVector& RotateCenter,
		const FVector& LocalOffsetToCenter, const FVector& ReachTarget, const FVector& ViewLocation,
		const FRotator& ViewRotator);
};
