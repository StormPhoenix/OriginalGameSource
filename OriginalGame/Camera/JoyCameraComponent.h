#pragma once
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"

#include "JoyCameraComponent.generated.h"

struct FGameplayTag;
class UJoyCameraMode;
class UJoyCameraModeStack;
class AJoyPlayerCameraManager;

template <typename T>
struct TDataRecord
{
	T Data;

	void Set(T NewData)
	{
		Data = NewData;
		bValid = true;
	}

	bool CheckValid() const
	{
		return bValid;
	}

	void Reset()
	{
		bValid = false;
	}

private:
	bool bValid{false};
};

USTRUCT()
struct FCameraData
{
	GENERATED_BODY()

	TDataRecord<FVector> ArmLocation{};

	TDataRecord<FVector> ViewLocation{};

	TDataRecord<FRotator> ViewRotation{};

	TDataRecord<FVector> AvatarLocation{};

	void Clean()
	{
		ArmLocation.Reset();
		ViewLocation.Reset();
		ViewRotation.Reset();
		AvatarLocation.Reset();
	}
};

USTRUCT(BlueprintType)
struct FJoyCameraIDHandle
{
	GENERATED_BODY()

	FJoyCameraIDHandle() = default;

	explicit FJoyCameraIDHandle(int64 Seq);

	friend static bool operator==(FJoyCameraIDHandle const& L, FJoyCameraIDHandle const& R)
	{
		return L.SequenceID == R.SequenceID;
	}

	friend static bool operator!=(FJoyCameraIDHandle const& L, FJoyCameraIDHandle const& R)
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
struct FJoyAddCameraIDRequestCache
{
	GENERATED_BODY()

	UPROPERTY()
	FJoyCameraIDHandle Handle{};

	UPROPERTY()
	FName CameraID;

	friend static bool operator==(FJoyAddCameraIDRequestCache const& L, FJoyCameraIDHandle const& R)
	{
		return L.Handle == R;
	}

	friend static bool operator==(FJoyCameraIDHandle const& L, FJoyAddCameraIDRequestCache const& R)
	{
		return L == R.Handle;
	}
};

UCLASS(HideCategories = (Mobility, Rendering, LOD), Blueprintable, ClassGroup = Camera,
	meta = (BlueprintSpawnableComponent))
class ORIGINALGAME_API UJoyCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

	friend class UJoyCameraMode;
	friend class UJoyCameraMode_ThirdPerson;
	friend class AJoyPlayerCameraManager;

public:
	UJoyCameraComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	// Returns the camera component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Joy|Camera")
	static UJoyCameraComponent* FindCameraComponent(const AActor* Actor)
	{
		return (Actor ? Actor->FindComponentByClass<UJoyCameraComponent>() : nullptr);
	}

	// Returns the target actor that the camera is looking at.
	virtual AActor* GetTargetActor() const
	{
		return GetOwner();
	}

	// Gets the tag associated with the top layer and the blend weight of it
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

	void PushCameraMode(TSubclassOf<UJoyCameraMode> CameraModeClass, bool bBlend = true) const;

	void PopCameraMode(TSubclassOf<UJoyCameraMode> CameraModeClass, bool bBlend = true) const;

	UJoyCameraMode* GetCameraModeInstance(TSubclassOf<UJoyCameraMode> GameModeClass) const;

	TSubclassOf<UJoyCameraMode> GetTopCameraModeClass() const;

	void FrozeCamera();

	const FMinimalViewInfo& GetFrozenView() const;

	/** ========================= 相机镜头配置 ============================ */
	UFUNCTION(BlueprintCallable, Category = "Joy|Camera")
	FJoyCameraIDHandle PushCameraConfig(FName CameraID);

	UFUNCTION(BlueprintCallable, Category = "Joy|Camera")
	bool RemoveCameraConfig(FJoyCameraIDHandle Handler);

	UFUNCTION(BlueprintCallable, Category = "Joy|Camera")
	bool RemoveCameraConfigByID(FName CameraID);

private:
	bool bIsCameraConfigDirty{false};

public:
	TArray<FName> GetCameraIDs() const;

	bool IsCameraConfigDirty() const;

	void RefreshCameraConfig();
	/** ========================= END ============================ */

protected:
	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	virtual void UpdateCameraModes();

	UPROPERTY()
	TObjectPtr<UJoyCameraModeStack> CameraModeStack;

	UPROPERTY()
	FCameraData CameraDataThisFrame{};

private:
	UPROPERTY()
	TArray<FJoyAddCameraIDRequestCache> CameraIDRequestQueue;

	UPROPERTY()
	TSet<FName> CameraIDs;

	UPROPERTY()
	bool bCameraFrozen = false;

	UPROPERTY()
	FMinimalViewInfo FrozenCameraView;

	UPROPERTY()
	mutable int64 SequenceNumber{0};
};
