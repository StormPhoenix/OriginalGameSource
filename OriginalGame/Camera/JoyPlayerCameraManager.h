#pragma once

#include "Camera/PlayerCameraManager.h"
#include "Camera/Controller/JoyCameraMeta.h"
#include "Camera/Controller/JoyCameraModifierController.h"
#include "Input/JoyInputBlocker.h"

#include "JoyPlayerCameraManager.generated.h"

struct FGameplayTag;
struct FCameraModifyHandle;
class AJoyCharacter;
class FDebugDisplayInfo;
class UCanvas;
class UObject;
class UJoyCameraComponent;
class UJoyCameraConfigController;
class UJoyCameraInputController;
class UJoyCameraModifierController;

#define JOY_CAMERA_DEFAULT_FOV (80.0f)
#define JOY_CAMERA_DEFAULT_PITCH_MIN (-88.0f)
#define JOY_CAMERA_DEFAULT_PITCH_MAX (88.0f)

UENUM()
enum class EJoyCameraBlendType : uint8
{
	Default = 0 UMETA(Tooltip = "默认"),
	LockTarget = 1 UMETA(Tooltip = "切换过程中锁定目标"),
	KeepDirection = 2 UMETA(Tooltip = "切换过程中保持方向不变"),
};

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FViewInfoBlendFunction, FMinimalViewInfo&, A, FMinimalViewInfo&, B, float, T);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnViewTargetBlendComplete, AActor* ViewTarget, AActor* PendingViewTarget);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnCameraModifyFinishedDelegate, AActor*, CameraActor, FCameraModifyHandle, ModifierHandle);

USTRUCT()
struct FVirtualCamera
{
	GENERATED_BODY()

	UPROPERTY()
	FVector ArmCenterOffset = FVector::ZeroVector;

	UPROPERTY()
	float ArmLength = 0.;

	UPROPERTY()
	float MinArmLength = 0;

	UPROPERTY()
	float MaxArmLength = 0;

	FVirtualCamera& operator=(const FVirtualCamera& Other);

	UPROPERTY(DisplayName = "设置相机额外偏移(局部空间)")
	FVector LocalArmCenterOffset = FVector::ZeroVector;

	UPROPERTY(DisplayName = "设置相机额外偏移(世界空间)")
	FVector WorldArmOffsetAdditional = FVector::ZeroVector;

	UPROPERTY()
	float Fov = 0;

	UPROPERTY()
	FRotator ArmCenterRotation = FRotator::ZeroRotator;

	void CopyCamera(const FVirtualCamera& Other);

	void SetArmCenterOffset(const FVector& NewArmCenterOffset);

	void SetArmCenterRotation(const FRotator& NewArmCenterRotation);
};

USTRUCT()
struct FViewTargetCameraInfo
{
	GENERATED_BODY()

	/** 对相机执行 Modifier 之前保存的相机参数 */
	UPROPERTY()
	FVirtualCamera LastCamera;

	/** Modify 过程中插值得到的相机参数，并期望相机移动到 DesiredCamera 位置 */
	UPROPERTY()
	FVirtualCamera DesiredCamera;

	/** 对 DesiredCamera 做 Lag 的相机结果参数 */
	UPROPERTY()
	FVirtualCamera CurrentCamera;

	/** 若 Modify 修正结束后需恢复，则恢复到 RestoreCamera 镜头，一般
	 * RestoreCamera 与 LastCamera 一致， 但若一段 Modify 未执行完就被新的 Modify
	 * 打断时，就需要存储上一段 Modify 的恢复位置 */
	UPROPERTY()
	FVirtualCamera RestoreCamera;

	bool bNeedFading = false;

	// 是否要对弹簧臂长度变动做淡入
	bool bFadeArmLength = true;

	// 是否对弹簧臂长度范围做淡入
	bool bFadeArmLengthRange = true;

	// 是否要对弹簧臂旋转变动做淡入
	bool bFadeArmPitch = true;

	bool bFadeArmYaw = true;

	// 是否对相机的 local arm center Offset 做 fade 变换
	bool bFadeLocalArmCenterOffset = true;

	bool bFadeCameraFov = true;

	// 做 Fading 时，Arm 变化速度
	float ModifyArmLagSpeed = 1.0;

	/** ************ Current Camera Attributes Begin ************* */
	// 弹簧臂臂长是否被修改
	bool bCameraArmLength_HasModified = false;

	// 相机基础位置是否被修改
	bool bCameraOffset_HasModified = false;

	// 弹簧臂 Yaw 是否被修改
	bool bArmYaw_HasModified = false;

	// 弹簧臂 Pitch 是否被修改
	bool bArmPitch_HasModified = false;

