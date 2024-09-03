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
	bool bCameraFrozen = false;

	UPROPERTY()
	FMinimalViewInfo FrozenCameraView;
};
