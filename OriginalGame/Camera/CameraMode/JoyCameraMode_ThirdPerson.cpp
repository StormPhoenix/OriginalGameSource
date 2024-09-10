// Copyright Epic Games, Inc. All Rights Reserved.

#include "JoyCameraMode_ThirdPerson.h"

#include "Camera/JoyCameraComponent.h"
#include "Camera/JoyPenetrationAvoidanceFeeler.h"
#include "Camera/JoyPlayerCameraManager.h"
#include "Character/JoyCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/CameraBlockingVolume.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Math/RotationMatrix.h"
#include "Utils/JoyCharacterBlueprintLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(JoyCameraMode_ThirdPerson)

namespace JoyCameraMode_ThirdPerson_Statics
{
static const FName NAME_IgnoreCameraCollision = TEXT("IgnoreCameraCollision");
}

UJoyCameraMode_ThirdPerson::UJoyCameraMode_ThirdPerson()
{
	UpdateFeelers();
}

void UJoyCameraMode_ThirdPerson::LookAtLockTarget()
{
}

FVector UJoyCameraMode_ThirdPerson::GetBaseEye() const
{
	// return AJoyPlayerCameraManager::GetCharacterBaseEye(GetTargetActor()) + CurrentCrouchOffset;
	if (PlayerCameraManager != nullptr)
	{
		return PlayerCameraManager->GetCharacterCameraBase(GetTargetActor());
	}
	else
	{
		return FVector::ZeroVector;
	}
}

FVector UJoyCameraMode_ThirdPerson::CalcPivotLocation() const
{
	FVector PivotLocation = GetBaseEye();
	if (PlayerCameraManager != nullptr)
	{
		// 更新相机位置偏移
		PivotLocation += PlayerCameraManager->GetCurrentArmCenterOffset(GetTargetActor());
	}

	return PivotLocation;
}

void UJoyCameraMode_ThirdPerson::UpdateFeelers()
{
	constexpr float Extent = 14.0f;
	PenetrationAvoidanceFeelers.Empty();

	PenetrationAvoidanceFeelers.Add(
		FJoyPenetrationAvoidanceFeeler(FRotator(+00.0f, +00.0f, 0.0f), 1.00f, 1.00f, Extent, true, 0));
	PenetrationAvoidanceFeelers.Add(
		FJoyPenetrationAvoidanceFeeler(FRotator(+00.0f, +16.0f, 0.0f), 0.75f, 0.75f, 00.f, false, 3));
	PenetrationAvoidanceFeelers.Add(
		FJoyPenetrationAvoidanceFeeler(FRotator(+00.0f, -16.0f, 0.0f), 0.75f, 0.75f, 00.f, false, 3));
	PenetrationAvoidanceFeelers.Add(
		FJoyPenetrationAvoidanceFeeler(FRotator(+00.0f, +32.0f, 0.0f), 0.50f, 0.50f, 00.f, false, 5));
	PenetrationAvoidanceFeelers.Add(
		FJoyPenetrationAvoidanceFeeler(FRotator(+00.0f, -32.0f, 0.0f), 0.50f, 0.50f, 00.f, false, 5));
	// PenetrationAvoidanceFeelers.Add(
	// 	FFGPenetrationAvoidanceFeeler(FRotator(+20.0f, +00.0f, 0.0f), 1.00f, 1.00f, 00.f, 4));
	// PenetrationAvoidanceFeelers.Add(
	//	FFGPenetrationAvoidanceFeeler(FRotator(-20.0f, +00.0f, 0.0f), 0.50f, 0.50f, 00.f, false, 4));
}

bool UJoyCameraMode_ThirdPerson::IsEnableLocationLag() const
{
	return PlayerCameraManager != nullptr ? PlayerCameraManager->CameraLagEnable() : false;
}

bool UJoyCameraMode_ThirdPerson::IsEnableRotationLag() const
{
	return PlayerCameraManager != nullptr ? PlayerCameraManager->bEnableCameraRotLag : false;
}

