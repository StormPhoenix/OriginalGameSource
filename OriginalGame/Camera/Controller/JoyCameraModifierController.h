#pragma once
#include "JoyCameraControllerBase.h"

#include "JoyCameraModifierController.generated.h"

struct FVirtualCamera;

USTRUCT(BlueprintType)
struct FCameraAdaptiveOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "自适应开关")
	bool bAdaptiveOption = {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "是否与镜头同向",
		meta = (EditCondition = "bAdaptiveOption", EditConditionHides))
	bool bSync{true};
};

USTRUCT(BlueprintType)
struct FCameraLocalOffsetSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "是否修改")
	bool bModified{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "设置偏移",
		meta = (EditCondition = "bModified", EditConditionHides))
	FVector LocalArmOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Y 轴自适应选项",
		meta = (EditCondition = "bModified", EditConditionHides))
	FCameraAdaptiveOption LocalArmOffsetYAdaptiveOption{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "设置额外偏移")
	FVector LocalArmOffsetAdditional = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "混合完毕后是否复位")
	bool bReset = true;
};

USTRUCT(BlueprintType)
struct FCameraWorldOffsetAdditionalSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "是否修改")
	bool bModified{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "设置偏移",
		meta = (EditCondition = "bModified", EditConditionHides))
	FVector WorldArmOffsetAdditional{FVector::ZeroVector};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "混合完毕后是否复位",
		meta = (EditCondition = "bModified", EditConditionHides))
	bool bReset{true};
};

USTRUCT(BlueprintType)
struct FCameraLocalRotationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "是否修改 Yaw")
	bool bModifyYaw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "设置 Yaw",
		meta = (EditCondition = "bModifyYaw", EditConditionHides))
	float Yaw = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Yaw 自适应选项",
		meta = (EditCondition = "bModifyYaw", EditConditionHides))
	FCameraAdaptiveOption RelativeYawAdaptiveOption{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "混合完毕后是否复位 Yaw",
		meta = (EditCondition = "bModifyYaw", EditConditionHides))
	bool bResetYaw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "是否修改 Pitch")
	bool bModifyPitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "设置 Pitch",
		meta = (EditCondition = "bModifyPitch", EditConditionHides))
	float Pitch = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "混合完毕后是否复位 Pitch",
		meta = (EditCondition = "bModifyPitch", EditConditionHides))
	bool bResetPitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "是否修改 Roll")
	bool bModifyRoll = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "设置 Roll",
		meta = (EditCondition = "bModifyRoll", EditConditionHides))
	float Roll = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Roll 自适应选项",
		meta = (EditCondition = "bModifyRoll", EditConditionHides))
	FCameraAdaptiveOption RelativeRollAdaptiveOption{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "混合完毕后是否复位 Roll",
		meta = (EditCondition = "bModifyRoll", EditConditionHides))
	bool bResetRoll = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "是否使用曲线控制相机混合 Alpha")
	bool bUseCurveControlCameraBlendInRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "相机混合速率曲线",
		meta = (EditCondition = "bUseCurveControlCameraBlendInRotation", EditConditionHides))
	TObjectPtr<UCurveFloat> BlendInCameraRotationAlphaCurve{};
};

USTRUCT(BlueprintType)
struct FCameraWorldRotationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "是否修改")
	bool bModified = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "设置旋转",
		meta = (EditCondition = "bModified", EditConditionHides))
	FRotator ArmRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "混合完毕后是否复位",
		meta = (EditCondition = "bModified", EditConditionHides))
	bool bReset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "额外旋转增量")
	FRotator ArmRotationAdditional = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FCameraFovSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "是否修改")
	bool bModified{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "设置 Fov",
		meta = (EditCondition = "bModified", EditConditionHides))
	float CameraFov{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "是否曲线控制 Fov",
		meta = (EditCondition = "bModified", EditConditionHides))
	bool bCameraFovCurveControl{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Fov 曲线",
		meta = (EditCondition = "bModified && bCameraFovCurveControl", EditConditionHides))
	TObjectPtr<UCurveFloat> CameraFovCurve{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "混合完毕后是否复位",
		meta = (EditCondition = "bModified", EditConditionHides))
	bool bReset{true};
};

