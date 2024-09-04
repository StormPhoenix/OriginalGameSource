// Copyright Epic Games, Inc. All Rights Reserved.

#include "JoyCameraMode.h"

#include "Camera/JoyCameraComponent.h"
#include "Camera/JoyPlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Utils/JoyCameraBlueprintLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(JoyCameraMode)

//////////////////////////////////////////////////////////////////////////
// FJoyCameraModeView
//////////////////////////////////////////////////////////////////////////
///
FJoyCameraModeView::FJoyCameraModeView()
	: Location(ForceInit), Rotation(ForceInit), ControlRotation(ForceInit), FieldOfView(JOY_CAMERA_DEFAULT_FOV)
{
}

void FJoyCameraModeView::Blend(const FJoyCameraModeView& Other, float OtherWeight)
{
	if (OtherWeight <= 0.0f)
	{
		return;
	}
	else if (OtherWeight >= 1.0f)
	{
		*this = Other;
		return;
	}

	Location = FMath::Lerp(Location, Other.Location, OtherWeight);

	const FRotator DeltaRotation = (Other.Rotation - Rotation).GetNormalized();
	Rotation = Rotation + (OtherWeight * DeltaRotation);

	const FRotator DeltaControlRotation = (Other.ControlRotation - ControlRotation).GetNormalized();
	ControlRotation = ControlRotation + (OtherWeight * DeltaControlRotation);

	FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, OtherWeight);
}

//////////////////////////////////////////////////////////////////////////
// UJoyCameraMode
//////////////////////////////////////////////////////////////////////////
UJoyCameraMode::UJoyCameraMode()
{
	BlendTime = 0.5f;
	BlendFunction = EJoyCameraModeBlendFunction::EaseOut;
	BlendExponent = 4.0f;
	BlendAlpha = 1.0f;
	BlendWeight = 1.0f;
}

UJoyCameraComponent* UJoyCameraMode::GetJoyCameraComponent() const
{
	return CastChecked<UJoyCameraComponent>(GetOuter());
}

UWorld* UJoyCameraMode::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

AActor* UJoyCameraMode::GetTargetActor() const
{
	if (const auto* CameraComponent = GetJoyCameraComponent())
	{
		return CameraComponent->GetTargetActor();
	}

	return nullptr;
}

void UJoyCameraMode::OnActivation()
{
	PlayerCameraManager = UJoyCameraBlueprintLibrary::GetJoyPlayerCameraManager(GetTargetActor());
};

FVector UJoyCameraMode::GetPivotLocation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		// Height adjustments for characters to account for crouching.
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
		{
			const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
			check(TargetCharacterCDO);

			const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
			check(CapsuleComp);

			const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
			check(CapsuleCompCDO);

			const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
			const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
			const float HeightAdjustment = (DefaultHalfHeight - ActualHalfHeight) + TargetCharacterCDO->BaseEyeHeight;

			return TargetCharacter->GetActorLocation() + (FVector::UpVector * HeightAdjustment);
		}

		return TargetPawn->GetPawnViewLocation();
	}

	return TargetActor->GetActorLocation();
}

FRotator UJoyCameraMode::CalcPivotRotation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		return TargetPawn->GetViewRotation();
	}

	return TargetActor->GetActorRotation();
}

void UJoyCameraMode::UpdateCameraMode(float DeltaTime)
{
	UpdateView(DeltaTime);
	UpdateBlending(DeltaTime);
}

void UJoyCameraMode::UpdateView(float DeltaTime)
{
	const FVector PivotLocation = GetPivotLocation();
	FRotator PivotRotation = CalcPivotRotation();

	PivotRotation.Pitch =
		FMath::ClampAngle(PivotRotation.Pitch, JOY_CAMERA_DEFAULT_PITCH_MIN, JOY_CAMERA_DEFAULT_PITCH_MAX);

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = JOY_CAMERA_DEFAULT_FOV;
}

void UJoyCameraMode::SetBlendWeight(float Weight)
{
	BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

	// Since we're setting the blend weight directly, we need to calculate the blend alpha to account for the blend
	// function.
	const float InvExponent = (BlendExponent > 0.0f) ? (1.0f / BlendExponent) : 1.0f;

	switch (BlendFunction)
	{
		case EJoyCameraModeBlendFunction::Linear:
			BlendAlpha = BlendWeight;
			break;

		case EJoyCameraModeBlendFunction::EaseIn:
			BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InvExponent);
			break;

		case EJoyCameraModeBlendFunction::EaseOut:
			BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InvExponent);
			break;

		case EJoyCameraModeBlendFunction::EaseInOut:
			BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InvExponent);
			break;

		default:
			checkf(false, TEXT("SetBlendWeight: Invalid BlendFunction [%d]\n"), (uint8) BlendFunction);
			break;
	}
}

void UJoyCameraMode::UpdateBlending(float DeltaTime)
{
	if (BlendTime > 0.0f)
	{
		BlendAlpha += (DeltaTime / BlendTime);
		BlendAlpha = FMath::Min(BlendAlpha, 1.0f);
	}
	else
	{
		BlendAlpha = 1.0f;
	}

	const float Exponent = (BlendExponent > 0.0f) ? BlendExponent : 1.0f;

	switch (BlendFunction)
	{
		case EJoyCameraModeBlendFunction::Linear:
			BlendWeight = BlendAlpha;
			break;

		case EJoyCameraModeBlendFunction::EaseIn:
			BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
			break;

		case EJoyCameraModeBlendFunction::EaseOut:
			BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
			break;

		case EJoyCameraModeBlendFunction::EaseInOut:
			BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
			break;

		default:
			checkf(false, TEXT("UpdateBlending: Invalid BlendFunction [%d]\n"), (uint8) BlendFunction);
			break;
	}
}