void UJoyCameraMode_ThirdPerson::UpdateView(float DeltaTime)
{
	UpdateForTarget(DeltaTime);
	UpdateCrouchOffset(DeltaTime);

	/* ------------------------------------------------------------------------------ */
	/* ----------------- 计算 ArmLength \ ArmRotation \ ArmLocation ------------------ */
	/* ------------------------------------------------------------------------------ */
	float ArmLength = -350;
	if (PlayerCameraManager != nullptr)
	{
		// 获取相机臂长度
		ArmLength = -PlayerCameraManager->GetCurrentArmLength(GetTargetActor());

		// 更新相机 Fov
		View.FieldOfView = PlayerCameraManager->GetCurrentCameraFov(GetTargetActor());

		// 更新相机臂配置
		MinArmLength = PlayerCameraManager->GetBaseMinArmLength();
		MaxArmLength = PlayerCameraManager->GetBaseMaxArmLength();
	}
	else
	{
		View.FieldOfView = JOY_CAMERA_DEFAULT_FOV;
	}

	FRotator FinalRotator = CalcPivotRotation();
	FVector FinalLocation = CalcPivotLocation();

	/* --------------------------------------------------- */
	/* ----------------- 应用镜头位姿延迟 ------------------- */
	/* --------------------------------------------------- */

	// 应用相机臂变化延迟
	const FVector ArmVector = FVector(ArmLength, 0, 0);

	// 应用相机臂位姿延迟
	UpdateDesiredViewPose(DeltaTime, ArmVector, FinalLocation, FinalRotator);

	View.Rotation = FinalRotator;
	View.ControlRotation = View.Rotation;
	View.Location = FinalLocation + View.Rotation.RotateVector(ArmVector);

	// 记录真实相机臂中心位置
	if (auto* CameraComponent = GetJoyCameraComponent())
	{
		CameraComponent->CameraDataThisFrame.ViewLocation.Set(View.Location);
		if (GetTargetActor())
		{
			CameraComponent->CameraDataThisFrame.AvatarLocation.Set(GetTargetActor()->GetActorLocation());
		}
	}

	UpdatePreventPenetration(DeltaTime);
}

