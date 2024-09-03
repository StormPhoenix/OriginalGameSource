#pragma once
#include "JoyCameraMode.h"

#include "JoyCameraMode_ThirdPerson.generated.h"

struct FJoyPenetrationAvoidanceFeeler;

/**
 * UJoyCameraMode_ThirdPerson
 *
 *	A basic third person camera mode.
 */

UCLASS(Blueprintable)
class UJoyCameraMode_ThirdPerson : public UJoyCameraMode
{
	GENERATED_BODY()

public:
	UJoyCameraMode_ThirdPerson();

	void LookAtLockTarget();

protected:
	void InitParams();

	virtual FRotator CalcPivotRotation() const;

	virtual FVector CalcPivotLocation() const;

	void UpdateFeelers();

	virtual void UpdateView(float DeltaTime) override;

	virtual void UpdateDesiredViewPose(
		float DeltaTime, const FVector& ArmOffset, FVector& OutCameraLoc, FRotator& OutCameraRot);

	void UpdateForTarget(float DeltaTime);
	void UpdatePreventPenetration(float DeltaTime);
	virtual void PreventCameraPenetration(class AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc,
		float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly);

	virtual void OnActivation() override;

	virtual void OnDeactivation();

	virtual bool IsEnableLocationLag() const;

	virtual bool IsEnableRotationLag() const;

	FVector GetBaseEye() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetX;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetY;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetZ;

	// Alters the speed that a crouch offset is blended in or out
	float CrouchOffsetBlendMultiplier = 5.0f;

public:
	float PenetrationBlendInTime = 0.1f;

	float PenetrationBlendOutTime = 0.15f;

	/** If true, try to detect nearby walls and move the camera in anticipation.  Helps prevent popping. */
	bool bDoPredictiveAvoidance = true;

	float CollisionPushOutDistance = 2.f;

	float ReportPenetrationPercent = 0.f;

	/**
	 * These are the feeler rays that are used to find where to place the camera.
	 * Index: 0  : This is the normal feeler we use to prevent collisions.
	 * Index: 1+ : These feelers are used if you bDoPredictiveAvoidance=true, to scan for potential impacts if the
	 * player were to rotate towards that direction and primitively collide the camera so that it pulls in before
	 *             impacting the occluder.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<FJoyPenetrationAvoidanceFeeler> PenetrationAvoidanceFeelers;

	UPROPERTY(Transient)
	float AimLineToDesiredPosBlockedPct;

	UPROPERTY(Transient)
	TArray<TObjectPtr<const AActor>> DebugActorsHitDuringCameraPenetration;

#if ENABLE_DRAW_DEBUG
	mutable float LastDrawDebugTime = -MAX_FLT;
#endif

protected:
	void SetTargetCrouchOffset(FVector NewTargetOffset);
	void UpdateCrouchOffset(float DeltaTime);

	FVector InitialCrouchOffset = FVector::ZeroVector;
	FVector TargetCrouchOffset = FVector::ZeroVector;
	float CrouchOffsetBlendPct = 1.0f;
	FVector CurrentCrouchOffset = FVector::ZeroVector;

private:
	UPROPERTY()
	float MinArmLength;

	UPROPERTY()
	float MaxArmLength;

	UPROPERTY()
	float DefaultCameraRotLagSpeed = 5;

	UPROPERTY()
	float DefaultCameraLagMaxDistance = 100;

	UPROPERTY()
	float DefaultCameraLagRecoverSpeed = 1;
};