USTRUCT(BlueprintType)
struct FCameraArmLengthSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "是否修改")
	bool bModified = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "设置基础臂长",
		meta = (EditCondition = "bModified", EditConditionHides))
	float ArmLength = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "臂长变化增量")
	float ArmLengthAdditional{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "是否曲线控制臂长",
		meta = (EditCondition = "bModified", EditConditionHides))
	bool bArmLengthCurveControl{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "臂长曲线",
		meta = (EditCondition = "bModified && bArmLengthCurveControl", EditConditionHides))
	TObjectPtr<UCurveFloat> ArmLengthCurve{nullptr};

	/** 是否需要重置相机臂长 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "混合完毕后是否复位")
	bool bReset{true};
};

USTRUCT(BlueprintType)
struct FCameraModifiers
{
	GENERATED_BODY()

	// 是否还原 View Target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "当镜头修正结束", DisplayName = "是否重置镜头到主控对象")
	bool bResetViewTarget = false;

	// 需要多少时间恢复到原先 ViewTarget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "当镜头修正结束", DisplayName = "重置镜头用时",
		meta = (EditCondition = "bResetViewTarget", EditConditionHides))
	float TimeToResetViewTarget = 0.;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Arm Length 设置")
	FCameraArmLengthSettings ArmLengthSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "FOV 设置")
	FCameraFovSettings CameraFovSettings{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "局部 Arm Offset 设置")
	FCameraLocalOffsetSettings LocalOffsetSettings{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "世界 Arm Offset 设置")
	FCameraWorldOffsetAdditionalSettings WorldOffsetAdditionalSettings{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "局部 Arm Rotation 设置")
	FCameraLocalRotationSettings LocalRotationSettings{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "世界 Arm Rotation 设置)")
	FCameraWorldRotationSettings WorldRotationSettings{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "旋转", DisplayName = "是否曲线控制旋转")
	bool bArmRotationCurveControl{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "旋转", DisplayName = "旋转曲线",
		meta = (EditCondition = "bArmRotationCurveControl", EditConditionHides))
	TObjectPtr<UCurveFloat> ArmRotationCurve{nullptr};

	// 禁用相机输入
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", DisplayName = "相机混合过程中是否禁用键盘鼠标输入")
	bool bOverrideCameraInput = false;

	// 忽略时间膨胀
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", DisplayName = "是否忽略子弹时间")
	bool bIgnoreTimeDilation = false;
};

UENUM()
enum class EBlendState
{
	None,
	BlendIn,
	Loop,
	BlendOut,
};

UENUM()
enum class EResetWay
{
	KeepUnchanged = 0 UMETA(Tooltip = "保持不变"),
	ResetToInitial = 1 UMETA(Tooltip = "重置为初始值"),
	ResetToSpecify = 2 UMETA(Tooltip = "重置为默认值"),
};

USTRUCT()
struct FCameraFadeOutData
{
	GENERATED_BODY()

	bool bModifyArmLength{false};

	float ArmLength{0.};

	bool bModifyArmOffset{false};

	FVector WorldCameraOffsetAdditional{FVector::ZeroVector};

	bool bModifyFov{false};

	float Fov{0};

	bool bModifyArmPitch{false};

	bool bModifyArmYaw{false};

	bool bModifyArmRoll{false};

	UPROPERTY()
	FRotator ArmRotation{FRotator::ZeroRotator};
};

/**
 * 计算相机位姿时依赖的外部相机数据，在运行时随时会变化
 */
USTRUCT()
struct FExternalDependencyCameraData
{
	GENERATED_BODY()

public:
	void UpdateCameraData(const FQuat& InPawnFaceViewQuat)
	{
		if (!bPawnFaceViewQuatLocked)
		{
			PawnFaceViewQuat = InPawnFaceViewQuat;
		}

		bExternalDataValid = true;
	}

	FQuat GetPawnFaceViewQuat() const
	{
		return PawnFaceViewQuat;
	}

	void LockPawnFaceViewQuat()
	{
		bPawnFaceViewQuatLocked = true;
	}

	void Clean()
	{
		bExternalDataValid = false;
		bPawnFaceViewQuatLocked = false;
	}

	bool IsValid() const
	{
		return bExternalDataValid;
	}

private:
	// 角色面朝方向
	FQuat PawnFaceViewQuat;
	bool bPawnFaceViewQuatLocked = false;

	bool bExternalDataValid = false;
};

USTRUCT(BlueprintType)
struct FCameraModifyHandle
{
	GENERATED_BODY()

	FCameraModifyHandle() = default;

	explicit FCameraModifyHandle(int64 Seq);

	friend static bool operator==(FCameraModifyHandle const& L, FCameraModifyHandle const& R)
	{
		return L.SequenceID == R.SequenceID;
	}

	friend static bool operator!=(FCameraModifyHandle const& L, FCameraModifyHandle const& R)
	{
		return L.SequenceID != R.SequenceID;
	}

	bool IsValid() const
	{
		return SequenceID > 0;
	}

	UPROPERTY()
	int64 SequenceID{};
};

USTRUCT()
struct FCameraModifySpec
{
	GENERATED_BODY()

