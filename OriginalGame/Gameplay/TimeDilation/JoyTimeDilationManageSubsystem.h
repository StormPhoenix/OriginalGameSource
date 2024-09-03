#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "JoyTimeDilationManageSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FJoyTimeDilationHandle
{
	GENERATED_BODY()

	FJoyTimeDilationHandle() = default;
	explicit FJoyTimeDilationHandle(int64 Seq);

	friend static bool operator==(FJoyTimeDilationHandle const& L, FJoyTimeDilationHandle const& R)
	{
		return L.SequenceID == R.SequenceID;
	}

	friend static bool operator!=(FJoyTimeDilationHandle const& L, FJoyTimeDilationHandle const& R)
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

DECLARE_DELEGATE_TwoParams(FFGOnTimeDilationApply, const FJoyTimeDilationHandle&, bool);

USTRUCT()
struct FJoyTimeDilationHandleCache
{
	GENERATED_BODY()

	FJoyTimeDilationHandleCache() = default;
	FJoyTimeDilationHandleCache(int64 Seq, float Dilation);
	FJoyTimeDilationHandleCache(FJoyTimeDilationHandle Handle, float Dilation);

	friend static bool operator==(FJoyTimeDilationHandleCache const& L, FJoyTimeDilationHandle const& R)
	{
		return L.Handle == R;
	}

	friend static bool operator==(FJoyTimeDilationHandle const& L, FJoyTimeDilationHandleCache const& R)
	{
		return L == R.Handle;
	}

	UPROPERTY()
	FJoyTimeDilationHandle Handle{};

	UPROPERTY()
	float TimeDilation{};
};

USTRUCT()
struct FJoyTimeDilationManageCache
{
	GENERATED_BODY()

	UPROPERTY()
	float CurrentDilation{1.0f};

	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor{};

	UPROPERTY()
	TArray<FJoyTimeDilationHandleCache> HandleCaches{};
};

USTRUCT()
struct FJoyTimeDilationRequestCache
{
	GENERATED_BODY()

	UPROPERTY()
	FJoyTimeDilationHandle Handle{};

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

	friend static bool operator==(FJoyTimeDilationRequestCache const& L, FJoyTimeDilationHandle const& R)
	{
		return L.Handle == R;
	}

	friend static bool operator==(FJoyTimeDilationHandle const& L, FJoyTimeDilationRequestCache const& R)
	{
		return L == R.Handle;
	}
};

/**
 * @brief 处理时间膨胀相关逻辑的子系统，目的是方便全局统一获取正确的时间膨胀系数值。
 *        该子系统的 tick 逻辑会在一般的 actor 和 component 的 tick 之后进行，新设置的时间膨胀会在下一帧生效。
 */
UCLASS(DisplayName = "FG Time Dilation Manage Subsystem")
class ORIGINALGAME_API UJoyTimeDilationManageSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
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
	FJoyTimeDilationHandle AddGlobalTimeDilation(float TimeDilation, const FString Description = "");
	FJoyTimeDilationHandle AddGlobalTimeDilationWithCallback(
		float TimeDilation, FFGOnTimeDilationApply OnApply, const FString Description = "");

	/**
	 * 覆盖全局的时间膨胀系数，所有之前正在生效的时间膨胀系数会失效，只有本次新覆盖的时间膨胀系数会生效。
	 */
	UFUNCTION(BlueprintCallable)
	FJoyTimeDilationHandle OverrideGlobalTimeDilation(float TimeDilation, const FString Description = "");
	FJoyTimeDilationHandle OverrideGlobalTimeDilationWithCallback(
		float TimeDilation, FFGOnTimeDilationApply OnApply, const FString Description = "");

	/**
	 * 添加一个角色的时间膨胀系数，所有当前正在生效的时间膨胀系数会相乘得出当前应该生效的时间膨胀系数。
	 */
	UFUNCTION(BlueprintCallable)
	FJoyTimeDilationHandle AddActorTimeDilation(AActor* Actor, float TimeDilation, const FString Description = "");
	FJoyTimeDilationHandle AddActorTimeDilationWithCallback(
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
	FJoyTimeDilationHandle OverrideActorTimeDilation(
		AActor* Actor, float TimeDilation, bool bUseAbsoluteValue = true, const FString Description = "");
	FJoyTimeDilationHandle OverrideActorTimeDilationWithCallback(AActor* Actor, float TimeDilation,
		bool bUseAbsoluteValue, FFGOnTimeDilationApply OnApply, const FString Description = "");

	/**
	 * 更新一个全局的时间膨胀系数。
	 */
	UFUNCTION(BlueprintCallable)
	bool UpdateGlobalTimeDilation(FJoyTimeDilationHandle const& Handle, float TimeDilation);

	/**
	 * 更新一个角色的时间膨胀系数。
	 */
	UFUNCTION(BlueprintCallable)
	bool UpdateActorTimeDilation(AActor* Actor, FJoyTimeDilationHandle const& Handle, float TimeDilation);

	/**
	 * 移除一个全局的时间膨胀系数。
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveGlobalTimeDilation(FJoyTimeDilationHandle const& Handle);
	void RemoveGlobalTimeDilationWithCallback(FJoyTimeDilationHandle const& Handle, FFGOnTimeDilationApply OnApply);

	/**
	 * 移除一个角色的时间膨胀系数。
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveActorTimeDilation(AActor* Actor, FJoyTimeDilationHandle const& Handle);
	void RemoveActorTimeDilationWithCallback(
		AActor* Actor, FJoyTimeDilationHandle const& Handle, FFGOnTimeDilationApply OnApply);

	UFUNCTION(BlueprintCallable)
	float GetGlobalTimeDilation() const;
	float GetGlobalTimeDilationOfHandle(FJoyTimeDilationHandle const& Handle) const;

private:
	void SetGlobalTimeDilationByCache(FJoyTimeDilationManageCache& Cache) const;
	bool AddGlobalTimeDilationImpl(FJoyTimeDilationHandle Handle, float TimeDilation, bool bOverride);
	bool AddActorTimeDilationImpl(
		FJoyTimeDilationHandle Handle, AActor* Actor, float TimeDilation, bool bOverride, bool bUseAbsoluteValue);
	bool RemoveGlobalTimeDilationImpl(FJoyTimeDilationHandle Handle);
	bool RemoveActorTimeDilationImpl(FJoyTimeDilationHandle Handle, AActor* Actor);

	FJoyTimeDilationHandle NewAddTimeDilationRequestCache(bool bIsGlobal, bool bOverride, AActor* Actor,
		float TimeDilation, bool bUseAbsoluteValue, FFGOnTimeDilationApply OnApply, const FString& Description = "");
	void NewRemoveTimeDilationRequestCache(
		FJoyTimeDilationHandle Handle, bool bIsGlobal, AActor* Actor, FFGOnTimeDilationApply OnApply);

	static void ReCalculateCacheTimeDilation(FJoyTimeDilationManageCache& Cache);

	UPROPERTY()
	mutable int64 SequenceNumber{};

	UPROPERTY()
	TArray<FJoyTimeDilationRequestCache> RequestCaches{};

	UPROPERTY()
	TMap<uint32, FJoyTimeDilationManageCache> ActorCaches{};

	UPROPERTY()
	FJoyTimeDilationManageCache GlobalCache{};
};