void UJoyCameraMode_ThirdPerson::UpdateDesiredViewPose(
	float DeltaTime, const FVector& ArmOffset, FVector& OutCameraLoc, FRotator& OutCameraRot)
{
	auto* CameraComponent = GetJoyCameraComponent();
	if (CameraComponent == nullptr)
	{
		return;
	}

	FRotator DesiredRot = OutCameraRot;

	if (IsEnableRotationLag())
	{
		if (!CameraComponent->CameraDataThisFrame.ViewRotation.CheckValid())
		{
			CameraComponent->CameraDataThisFrame.ViewRotation.Set(DesiredRot);
			return;
		}

		FQuat const QCurrent = CameraComponent->CameraDataThisFrame.ViewRotation.Data.GetNormalized().Quaternion();
		FQuat const QTarget = DesiredRot.GetNormalized().Quaternion();
		const float CameraRotLagRecoverSpeed = PlayerCameraManager != nullptr
			                                       ? PlayerCameraManager->GetArmRotatorLagRecoverSpeed()
			                                       : DefaultCameraRotLagSpeed;
		DesiredRot = FMath::RInterpTo(QCurrent.Rotator(), QTarget.Rotator(), DeltaTime, CameraRotLagRecoverSpeed);
	}

	/*
	 * CameraLoc: 相机基点
	 * 未添加 TargetOffset 之前的相机位置 PivotLocation
	 */
	FVector DesiredLoc = OutCameraLoc;
	// @TODO Location Lag 的代码之后迁移 PlayerCameraManager 里面

	if (!CameraComponent->CameraDataThisFrame.ArmLocation.CheckValid())
	{
		CameraComponent->CameraDataThisFrame.ArmLocation.Set(DesiredLoc);
		return;
	}

	if (IsEnableLocationLag())
	{
		const float CameraLagRecoverSpeed = PlayerCameraManager != nullptr
			                                    ? PlayerCameraManager->GetArmCenterLagRecoverSpeed()
			                                    : DefaultCameraLagRecoverSpeed;

		DesiredLoc = FMath::VInterpTo(
			CameraComponent->CameraDataThisFrame.ArmLocation.Data, DesiredLoc, DeltaTime, CameraLagRecoverSpeed);
		const float CameraLagMaxDistanceXY = PlayerCameraManager != nullptr
			                                     ? PlayerCameraManager->GetArmCenterLagMaxDistanceXY()
			                                     : DefaultCameraLagMaxDistance;

		const float CameraLagMaxDistanceZ = PlayerCameraManager != nullptr
			                                    ? PlayerCameraManager->GetArmCenterLagMaxDistanceZ()
			                                    : DefaultCameraLagMaxDistance;

		if (CameraLagMaxDistanceXY >= 0.f && CameraLagMaxDistanceZ >= 0.f)
		{
			bool bNeedClamp = false;
			const FVector FromOrigin = DesiredLoc - OutCameraLoc;
			FVector RealCameraLag = FromOrigin;
			if (FVector::DistXY(DesiredLoc, OutCameraLoc) > CameraLagMaxDistanceXY)
			{
				// XY 方向超过限制
				const FVector RealFromOrigin2D = FromOrigin.GetSafeNormal2D() * CameraLagMaxDistanceXY;
				RealCameraLag.X = RealFromOrigin2D.X;
				RealCameraLag.Y = RealFromOrigin2D.Y;
				bNeedClamp = true;
			}

			if (FMath::Abs(FromOrigin.Z) > CameraLagMaxDistanceZ)
			{
				RealCameraLag.Z = CameraLagMaxDistanceZ * (RealCameraLag.Z > 0 ? 1.0 : -1.0);
				bNeedClamp = true;
			}

			if (bNeedClamp)
			{
				// DesiredLoc = OutCameraLoc + FromOrigin.GetClampedToMaxSize(CameraLagMaxDistanceXY);
				DesiredLoc = OutCameraLoc + RealCameraLag;
			}
		}

		// 如果角色是面对着相机移动的，那么要防止角色太靠近相机导致穿模
		if (FVector::DotProduct(
			    DesiredRot.Vector(), (DesiredLoc - CameraComponent->CameraDataThisFrame.ArmLocation.Data)) < 0.)
		{
			float LaggedLength = (DesiredLoc - OutCameraLoc).Length();
			if ((ArmOffset.Length() - LaggedLength) < MinArmLength)
			{
				LaggedLength = ArmOffset.Length() - MinArmLength;
				DesiredLoc = OutCameraLoc + (DesiredLoc - OutCameraLoc).GetSafeNormal() * LaggedLength;
			}
		}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (PlayerCameraManager != nullptr && PlayerCameraManager->bDrawDebugLagMarkers)
		{
			DrawDebugSphere(GetWorld(), OutCameraLoc, 5.f, 8, FColor::Green);
			DrawDebugSphere(GetWorld(), DesiredLoc, 5.f, 8, FColor::Yellow);
		}
#endif
	}

	CameraComponent->CameraDataThisFrame.ArmLocation.Set(DesiredLoc);
	OutCameraLoc = DesiredLoc;

	CameraComponent->CameraDataThisFrame.ViewRotation.Set(DesiredRot);
	OutCameraRot = DesiredRot;
}

void UJoyCameraMode_ThirdPerson::UpdateForTarget(float DeltaTime)
{
	if (const ACharacter* TargetCharacter = Cast<ACharacter>(GetTargetActor()))
	{
		if (TargetCharacter->bIsCrouched)
		{
			const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
			const float CrouchedHeightAdjustment =
				TargetCharacterCDO->CrouchedEyeHeight - TargetCharacterCDO->BaseEyeHeight;

			SetTargetCrouchOffset(FVector(0.f, 0.f, CrouchedHeightAdjustment));
			return;
		}
	}

	SetTargetCrouchOffset(FVector::ZeroVector);
}