	// 弹簧臂 Roll 是否被修改
	bool bArmRoll_HasModified = false;

	// 相机 Fov 是否被修改
	bool bCameraFov_HasModified = false;
	/** ************ Current Camera Attributes End ************* */

	UPROPERTY()
	TObjectPtr<class UJoyCameraModifierController> CameraModifierController;
};

USTRUCT()
struct FCameraConfigDescription
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FName> CameraStack{};

	UPROPERTY()
	bool bForceUpdateCameraConfigSet{true};

	UPROPERTY()
	FName FadeOutCamera{};
};

USTRUCT()
struct FMultiViewTargetCameraManager
{
	GENERATED_BODY()

	TObjectPtr<class UJoyCameraModifierController> GetCameraModifier(AActor* InViewTarget);

	bool ContainsViewTarget(const AActor* InViewTarget) const;

	FViewTargetCameraInfo& operator[](const AActor* InViewTarget)
	{
		return ViewTargetCameraInfos[InViewTarget];
	}

	const FViewTargetCameraInfo& operator[](const AActor* InViewTarget) const
	{
		return ViewTargetCameraInfos[InViewTarget];
	}

	void AddViewTarget(
		class AJoyPlayerCameraManager* CameraManager, AActor* InViewTarget, const FVirtualCamera& VirtualCamera);

	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, FViewTargetCameraInfo> ViewTargetCameraInfos{};
};

USTRUCT()
struct FCameraConfigFadingDescription
{
	GENERATED_BODY()

	UPROPERTY()
	bool bDuringFading{false};

	UPROPERTY()
	float Duration{0.f};

	UPROPERTY()
	float ElapseTime{0.f};

	UPROPERTY()
	bool bIgnoreTimeDilation{true};

	UPROPERTY()
	bool bOverrideCameraInput{false};
};

USTRUCT()
struct FInputOverrideDescription
{
	GENERATED_BODY()

	int BlockArmPitchCounter{0};

	int BlockArmYawCounter{0};

	int BlockArmLengthCounter{0};
};

/**
 * AJoyPlayerCameraManager
 *
 *	The base player camera manager class used by this project.
 */
UCLASS(notplaceable)
class ORIGINALGAME_API AJoyPlayerCameraManager : public APlayerCameraManager, public IJoyInputBlocker
{
	GENERATED_BODY()

	friend class UJoyCameraModifierController;
	friend class UJoyCameraConfigController;
	friend class UJoyCameraInputController;

public:
	virtual bool BlockMoveInput_Implementation(
		UObject* InputReceiver, const FInputActionValue& InputActionValue) override
	{
		return false;
	}

	virtual bool BlockAbilityTagPressInput_Implementation(UObject* InputReceiver, FGameplayTag const& InputTag) override
	{
		return false;
	}

	virtual bool BlockAbilityTagReleaseInput_Implementation(
		UObject* InputReceiver, FGameplayTag const& InputTag) override
	{
		return false;
	}

	virtual bool BlockLookMoveInput_Implementation(
		UObject* InputReceiver, const FInputActionValue& InputActionValue) override;

	virtual bool BlockMouseScrollInput_Implementation(
		UObject* InputReceiver, const FInputActionValue& InputActionValue) override;

	void SetArmPitchInputEnabled(bool bEnabled);

	void SetArmYawInputEnabled(bool bEnabled);

	void SetArmLengthInputEnabled(bool bEnabled);

	void AddDeviceYawInput(float Val) const;

	void AddDevicePitchInput(float Val) const;

	// @TODO 此处配置挪动到资产中
	UPROPERTY(EditAnywhere, Category = "Joy|Camera", DisplayName = "相机垂直运动方向参数 a")
	float MaxHeightParam_A = 0.5;

	UPROPERTY(EditAnywhere, Category = "Joy|Camera", DisplayName = "相机垂直运动方向参数 b")
	float MaxHeightParam_B = 10;

	UPROPERTY(EditAnywhere, Category = "Joy|Camera", DisplayName = "相机垂直运动最大高度 h")
	float MaxHeightParam_C = 500;

	UPROPERTY(EditAnywhere, Category = "Joy|Camera", DisplayName = "相机水平运动方向参数 a")
	float MaxXYOffsetParam_A = 0.5;

	UPROPERTY(EditAnywhere, Category = "Joy|Camera", DisplayName = "相机水平运动方向参数 b")
	float MaxXYOffsetParam_B = 10;

	UPROPERTY(EditAnywhere, Category = "Joy|Camera", DisplayName = "相机水平运动最大偏移 w")
	float MaxXYOffsetParam_C = 300;

	UPROPERTY(EditAnywhere, Category = "Joy|Camera", DisplayName = "战斗状态下，相机开始水平时的阈值")
	float ThresholdWhenOffsetBegin = 135;

