#include "JoyObjectCachePoolSubSystem.h"

UJoyObjectCachePoolSubSystem* UJoyObjectCachePoolSubSystem::Get(UWorld const* World)
{
	if (World)
	{
		return UGameInstance::GetSubsystem<UJoyObjectCachePoolSubSystem>(World->GetGameInstance());
	}

	return nullptr;
}

UJoyObjectCachePoolSubSystem* UJoyObjectCachePoolSubSystem::GetJoyObjectCachePoolSubSystem(
	UObject const* WorldContextObject)
{
	if (UWorld const* World =
			GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return UJoyObjectCachePoolSubSystem::Get(World);
	}

	return nullptr;
}

UObject* UJoyObjectCachePoolSubSystem::GetOrLoadObjectGeneral(FString const& KeyPrefix, FSoftObjectPath const& SoftPath)
{
	FString const Key = KeyPrefix + TEXT("::") + SoftPath.ToString();
	if (TObjectPtr<UObject>* CacheObject = CachedObjects.Find(Key))
	{
		return *CacheObject;
	}

	UObject* NewObject = SoftPath.TryLoad();
	if (!NewObject)
	{
		return nullptr;
	}

	CachedObjects.Add(Key, NewObject);
	return NewObject;
}

UObject* UJoyObjectCachePoolSubSystem::GetObjectGeneral(FString const& KeyPrefix, FSoftObjectPath const& SoftPath)
{
	FString const Key = KeyPrefix + TEXT("::") + SoftPath.ToString();
	if (TObjectPtr<UObject>* CacheObject = CachedObjects.Find(Key))
	{
		return *CacheObject;
	}

	return nullptr;
}