void UJoyCameraMode_ThirdPerson::UpdatePreventPenetration(float DeltaTime)
{
	AActor* TargetActor = GetTargetActor();

	APawn* TargetPawn = Cast<APawn>(TargetActor);
	AController* TargetController = TargetPawn ? TargetPawn->GetController() : nullptr;

	AActor* PPActor = TargetActor;

	if (const UPrimitiveComponent* PPActorRootComponent = Cast<UPrimitiveComponent>(PPActor->GetRootComponent()))
	{
		// Attempt at picking SafeLocation automatically, so we reduce camera translation when aiming.
		// Our camera is our reticle, so we want to preserve our aim and keep that as steady and smooth as possible.
		// Pick closest point on capsule to our aim line.
		FVector ClosestPointOnLineToCapsuleCenter;
		FVector SafeLocation = PPActor->GetActorLocation();
		FMath::PointDistToLine(SafeLocation, View.Rotation.Vector(), View.Location, ClosestPointOnLineToCapsuleCenter);

		// Adjust Safe distance height to be same as aim line, but within capsule.
		float const PushInDistance = PenetrationAvoidanceFeelers[0].Extent + CollisionPushOutDistance;
		float const MaxHalfHeight = PPActor->GetSimpleCollisionHalfHeight() - PushInDistance;
		SafeLocation.Z = FMath::Clamp(
			ClosestPointOnLineToCapsuleCenter.Z, SafeLocation.Z - MaxHalfHeight, SafeLocation.Z + MaxHalfHeight);

		float DistanceSqr;
		PPActorRootComponent->GetSquaredDistanceToCollision(
			ClosestPointOnLineToCapsuleCenter, DistanceSqr, SafeLocation);
		// Push back inside capsule to avoid initial penetration when doing line checks.
		if (PenetrationAvoidanceFeelers.Num() > 0)
		{
			SafeLocation += (SafeLocation - ClosestPointOnLineToCapsuleCenter).GetSafeNormal() * PushInDistance;
		}

		// Then aim line to desired camera position
		bool const bSingleRayPenetrationCheck = !bDoPredictiveAvoidance;

		PreventCameraPenetration(*PPActor, SafeLocation, View.Location, DeltaTime, AimLineToDesiredPosBlockedPct,
			bSingleRayPenetrationCheck);
	}
}

FRotator UJoyCameraMode_ThirdPerson::CalcPivotRotation() const
{
	if (PlayerCameraManager == nullptr)
	{
		return Super::CalcPivotRotation();
	}
	else
	{
		return PlayerCameraManager->GetCurrentCameraArmCenterRotation(GetTargetActor());
	}
}

void UJoyCameraMode_ThirdPerson::InitParams()
{
	PenetrationBlendInTime = 0.1;
	PenetrationBlendOutTime = 0.15;
	BlendTime = 0.5;

	CollisionPushOutDistance = 2.0;
	ReportPenetrationPercent = 0.0;
	CrouchOffsetBlendMultiplier = 5.0;

	bDoPredictiveAvoidance = true;
}

void UJoyCameraMode_ThirdPerson::OnDeactivation()
{
	Super::OnDeactivation();
}

void UJoyCameraMode_ThirdPerson::OnActivation()
{
	Super::OnActivation();

	// 初始化基本参数
	InitParams();
	AimLineToDesiredPosBlockedPct = 1.0f;
}

