#include "JoyTimeDilationManageSubsystem.h"

#include "Character/JoyCharacter.h"
#include "JoyLogChannels.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"

FFGTimeDilationHandle::FFGTimeDilationHandle(int64 const Seq) : SequenceID(Seq)
{
}

FFGTimeDilationHandleCache::FFGTimeDilationHandleCache(int64 const Seq, float const Dilation)
	: FFGTimeDilationHandleCache(FFGTimeDilationHandle(Seq), Dilation)
{
}

FFGTimeDilationHandleCache::FFGTimeDilationHandleCache(FFGTimeDilationHandle const Handle, float const Dilation)
	: Handle(Handle), TimeDilation(Dilation)
{
}

UJoyTimeDilationManageSubsystem* UJoyTimeDilationManageSubsystem::Get(const UWorld* World)
{
	if (World)
	{
		return UGameInstance::GetSubsystem<UJoyTimeDilationManageSubsystem>(World->GetGameInstance());
	}

	return nullptr;
}

UJoyTimeDilationManageSubsystem* UJoyTimeDilationManageSubsystem::GetTimeDilationManageSubsystem(
	const UObject* WorldContextObject)
{
	if (UWorld const* World =
		GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return UJoyTimeDilationManageSubsystem::Get(World);
	}

	return nullptr;
}

UWorld* UJoyTimeDilationManageSubsystem::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

void UJoyTimeDilationManageSubsystem::Tick(float)
{
	if (RequestCaches.IsEmpty())
	{
		return;
	}

	TArray<FFGTimeDilationRequestCache> TempRequestCaches;;
	std::swap(RequestCaches, TempRequestCaches);

	for (FFGTimeDilationRequestCache& Req : TempRequestCaches)
	{
		UE_LOG(LogJoyTimeDilation, Log, TEXT("%s"), *Req.Description);

		if (Req.bIsAdd)
		{
			if (Req.bIsGlobal)
			{
				Req.bSuccess = AddGlobalTimeDilationImpl(Req.Handle, Req.Dilation, Req.bOverride);
			}
			else
			{
				Req.bSuccess = AddActorTimeDilationImpl(
					Req.Handle, Req.Actor.Get(), Req.Dilation, Req.bOverride, Req.bUseAbsoluteValue);
			}
		}
		else
		{
			if (Req.bIsGlobal)
			{
				Req.bSuccess = RemoveGlobalTimeDilationImpl(Req.Handle);
			}
			else
			{
				Req.bSuccess = RemoveActorTimeDilationImpl(Req.Handle, Req.Actor.Get());
			}
		}
	}

	for (FFGTimeDilationRequestCache const& Req : TempRequestCaches)
	{
		// ReSharper disable once CppExpressionWithoutSideEffects
		Req.OnApplyCallback.ExecuteIfBound(Req.Handle, Req.bSuccess);
	}
}

bool UJoyTimeDilationManageSubsystem::IsTickable() const
{
	return !IsTemplate();
}

ETickableTickType UJoyTimeDilationManageSubsystem::GetTickableTickType() const
{
	return (HasAnyFlags(RF_ClassDefaultObject) ? ETickableTickType::Never : ETickableTickType::Conditional);
}

TStatId UJoyTimeDilationManageSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFGTimeDilationManageSubsystem, STATGROUP_Tickables);
}

FFGTimeDilationHandle UJoyTimeDilationManageSubsystem::AddGlobalTimeDilation(
	float const TimeDilation, const FString Description)
{
	return NewAddTimeDilationRequestCache(true, false, nullptr, TimeDilation, false, nullptr, Description);
}

FFGTimeDilationHandle UJoyTimeDilationManageSubsystem::AddGlobalTimeDilationWithCallback(
	float const TimeDilation, FFGOnTimeDilationApply OnApply, const FString Description)
{
	return NewAddTimeDilationRequestCache(true, false, nullptr, TimeDilation, false, std::move(OnApply), Description);
}

FFGTimeDilationHandle UJoyTimeDilationManageSubsystem::OverrideGlobalTimeDilation(
	float const TimeDilation, const FString Description)
{
	return NewAddTimeDilationRequestCache(true, true, nullptr, TimeDilation, false, nullptr, Description);
}

FFGTimeDilationHandle UJoyTimeDilationManageSubsystem::OverrideGlobalTimeDilationWithCallback(
	float const TimeDilation, FFGOnTimeDilationApply OnApply, const FString Description)
{
	return NewAddTimeDilationRequestCache(true, true, nullptr, TimeDilation, false, std::move(OnApply), Description);
}