	bool bArmRotationModifyInterrupted{false};

	FCameraModifiers CameraModifiers{};

	void Clear()
	{
		bArmRotationModifyInterrupted = false;
	}
};

UCLASS()
class ORIGINALGAME_API UJoyCameraModifierController : public UJoyCameraControllerBase
{
	GENERATED_BODY()

	friend class AJoyPlayerCameraManager;

public:
	UJoyCameraModifierController();

	FCameraModifyHandle ApplyCameraModify(float Duration, float BlendInTime, float BlendOutTime,
		FCameraModifiers const& CameraModifiers, bool bNeedManualBreak = false);

	FCameraModifyHandle GetLastModifierHandle() const;

	void ApplyCameraModify_Immediately(const FCameraModifiers& InCameraModifiers);

	void BreakModifier(FCameraModifyHandle ModifyHandler = FCameraModifyHandle(0));

	void EndModify();

	// 修改完毕后尝试重置 ViewTarget 到原始值
	void ResetViewTarget();

	virtual void UpdateInternal(float DeltaSeconds) override;

	bool IsModifiedAndNeedUpdate() const;

	void SetModifyTarget(AActor* ModifyTarget);

	AActor* GetModifyTarget() const
	{
		return ModifiedViewTarget;
	}

private:
	void StartModifyFadeOut();

	void MakeRestoreCameraData(FVirtualCamera& RestoreCamera) const;

	void UpdateModifyFadeOut(float DeltaSeconds);

	void UpdateModifiers(float DeltaSeconds);

	void UpdateArmLengthModifier(EBlendState State, float Alpha);

	void UpdateLocalArmCenterOffsetModifier(EBlendState State, float Alpha) const;

	void UpdateWorldArmCenterOffsetModifier(EBlendState State, float Alpha) const;

	void UpdateArmRotationModifier(EBlendState State, float Alpha);

	void UpdateFovModifier(EBlendState State, float Alpha);

	bool IsCameraRotationModified() const;

	bool IsCameraOffsetModified() const;

private:
	// 从更改开始到现在经过的时间
	float ModifyElapsedTime = 0;

	// 从淡出开始到现在经过的时间
	float ModifyBlendOutElapsedTime = 0;

	// 修改起作用的淡入时间
	float ModifyBlendInTime = 0;

	// 修改起作用的淡出时间
	float ModifyBlendOutTime = 0;

	// 修改值持续时间
	float ModifyDuration = 0;

	// 是否需要手动中断 Duration
	bool bNeedManualBreakModify = false;

	// 是否需要已经手动中断 Duration
	bool bHasManualBreakModify = false;

	// 是否进入了淡出状态
	bool bIsModifyBlendOut = false;

	// 由 CameraParaModifier 自己判断是否要修改弹簧臂长的值
	bool bNeedModifyAdditionalArmLength = false;

	// 由 CameraParaModifier 自己判断是否要修改相机 Offset 值
	bool bNeedModifyAdditionalLocalCameraOffset = false;

	// 由 CameraParaModifier 自己判断是否要修改相机 Rotation 的值
	bool bNeedModifyAdditionalCameraRotation = false;

	// 是否需要修改相机 Fov
	bool bNeedModifyFov{false};

	bool bNeedModifyFadeOut{false};

	bool bIgnoreTimeDilation{false};

	float GetFinalArmLength() const;

	FVector GetFinalLocalArmOffset() const;

	FVector GetFinalWorldArmOffset() const;

	// 获取相机弹簧臂最终方向
	FRotator GetFinalArmRotation();

	float GetFinalFov() const;

	// 当前值是否处于修改状态
	bool bIsModified = false;

	// 是否会附加到新 View Target
	bool bResetViewTarget = false;

	// 需要多少时间恢复到原先 ViewTarget
	float TimeToResetViewTarget = 0.;

	UPROPERTY()
	FCameraFadeOutData ModifyFadeOutData;

	UPROPERTY()
	EBlendState CurrentBlendState = EBlendState::None;

	// 相机臂旋转恢复速度
	float ModifyArmRotationLagSpeed = 1.;

	// 相机臂长度恢复速度
	float ModifyArmLengthLagSpeed = 1.;

	UPROPERTY()
	FExternalDependencyCameraData ExternalDependencyCameraData;

	UPROPERTY()
	TObjectPtr<AActor> ModifiedViewTarget = nullptr;

	UPROPERTY()
	mutable int64 SequenceNumber{0};

	UPROPERTY()
	FCameraModifyHandle LastModifierHandle{0};

	UPROPERTY()
	FCameraModifySpec CurrentCameraModifySpec{};

	float CurrentRawBlendAlpha{0.f};
};
