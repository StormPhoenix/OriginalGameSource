#pragma once

#include "Engine/World.h"
#include "GameplayTagContainer.h"

#include "JoyCameraMode.generated.h"

class UJoyCameraComponent;
class AJoyPlayerCameraManager;

/**
 * EJoyCameraModeBlendFunction
 *
 *	Blend function used for transitioning between camera modes.
 */
UENUM(BlueprintType)
enum class EJoyCameraModeBlendFunction : uint8
{
	// Does a simple linear interpolation.
	Linear,

	// Immediately accelerates, but smoothly decelerates into the target.  Ease amount controlled by the exponent.
	EaseIn,

	// Smoothly accelerates, but does not decelerate into the target.  Ease amount controlled by the exponent.
	EaseOut,

	// Smoothly accelerates and decelerates.  Ease amount controlled by the exponent.
	EaseInOut,

	COUNT UMETA(Hidden)
};

/**
 * FJoyCameraModeView
 *
 *	View data produced by the camera mode that is used to blend camera modes.
 */
struct FJoyCameraModeView
{
public:
	FJoyCameraModeView();

	void Blend(const FJoyCameraModeView& Other, float OtherWeight);

public:
	FVector Location;
	FRotator Rotation;
	FRotator ControlRotation;
	float FieldOfView;
};

/**
 * UJoyCameraMode
 *
 *	Base class for all camera modes.
 */
UCLASS(Abstract, NotBlueprintable)
class ORIGINALGAME_API UJoyCameraMode : public UObject
{
	GENERATED_BODY()

public:
	UJoyCameraMode();

	UJoyCameraComponent* GetJoyCameraComponent() const;

	virtual UWorld* GetWorld() const override;

	AActor* GetTargetActor() const;

	const FJoyCameraModeView& GetCameraModeView() const
	{
		return View;
	}

	// Called when this camera mode is activated on the camera mode stack.
	virtual void OnActivation();

	// Called when this camera mode is deactivated on the camera mode stack.
	virtual void OnDeactivation(){};

	virtual void UpdateCameraMode(float DeltaTime);

	float GetBlendTime() const
	{
		return BlendTime;
	}

	float GetBlendWeight() const
	{
		return BlendWeight;
	}

	void SetBlendWeight(float Weight);

	FGameplayTag GetCameraTypeTag() const
	{
		return CameraTypeTag;
	}

protected:
	virtual FVector GetPivotLocation() const;
	virtual FRotator CalcPivotRotation() const;

	virtual void UpdateView(float DeltaTime);
	virtual void UpdateBlending(float DeltaTime);

protected:
	// A tag that can be queried by gameplay code that cares when a kind of camera mode is active
	// without having to ask about a specific mode (e.g., when aiming downsights to get more accuracy)
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	FGameplayTag CameraTypeTag;

	// View output produced by the camera mode.
	FJoyCameraModeView View;

	float BlendTime;

	// Function used for blending.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	EJoyCameraModeBlendFunction BlendFunction;

	// Exponent used by blend functions to control the shape of the curve.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendExponent = 4.0;

	// Linear blend alpha used to determine the blend weight.
	float BlendAlpha;

	// Blend weight calculated using the blend alpha and function.
	float BlendWeight;

protected:
	/** If true, skips all interpolation and puts camera in ideal location.  Automatically set to false next frame. */
	UPROPERTY(transient)
	uint32 bResetInterpolation : 1;

	UPROPERTY()
	TObjectPtr<AJoyPlayerCameraManager> PlayerCameraManager;
};