FFGTimeDilationHandle UJoyTimeDilationManageSubsystem::AddActorTimeDilation(
	AActor* Actor, float const TimeDilation, const FString Description)
{
	return NewAddTimeDilationRequestCache(false, false, Actor, TimeDilation, false, nullptr, Description);
}

FFGTimeDilationHandle UJoyTimeDilationManageSubsystem::AddActorTimeDilationWithCallback(
	AActor* Actor, float const TimeDilation, FFGOnTimeDilationApply OnApply, const FString Description)
{
	return NewAddTimeDilationRequestCache(false, false, Actor, TimeDilation, false, std::move(OnApply), Description);
}

FFGTimeDilationHandle UJoyTimeDilationManageSubsystem::OverrideActorTimeDilation(
	AActor* Actor, float const TimeDilation, bool const bUseAbsoluteValue, FString Description)
{
	return NewAddTimeDilationRequestCache(false, true, Actor, TimeDilation, bUseAbsoluteValue, nullptr, Description);
}

FFGTimeDilationHandle UJoyTimeDilationManageSubsystem::OverrideActorTimeDilationWithCallback(AActor* Actor,
	float const TimeDilation, bool const bUseAbsoluteValue, FFGOnTimeDilationApply OnApply, const FString Description)
{
	return NewAddTimeDilationRequestCache(
		false, true, Actor, TimeDilation, bUseAbsoluteValue, std::move(OnApply), Description);
}

bool UJoyTimeDilationManageSubsystem::UpdateGlobalTimeDilation(
	FFGTimeDilationHandle const& Handle, float const TimeDilation)
{
	if (FFGTimeDilationRequestCache* CacheItem = RequestCaches.FindByKey(Handle))
	{
		CacheItem->Dilation = TimeDilation;
		return true;
	}

	FFGTimeDilationManageCache& Cache = GlobalCache;
	if (FFGTimeDilationHandleCache* CacheItem = Cache.HandleCaches.FindByKey(Handle))
	{
		CacheItem->TimeDilation = TimeDilation;
		ReCalculateCacheTimeDilation(Cache);
		SetGlobalTimeDilationByCache(Cache);
		return true;
	}

	return false;
}

bool UJoyTimeDilationManageSubsystem::UpdateActorTimeDilation(
	AActor* Actor, FFGTimeDilationHandle const& Handle, float const TimeDilation)
{
	if (!Actor)
	{
		return false;
	}

	if (FFGTimeDilationRequestCache* CacheItem = RequestCaches.FindByKey(Handle))
	{
		CacheItem->Dilation = TimeDilation;
		return true;
	}

	uint32 const ID = Actor->GetUniqueID();
	FFGTimeDilationManageCache* CachePtr = ActorCaches.Find(ID);
	if (!CachePtr)
	{
		return false;
	}

	FFGTimeDilationManageCache& Cache = *CachePtr;
	if (Cache.OwnerActor.Get() != Actor)
	{
		return false;
	}

	FFGTimeDilationHandleCache* CacheItem = Cache.HandleCaches.FindByKey(Handle);
	if (!CacheItem)
	{
		return false;
	}

	CacheItem->TimeDilation = TimeDilation;
	ReCalculateCacheTimeDilation(Cache);
	if (IsValid(Actor) && !Actor->IsActorBeingDestroyed())
	{
		if (AJoyCharacter* Character = Cast<AJoyCharacter>(Actor))
		{
			Character->SetCustomTimeDilation(Cache.CurrentDilation);
		}
		else
		{
			Actor->CustomTimeDilation = Cache.CurrentDilation;
		}
	}

	return true;
}

void UJoyTimeDilationManageSubsystem::RemoveGlobalTimeDilation(FFGTimeDilationHandle const& Handle)
{
	RemoveGlobalTimeDilationWithCallback(Handle, nullptr);
}

void UJoyTimeDilationManageSubsystem::RemoveGlobalTimeDilationWithCallback(
	FFGTimeDilationHandle const& Handle, FFGOnTimeDilationApply OnApply)
{
	if (int32 const Index = RequestCaches.IndexOfByKey(Handle); Index != INDEX_NONE)
	{
		// ReSharper disable once CppExpressionWithoutSideEffects
		RequestCaches[Index].OnApplyCallback.ExecuteIfBound(Handle, false);
		RequestCaches.RemoveAt(Index);
		// ReSharper disable once CppExpressionWithoutSideEffects
		OnApply.ExecuteIfBound(Handle, true);
		return;
	}

	NewRemoveTimeDilationRequestCache(Handle, true, nullptr, std::move(OnApply));
}

