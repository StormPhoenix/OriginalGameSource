#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "JoyGlobalGameSettings.generated.h"

struct FJoyCameraConfigTable;
class UJoyCameraData;

/**
 * 游戏全局配置。
 */
UCLASS(config = Game, defaultconfig)
class ORIGINALGAME_API UJoyGlobalGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static UJoyGlobalGameSettings const* Get();

	static UJoyCameraData* GetCameraDataConfig(UObject const* WorldContextObject);

	UPROPERTY(Config, EditDefaultsOnly, Category = "Joy|Camera")
	TSoftObjectPtr<UJoyCameraData> CameraDataConfig{};
	
private:
	static UJoyGlobalGameSettings const* GetCDO();
};
