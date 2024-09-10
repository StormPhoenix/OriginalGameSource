#include "JoyGlobalGameSettings.h"

#include "Camera/JoyCameraData.h"
#include "System/JoyObjectCachePoolSubSystem.h"

UJoyGlobalGameSettings const* UJoyGlobalGameSettings::Get()
{
	return GetCDO();
}

UJoyGlobalGameSettings const* UJoyGlobalGameSettings::GetCDO()
{
	return StaticClass()->GetDefaultObject<UJoyGlobalGameSettings>();
}

UJoyCameraData* UJoyGlobalGameSettings::GetCameraDataConfig(UObject const* WorldContextObject)
{
	if (UJoyGlobalGameSettings const* Self = GetCDO())
	{
		if (UJoyObjectCachePoolSubSystem* CachePool =
				UJoyObjectCachePoolSubSystem::GetJoyObjectCachePoolSubSystem(WorldContextObject))
		{
			UJoyCameraData* CameraDataConfig = CachePool->GetObject(Self->GetName(), Self->CameraDataConfig);
			if (!CameraDataConfig)
			{
				CameraDataConfig = CachePool->GetOrLoadObject(Self->GetName(), Self->CameraDataConfig);
				CameraDataConfig->CacheCameraData();
			}

			return CameraDataConfig;
		}
	}

	return nullptr;
}