void UJoyTimeDilationManageSubsystem::RemoveActorTimeDilation(AActor* Actor, FFGTimeDilationHandle const& Handle)
{
	RemoveActorTimeDilationWithCallback(Actor, Handle, nullptr);
}

void UJoyTimeDilationManageSubsystem::RemoveActorTimeDilationWithCallback(
	AActor* Actor, FFGTimeDilationHandle const& Handle, FFGOnTimeDilationApply OnApply)
{
	if (int32 const Index = RequestCaches.IndexOfByKey(Handle); Index != INDEX_NONE)
	{
		// ReSharper disable once CppExpressionWithoutSideEffects
		RequestCaches[Index].OnApplyCallback.ExecuteIfBound(Handle, false);
		RequestCaches.RemoveAt(Index);
		// ReSharper disable once CppExpressionWithoutSideEffects
		OnApply.ExecuteIfBound(Handle, true);
		return;
	}

	NewRemoveTimeDilationRequestCache(Handle, false, Actor, std::move(OnApply));
}

float UJoyTimeDilationManageSubsystem::GetGlobalTimeDilation() const
{
	return GlobalCache.CurrentDilation;
}

float UJoyTimeDilationManageSubsystem::GetGlobalTimeDilationOfHandle(FFGTimeDilationHandle const& Handle) const
{
	FFGTimeDilationHandleCache const* CacheItem = GlobalCache.HandleCaches.FindByKey(Handle);
	if (!CacheItem)
	{
		return 1.0f;
	}

	return CacheItem->TimeDilation;
}

void UJoyTimeDilationManageSubsystem::SetGlobalTimeDilationByCache(FFGTimeDilationManageCache& Cache) const
{
	UWorld const* World = GetWorld();
	if (!World)
	{
		return;
	}

	AWorldSettings* WorldSettings = World->GetWorldSettings();
	if (!WorldSettings)
	{
		return;
	}

	float const ActualTimeDilation = WorldSettings->SetTimeDilation(Cache.CurrentDilation);
	if (Cache.CurrentDilation == ActualTimeDilation)
	{
		return;
	}

	UE_LOG(LogJoy, Warning,
	       TEXT("SetGlobalTimeDilation: Time Dilation must be between %f and %f. Clamped value to that range."),
	       WorldSettings->MinGlobalTimeDilation, WorldSettings->MaxGlobalTimeDilation);
	Cache.CurrentDilation = ActualTimeDilation;
}

bool UJoyTimeDilationManageSubsystem::AddGlobalTimeDilationImpl(
	FFGTimeDilationHandle const Handle, float const TimeDilation, bool const bOverride)
{
	FFGTimeDilationManageCache& Cache = GlobalCache;
	if (bOverride)
	{
		Cache.HandleCaches.Reset();
	}

	FFGTimeDilationHandleCache const CacheItem(Handle, TimeDilation);
	Cache.HandleCaches.Add(CacheItem);
	ReCalculateCacheTimeDilation(Cache);
	SetGlobalTimeDilationByCache(Cache);

	return true;
}

bool UJoyTimeDilationManageSubsystem::AddActorTimeDilationImpl(FFGTimeDilationHandle const Handle, AActor* Actor,
                                                               float const TimeDilation, bool const bOverride,
                                                               bool const bUseAbsoluteValue)
{
	if (!Actor)
	{
		return false;
	}

	uint32 const ID = Actor->GetUniqueID();
	FFGTimeDilationManageCache* CachePtr = ActorCaches.Find(ID);
	if (CachePtr)
	{
		// 如果一个 Actor 已经被回收但没有正确处理时间膨胀的反注册，那么可能出现 ID 冲突，
		// 此时将旧的数据清空
		if (CachePtr->OwnerActor.Get() != Actor)
		{
			*CachePtr = {};
			CachePtr->OwnerActor = Actor;
		}
	}
	else
	{
		CachePtr = &ActorCaches.Emplace(ID);
		CachePtr->OwnerActor = Actor;
	}

	FFGTimeDilationManageCache& Cache = *CachePtr;
	if (bOverride)
	{
		Cache.HandleCaches.Reset();
	}

	float const ActualTimeDilation = bUseAbsoluteValue ? TimeDilation / GetGlobalTimeDilation() : TimeDilation;
	FFGTimeDilationHandleCache const CacheItem(Handle, ActualTimeDilation);
	Cache.HandleCaches.Add(CacheItem);
	ReCalculateCacheTimeDilation(Cache);
	if (IsValid(Actor) && !Actor->IsActorBeingDestroyed())
	{
		if (AJoyCharacter* Character = Cast<AJoyCharacter>(Actor))
		{
			Character->SetCustomTimeDilation(Cache.CurrentDilation);
		}
		else
		{
			Actor->CustomTimeDilation = Cache.CurrentDilation;
		}
	}

	return true;
}