void UJoyCameraMode_ThirdPerson::PreventCameraPenetration(class AActor const& ViewTarget, FVector const& SafeLoc,
	FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly)
{
#if ENABLE_DRAW_DEBUG
	DebugActorsHitDuringCameraPenetration.Reset();
#endif

	float HardBlockedPct = DistBlockedPct;
	float SoftBlockedPct = DistBlockedPct;

	FVector BaseRay = CameraLoc - SafeLoc;

	const FVector BaseRayLocalUp = View.Rotation.RotateVector(FVector(0.0, 0.0, 1.0));
	const FVector BaseRayLocalRight = View.Rotation.RotateVector(FVector(0.0, 1.0, 0.0));

	UpdateFeelers();

	float DistBlockedPctThisFrame = 1.f;

	int32 const NumRaysToShoot =
		bSingleRayOnly ? FMath::Min(1, PenetrationAvoidanceFeelers.Num()) : PenetrationAvoidanceFeelers.Num();
	FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(CameraPen), false, nullptr /*PlayerCamera*/);

	SphereParams.AddIgnoredActor(&ViewTarget);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(0.f);
	UWorld* World = GetWorld();
	for (int32 RayIdx = 0; RayIdx < NumRaysToShoot; ++RayIdx)
	{
		FJoyPenetrationAvoidanceFeeler& Feeler = PenetrationAvoidanceFeelers[RayIdx];
		if (Feeler.FramesUntilNextTrace <= 0)
		{
			// calc ray target
			FVector RayTarget;
			{
				FVector RotatedRay = BaseRay.RotateAngleAxis(Feeler.AdjustmentRot.Yaw, BaseRayLocalUp);
				RotatedRay = RotatedRay.RotateAngleAxis(Feeler.AdjustmentRot.Pitch, BaseRayLocalRight);
				RayTarget = SafeLoc + RotatedRay;
			}

			// cast for world and pawn hits separately.  this is so we can safely ignore the
			// camera's target pawn
			SphereShape.Sphere.Radius = Feeler.Extent;
			ECollisionChannel TraceChannel = ECC_Camera; //(Feeler.PawnWeight > 0.f) ? ECC_Pawn : ECC_Camera;

			// do multi-line check to make sure the hits we throw out aren't
			// masking real hits behind (these are important rays).

			// MT-> passing camera as actor so that camerablockingvolumes know when it's the camera doing traces
			FHitResult Hit;
			const bool bHit = World->SweepSingleByChannel(
				Hit, SafeLoc, RayTarget, FQuat::Identity, TraceChannel, SphereShape, SphereParams);
#if ENABLE_DRAW_DEBUG
			if (PlayerCameraManager != nullptr && PlayerCameraManager->bDrawDebugPenetrationMarkers)
			{
				DrawDebugSphere(World, SafeLoc, SphereShape.Sphere.Radius, 8, FColor::Red);
				DrawDebugSphere(World, bHit ? Hit.Location : RayTarget, 10, 8, FColor::Purple);
				DrawDebugLine(World, SafeLoc, bHit ? Hit.Location : RayTarget, FColor::Red);
			}
#endif	  // ENABLE_DRAW_DEBUG

			Feeler.FramesUntilNextTrace = Feeler.TraceInterval;

			AActor* HitActor = Hit.GetActor();
			if (bHit && HitActor)
			{
				bool bIgnoreHit = false;

				if (HitActor->ActorHasTag(JoyCameraMode_ThirdPerson_Statics::NAME_IgnoreCameraCollision))
				{
					bIgnoreHit = true;
					SphereParams.AddIgnoredActor(HitActor);
				}

				// Ignore CameraBlockingVolume hits that occur in front of the ViewTarget.
				if (!bIgnoreHit && HitActor->IsA<ACameraBlockingVolume>())
				{
					const FVector ViewTargetForwardXY = ViewTarget.GetActorForwardVector().GetSafeNormal2D();
					const FVector ViewTargetLocation = ViewTarget.GetActorLocation();
					const FVector HitOffset = Hit.Location - ViewTargetLocation;
					const FVector HitDirectionXY = HitOffset.GetSafeNormal2D();
					const float DotHitDirection = FVector::DotProduct(ViewTargetForwardXY, HitDirectionXY);
					if (DotHitDirection > 0.0f)
					{
						bIgnoreHit = true;
						// Ignore this CameraBlockingVolume on the remaining sweeps.
						SphereParams.AddIgnoredActor(HitActor);
					}
					else
					{
#if ENABLE_DRAW_DEBUG
						DebugActorsHitDuringCameraPenetration.AddUnique(TObjectPtr<const AActor>(HitActor));
#endif
					}
				}

				if (!bIgnoreHit)
				{
					float const Weight = Cast<APawn>(Hit.GetActor()) ? Feeler.PawnWeight : Feeler.WorldWeight;
					float NewBlockPct = Hit.Time;
					NewBlockPct += (1.f - NewBlockPct) * (1.f - Weight);

					// Recompute blocked pct taking into account pushout distance.
					NewBlockPct =
						((Hit.Location - SafeLoc).Size() - CollisionPushOutDistance) / (RayTarget - SafeLoc).Size();

#if ENABLE_DRAW_DEBUG
					if (PlayerCameraManager != nullptr && PlayerCameraManager->bDrawDebugPenetrationMarkers)
					{
						DrawDebugSphere(World, Hit.Location, 10, 8, FColor::Orange);
						DrawDebugLine(World, SafeLoc, Hit.Location, FColor::White);
					}
#endif

					DistBlockedPctThisFrame = FMath::Min(NewBlockPct, DistBlockedPctThisFrame);

					// This feeler got a hit, so do another trace next frame
					Feeler.FramesUntilNextTrace = 0;

#if ENABLE_DRAW_DEBUG
					DebugActorsHitDuringCameraPenetration.AddUnique(TObjectPtr<const AActor>(HitActor));
#endif
				}
			}

			if (RayIdx == 0)
			{
				// don't interpolate toward this one, snap to it
				// assumes ray 0 is the center/main ray
				HardBlockedPct = DistBlockedPctThisFrame;
			}
			else
			{
				SoftBlockedPct = DistBlockedPctThisFrame;
			}
		}
		else
		{
			--Feeler.FramesUntilNextTrace;
		}
	}

	if (bResetInterpolation)
	{
		DistBlockedPct = DistBlockedPctThisFrame;
	}
	else if (DistBlockedPct < DistBlockedPctThisFrame)
	{
		// interpolate smoothly out
		if (PenetrationBlendOutTime > DeltaTime)
		{
			DistBlockedPct =
				DistBlockedPct + DeltaTime / PenetrationBlendOutTime * (DistBlockedPctThisFrame - DistBlockedPct);
		}
		else
		{
			DistBlockedPct = DistBlockedPctThisFrame;
		}
	}
	else
	{
		if (DistBlockedPct > HardBlockedPct)
		{
			DistBlockedPct = HardBlockedPct;
		}
		else if (DistBlockedPct > SoftBlockedPct)
		{
			// interpolate smoothly in
			if (PenetrationBlendInTime > DeltaTime)
			{
				DistBlockedPct =
					DistBlockedPct - DeltaTime / PenetrationBlendInTime * (DistBlockedPct - SoftBlockedPct);
			}
			else
			{
				DistBlockedPct = SoftBlockedPct;
			}
		}
	}

	DistBlockedPct = FMath::Clamp<float>(DistBlockedPct, 0.f, 1.f);
	if (DistBlockedPct < (1.f - ZERO_ANIMWEIGHT_THRESH))
	{
		CameraLoc = SafeLoc + (CameraLoc - SafeLoc) * DistBlockedPct;
	}
}

void UJoyCameraMode_ThirdPerson::SetTargetCrouchOffset(FVector NewTargetOffset)
{
	CrouchOffsetBlendPct = 0.0f;
	InitialCrouchOffset = CurrentCrouchOffset;
	TargetCrouchOffset = NewTargetOffset;
}

void UJoyCameraMode_ThirdPerson::UpdateCrouchOffset(float DeltaTime)
{
	if (CrouchOffsetBlendPct < 1.0f)
	{
		CrouchOffsetBlendPct = FMath::Min(CrouchOffsetBlendPct + DeltaTime * CrouchOffsetBlendMultiplier, 1.0f);
		CurrentCrouchOffset =
			FMath::InterpEaseInOut(InitialCrouchOffset, TargetCrouchOffset, CrouchOffsetBlendPct, 1.0f);
	}
	else
	{
		CurrentCrouchOffset = TargetCrouchOffset;
		CrouchOffsetBlendPct = 1.0f;
	}
}
