#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "JoyObjectCachePoolSubSystem.generated.h"

UCLASS()
class ORIGINALGAME_API UJoyObjectCachePoolSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UJoyObjectCachePoolSubSystem* Get(UWorld const* World);
	static UJoyObjectCachePoolSubSystem* GetJoyObjectCachePoolSubSystem(UObject const* WorldContextObject);

	template <class T>
	T* GetOrLoadObject(FString const& KeyPrefix, TSoftObjectPtr<T> const& SoftPath)
	{
		return Cast<T>(GetOrLoadObjectGeneral(KeyPrefix, SoftPath.GetUniqueID()));
	}

	UObject* GetOrLoadObjectGeneral(FString const& KeyPrefix, FSoftObjectPath const& SoftPath);

	template <class T>
	T* GetObject(FString const& KeyPrefix, TSoftObjectPtr<T> const& SoftPath)
	{
		return Cast<T>(GetObjectGeneral(KeyPrefix, SoftPath.GetUniqueID()));
	}

	UObject* GetObjectGeneral(FString const& KeyPrefix, FSoftObjectPath const& SoftPath);

private:
	UPROPERTY()
	TMap<FString, TObjectPtr<UObject>> CachedObjects{};
};