	UPROPERTY(EditAnywhere, Category = "Joy|Camera", DisplayName = "相机 Pitch 参数 a")
	float PitchParam_A = 0.01;

	UPROPERTY(EditAnywhere, Category = "Joy|Camera", DisplayName = "相机 Pitch 参数 b")
	float PitchParam_B = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Joy|Camera|Lag")
	bool bEnableCameraRotLag = true;

	/**
	 * If true and camera location lag is enabled, draws markers at the camera
	 * target (in green) and the lagged position (in yellow). A line is drawn
	 * between the two locations, in green normally but in red if the distance to
	 * the lag target has been clamped (by CameraLagMaxDistance).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joy|Camera|Debug")
	uint32 bDrawDebugLagMarkers : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joy|Camera|Debug")
	uint32 bDrawDebugAttachSocket : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joy|Camera|Debug")
	uint32 bDrawDebugATBMarkers : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joy|Camera|Debug")
	uint32 bDrawDebugPenetrationMarkers : 1;

	FVector GetBaseLocalArmOffset() const;

	float GetBaseArmLength() const;

	float GetBaseCameraFov() const;

	AJoyPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	void ProcessViewRotation(float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot);

	void ResetCameraToPlayer(float BlendTime) const;

	virtual void Tick(float DeltaSeconds) override;

	virtual void InitializeFor(class APlayerController* PC) override;

	virtual void SetViewTarget(class AActor* NewViewTarget,
		FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams()) override;

	void SetViewTargetWithCurveBlend(AActor* NewViewTarget, TObjectPtr<UCurveFloat> BlendCurve,
		bool bEnableUpdateCameraConfig, FViewTargetTransitionParams TransitionParams = {});

	mutable FOnViewTargetBlendComplete OnViewTargetBlendComplete;

	UPROPERTY(BlueprintAssignable)
	FOnCameraModifyFinishedDelegate OnCameraModifyEndDelegate;

	UFUNCTION(BlueprintCallable, Category = "Camera", DisplayName = "获取相机在角色上默认挂点位置")
	FVector GetCharacterCameraBase(const AActor* Target) const;

	UFUNCTION(BlueprintCallable, Category = "Camera", DisplayName = "获取相机在角色上脸部挂点位置")
	FVector GetCharacterFaceBase(const AActor* Target);

	FVector GetCharacterHeadLocation(const AActor* Character) const;

	FVector GetNegativeGravityNormal() const;

	/**
	 * 获取当前相机弹簧臂旋转
	 */
	FRotator GetCurrentCameraArmCenterRotation(AActor* InViewTarget) const;

	/**
	 * 获取当前相机弹簧臂长
	 */
	float GetCurrentArmLength(AActor* InViewTarget) const;

	FViewTargetCameraInfo* GetViewTargetCameraInfo(const AActor* InViewTarget);

	/**
	 * 获取当前相机 Fov
	 */
	float GetCurrentCameraFov(AActor* InViewTarget) const;