bool UJoyTimeDilationManageSubsystem::RemoveGlobalTimeDilationImpl(FFGTimeDilationHandle const Handle)
{
	FFGTimeDilationManageCache& Cache = GlobalCache;
	int32 const Index = Cache.HandleCaches.IndexOfByKey(Handle);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	Cache.HandleCaches.RemoveAt(Index);
	ReCalculateCacheTimeDilation(Cache);
	SetGlobalTimeDilationByCache(Cache);
	return true;
}

bool UJoyTimeDilationManageSubsystem::RemoveActorTimeDilationImpl(FFGTimeDilationHandle const Handle, AActor* Actor)
{
	if (!Actor)
	{
		return false;
	}

	uint32 const ID = Actor->GetUniqueID();
	FFGTimeDilationManageCache* CachePtr = ActorCaches.Find(ID);
	if (!CachePtr)
	{
		return false;
	}

	FFGTimeDilationManageCache& Cache = *CachePtr;
	if (Cache.OwnerActor.Get() != Actor)
	{
		return false;
	}

	int32 const Index = Cache.HandleCaches.IndexOfByKey(Handle);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	Cache.HandleCaches.RemoveAt(Index);
	ReCalculateCacheTimeDilation(Cache);
	if (IsValid(Actor) && !Actor->IsActorBeingDestroyed())
	{
		if (AJoyCharacter* Character = Cast<AJoyCharacter>(Actor))
		{
			Character->SetCustomTimeDilation(Cache.CurrentDilation);
		}
		else
		{
			Actor->CustomTimeDilation = Cache.CurrentDilation;
		}
	}

	if (Cache.HandleCaches.IsEmpty())
	{
		ActorCaches.Remove(ID);
	}

	return true;
}

FFGTimeDilationHandle UJoyTimeDilationManageSubsystem::NewAddTimeDilationRequestCache(bool const bIsGlobal,
	bool const bOverride, AActor* Actor, float const TimeDilation, bool const bUseAbsoluteValue,
	FFGOnTimeDilationApply OnApply, const FString& Description)
{
	SequenceNumber++;
	FFGTimeDilationRequestCache& NewReq = RequestCaches.Emplace_GetRef();
	NewReq.bIsAdd = true;
	NewReq.bIsGlobal = bIsGlobal;
	NewReq.bOverride = bOverride;
	NewReq.bUseAbsoluteValue = bUseAbsoluteValue;
	NewReq.Handle = FFGTimeDilationHandle(SequenceNumber);
	NewReq.Actor = Actor;
	NewReq.Dilation = TimeDilation;
	NewReq.OnApplyCallback = std::move(OnApply);
	NewReq.Description = Description;
	return NewReq.Handle;
}

void UJoyTimeDilationManageSubsystem::NewRemoveTimeDilationRequestCache(
	FFGTimeDilationHandle const Handle, bool const bIsGlobal, AActor* Actor, FFGOnTimeDilationApply OnApply)
{
	FFGTimeDilationRequestCache& NewReq = RequestCaches.Emplace_GetRef();
	NewReq.bIsAdd = false;
	NewReq.bIsGlobal = bIsGlobal;
	NewReq.bOverride = false;
	NewReq.bUseAbsoluteValue = false;
	NewReq.Handle = Handle;
	NewReq.Actor = Actor;
	NewReq.OnApplyCallback = std::move(OnApply);
}

void UJoyTimeDilationManageSubsystem::ReCalculateCacheTimeDilation(FFGTimeDilationManageCache& Cache)
{
	float TimeDilation = 1.0f;
	for (FFGTimeDilationHandleCache const& CacheItem : Cache.HandleCaches)
	{
		TimeDilation *= CacheItem.TimeDilation;
	}

	Cache.CurrentDilation = TimeDilation;
}
