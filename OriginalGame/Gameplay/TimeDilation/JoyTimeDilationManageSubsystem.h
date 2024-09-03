#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "JoyTimeDilationManageSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FFGTimeDilationHandle
{
	GENERATED_BODY()

	FFGTimeDilationHandle() = default;
	explicit FFGTimeDilationHandle(int64 Seq);

	friend static bool operator==(FFGTimeDilationHandle const& L, FFGTimeDilationHandle const& R)
	{
		return L.SequenceID == R.SequenceID;
	}

	friend static bool operator!=(FFGTimeDilationHandle const& L, FFGTimeDilationHandle const& R)
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

DECLARE_DELEGATE_TwoParams(FFGOnTimeDilationApply, const FFGTimeDilationHandle&, bool);

USTRUCT()
struct FFGTimeDilationHandleCache
{
	GENERATED_BODY()

	FFGTimeDilationHandleCache() = default;
	FFGTimeDilationHandleCache(int64 Seq, float Dilation);
	FFGTimeDilationHandleCache(FFGTimeDilationHandle Handle, float Dilation);

	friend static bool operator==(FFGTimeDilationHandleCache const& L, FFGTimeDilationHandle const& R)
	{
		return L.Handle == R;
	}

	friend static bool operator==(FFGTimeDilationHandle const& L, FFGTimeDilationHandleCache const& R)
	{
		return L == R.Handle;
	}

	UPROPERTY()
	FFGTimeDilationHandle Handle{};

	UPROPERTY()
	float TimeDilation{};
};

USTRUCT()
struct FFGTimeDilationManageCache
{
	GENERATED_BODY()

	UPROPERTY()
	float CurrentDilation{1.0f};

	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor{};

	UPROPERTY()
	TArray<FFGTimeDilationHandleCache> HandleCaches{};
};

USTRUCT()
struct FFGTimeDilationRequestCache
{
	GENERATED_BODY()

	UPROPERTY()
	FFGTimeDilationHandle Handle{};

	UPROPERTY()
	TWeakObjectPtr<AActor> Actor{};

	FString Description = TEXT("");

	FFGOnTimeDilationApply OnApplyCallback{};

	bool bIsAdd{};
	bool bIsGlobal{};
	bool bOverride{};
	float Dilation{1.0f};
	bool bUseAbsoluteValue{};
	bool bSuccess{};

	friend static bool operator==(FFGTimeDilationRequestCache const& L, FFGTimeDilationHandle const& R)
	{
		return L.Handle == R;
	}

	friend static bool operator==(FFGTimeDilationHandle const& L, FFGTimeDilationRequestCache const& R)
	{
		return L == R.Handle;
	}
};

/**
 * @brief 处理时间膨胀相关逻辑的子系统，目的是方便全局统一获取正确的时间膨胀系数值。
 *        该子系统的 tick 逻辑会在一般的 actor 和 component 的 tick 之后进行，新设置的时间膨胀会在下一帧生效。
 */
UCLASS(DisplayName = "FG Time Dilation Manage Subsystem")
class OG3_API UJoyTimeDilationManageSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	static UJoyTimeDilationManageSubsystem* Get(const UWorld* World);
	static UJoyTimeDilationManageSubsystem* GetTimeDilationManageSubsystem(const UObject* WorldContextObject);

	//~FTickableGameObject begin
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual TStatId GetStatId() const override;
	//~FTickableGameObject end

	/**
	 * 添加一个全局的时间膨胀系数，所有当前正在生效的时间膨胀系数会相乘得出当前应该生效的时间膨胀系数。
	 */
	UFUNCTION(BlueprintCallable)
	FFGTimeDilationHandle AddGlobalTimeDilation(float TimeDilation, const FString Description = "");
	FFGTimeDilationHandle AddGlobalTimeDilationWithCallback(
		float TimeDilation, FFGOnTimeDilationApply OnApply, const FString Description = "");

	/**
	 * 覆盖全局的时间膨胀系数，所有之前正在生效的时间膨胀系数会失效，只有本次新覆盖的时间膨胀系数会生效。
	 */
	UFUNCTION(BlueprintCallable)
	FFGTimeDilationHandle OverrideGlobalTimeDilation(float TimeDilation, const FString Description = "");
	FFGTimeDilationHandle OverrideGlobalTimeDilationWithCallback(
		float TimeDilation, FFGOnTimeDilationApply OnApply, const FString Description = "");

	/**
	 * 添加一个角色的时间膨胀系数，所有当前正在生效的时间膨胀系数会相乘得出当前应该生效的时间膨胀系数。
	 */
	UFUNCTION(BlueprintCallable)
	FFGTimeDilationHandle AddActorTimeDilation(AActor* Actor, float TimeDilation, const FString Description = "");
	FFGTimeDilationHandle AddActorTimeDilationWithCallback(
		AActor* Actor, float TimeDilation, FFGOnTimeDilationApply OnApply, const FString Description = "");

	/**
	 * 覆盖角色的时间膨胀系数，所有之前正在生效的时间膨胀系数会失效，只有本次新覆盖的时间膨胀系数会生效。
	 * 输入参数，可以直接将目标的时间膨胀系数改为输入的值。
	 * 有两种形式，透过 bUseAbsoluteValue 进行控制：
	 * - 如果为 true，使用绝对时间膨胀系数，若当前世界时间膨胀系数为 0.1，且将角色时间膨胀系数设为
	 * 1，则实际角色时间膨胀系数为 10
	 * - 如果为 false，则使用相对时间膨胀系数，无论当前时间膨胀系数是多少，都会将角色时间膨胀系数设为指定值
	 *
	 * @param Actor 目标 actor
	 * @param TimeDilation 时间膨胀系数
	 * @param bUseAbsoluteValue 是否是绝对时间膨胀系数
	 * @return 时间膨胀系数句柄
	 */
	UFUNCTION(BlueprintCallable)
	FFGTimeDilationHandle OverrideActorTimeDilation(
		AActor* Actor, float TimeDilation, bool bUseAbsoluteValue = true, const FString Description = "");
	FFGTimeDilationHandle OverrideActorTimeDilationWithCallback(AActor* Actor, float TimeDilation,
		bool bUseAbsoluteValue, FFGOnTimeDilationApply OnApply, const FString Description = "");

	/**
	 * 更新一个全局的时间膨胀系数。
	 */
	UFUNCTION(BlueprintCallable)
	bool UpdateGlobalTimeDilation(FFGTimeDilationHandle const& Handle, float TimeDilation);

	/**
	 * 更新一个角色的时间膨胀系数。
	 */
	UFUNCTION(BlueprintCallable)
	bool UpdateActorTimeDilation(AActor* Actor, FFGTimeDilationHandle const& Handle, float TimeDilation);

	/**
	 * 移除一个全局的时间膨胀系数。
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveGlobalTimeDilation(FFGTimeDilationHandle const& Handle);
	void RemoveGlobalTimeDilationWithCallback(FFGTimeDilationHandle const& Handle, FFGOnTimeDilationApply OnApply);

	/**
	 * 移除一个角色的时间膨胀系数。
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveActorTimeDilation(AActor* Actor, FFGTimeDilationHandle const& Handle);
	void RemoveActorTimeDilationWithCallback(
		AActor* Actor, FFGTimeDilationHandle const& Handle, FFGOnTimeDilationApply OnApply);

	UFUNCTION(BlueprintCallable)
	float GetGlobalTimeDilation() const;
	float GetGlobalTimeDilationOfHandle(FFGTimeDilationHandle const& Handle) const;

private:
	void SetGlobalTimeDilationByCache(FFGTimeDilationManageCache& Cache) const;
	bool AddGlobalTimeDilationImpl(FFGTimeDilationHandle Handle, float TimeDilation, bool bOverride);
	bool AddActorTimeDilationImpl(
		FFGTimeDilationHandle Handle, AActor* Actor, float TimeDilation, bool bOverride, bool bUseAbsoluteValue);
	bool RemoveGlobalTimeDilationImpl(FFGTimeDilationHandle Handle);
	bool RemoveActorTimeDilationImpl(FFGTimeDilationHandle Handle, AActor* Actor);

	FFGTimeDilationHandle NewAddTimeDilationRequestCache(bool bIsGlobal, bool bOverride, AActor* Actor,
		float TimeDilation, bool bUseAbsoluteValue, FFGOnTimeDilationApply OnApply, const FString& Description = "");
	void NewRemoveTimeDilationRequestCache(
		FFGTimeDilationHandle Handle, bool bIsGlobal, AActor* Actor, FFGOnTimeDilationApply OnApply);

	static void ReCalculateCacheTimeDilation(FFGTimeDilationManageCache& Cache);

	UPROPERTY()
	mutable int64 SequenceNumber{};

	UPROPERTY()
	TArray<FFGTimeDilationRequestCache> RequestCaches{};

	UPROPERTY()
	TMap<uint32, FFGTimeDilationManageCache> ActorCaches{};

	UPROPERTY()
	FFGTimeDilationManageCache GlobalCache{};
};