	/**
	 * 获取世界坐标系下相机臂偏移
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera Modify", DisplayName = "获取当前相机臂中心偏移值")
	FVector GetCurrentArmCenterOffset(AActor* InViewTarget) const;

	FVector GetBaseArmCenterLocalOffset() const;

	UFUNCTION(BlueprintCallable, DisplayName = "是否处于队员切换期间")
	bool IsDuringMemberSwitching() const;

	bool IsCurrentViewTarget(const AActor* TestViewTarget) const;

	bool IsPendingViewTarget(const AActor* TestViewTarget) const;

protected:
	virtual void DoUpdateCamera(float DeltaTime) override;

	virtual void InternalUpdateCamera(float DeltaTime);

	virtual void ClearUnusedViewTargets();

	virtual void UpdateViewTargetPose();

	virtual bool NeedUpdateViewTarget(AActor* InViewTarget, const FViewTargetCameraInfo& CameraInfo) const;

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

	void SetConfigs(const TMap<EJoyCameraBasic, float>& Config);

	void ApplyConfig();

	void StartFade(float InFadeDuration, bool bFadeArmLengthRange, bool bInFadeArmLength, bool bInFadeArmRotationPitch,
		bool bInFadeArmRotationYaw, bool bInFadeLocalArmCenterOffset, bool bInFadeFov,
		bool bIgnoreTimeDilationDuringFading = true, bool bOverrideCameraInput = false);

public:
	EJoyCameraBlendType BlendViewType = EJoyCameraBlendType::Default;

	EJoyCameraBlendType GetBlendViewType() const;

	const FCameraConfigDescription& GetCameraConfigDescription() const;

	void SetBlendViewType(EJoyCameraBlendType InBlendViewWay);

	void RemoveBlendViewWay(EJoyCameraBlendType InBlendViewWay);

	void ResetBlendViewWay();

	void AddNewViewTarget(AActor* NewViewTarget);

	float GetBaseMinArmLength() const;

	float GetBaseMaxArmLength() const;

	float GetArmCenterLagRecoverSpeed() const
	{
		return ArmCenterLagRecoverSpeed;
	}

	float GetArmRotatorLagRecoverSpeed() const
	{
		return ArmRotationLagRecoverSpeed;
	}

	float GetArmCenterLagMaxDistanceXY() const
	{
		return ArmCenterLagMaxHorizontalDistance;
	}

	float GetArmCenterLagMaxDistanceZ() const
	{
		return ArmCenterLagMaxVerticalDistance;
	}

	float GetArmCenterSwitchTime() const
	{
		return ViewTargetSwitchTime;
	}

	bool CameraLagEnable() const
	{
		return bEnableCameraLag;
	}

	UJoyCameraModifierController* GetCameraModifier(AActor* InActor);

	UPROPERTY()
	FRotator BaseSpaceTransform;

	UPROPERTY(EditAnywhere, Category = "Joy|Camera|Pitch")
	bool bLimitYawAngle = false;

	float DeltaTimeThisFrame{0.f};

	float DeltaTimeThisFrame_IgnoreTimeDilation{0.f};

protected:
	bool bMoveInput = false;

	float FadingTarget_ArmPitch{0.f};

	float FadingTarget_ArmYaw{0.f};

private:
	/** ****************** Camera Configs Begin ****************** */
	// 相机基础臂长
	float BaseArmLength = 350;
	// 最小相机臂长
	float MinArmLength = 50;
	// 最大相机臂长
	float MaxArmLength = 1000;
	// 相机叠加臂长
	float OverlayArmLength = 0;
	// 相机 FOV
	float BaseFov = 90;
	// 相机偏移
	float ArmCenterOffsetX{0};

	float ArmCenterOffsetY{0};

	float ArmCenterOffsetZ{0};

	// 相机臂中心切换过度时间
	float ViewTargetSwitchTime{1.0};
	// 角色移动时相机臂中心滞后恢复速度
	float ArmCenterLagRecoverSpeed{1.};
	float ArmRotationLagRecoverSpeed{1.};
	// 角色移动时相机臂中心滞后最大距离
	float ArmCenterLagMaxHorizontalDistance{0};
	float ArmCenterLagMaxVerticalDistance{0};
	// 角色移动时相机臂长度回弹设置

	float MinArmPitch = -87;
	float MaxArmPitch = 87;

	bool bEnableCameraLag{false};

	void BlendViewFunc_Default(FMinimalViewInfo& A, FMinimalViewInfo& B, float T);

	UFUNCTION()
	void OnViewTargetChanged(APlayerController* PC, AActor* OldViewTarget, AActor* NewViewTarget);

	void BlendViewFunc_ArcCurve(FMinimalViewInfo& A, FMinimalViewInfo& B, float T);

	void BlendViewFunc_KeepViewDirection(FMinimalViewInfo& A, FMinimalViewInfo& B, float T);
	/** 视角混合切换 End */

	static FRotator GetViewTargetViewRotation(const AActor* InViewTarget);

	void BlendViewInfo(FMinimalViewInfo& A, FMinimalViewInfo& B, float T);

	void UpdateCameraConfigs(UJoyCameraComponent* CameraComponent);

	void UpdateCameraControllers(float DeltaTime);

	void ResetModifiedMarkers();

	void SyncDesireCameraData(float DeltaTime);

	void UpdateActorTransform(float DeltaTime);

	void UpdateArmLocation(float DeltaTime);

	void SetRotationInternal(const AActor* InViewTarget, FRotator Rotator);

	float ComputeTimeDilation(float DeltaTime) const;

	void EndCurrentCameraFadingProcess();

	UPROPERTY()
	TObjectPtr<UJoyCameraInputController> CameraInputController{nullptr};

	UPROPERTY()
	TObjectPtr<UJoyCameraConfigController> CameraConfigController{nullptr};

	UPROPERTY()
	FMultiViewTargetCameraManager MultiViewTargetCameraManager{};

	UPROPERTY()
	TWeakObjectPtr<UCurveFloat> BlendViewCurve{nullptr};

	UPROPERTY()
	FInputOverrideDescription InputOverrideDescription{};

	UPROPERTY()
	FCameraConfigDescription CameraConfigDescription{};

	UPROPERTY()
	FCameraConfigFadingDescription CameraConfigFadingDescription{};
};
