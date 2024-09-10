// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyCameraConfigController.h"

#include "JoyCameraInputController.h"
#include "JoyCameraMeta.h"
#include "JoyLogChannels.h"
#include "Camera/JoyPlayerCameraManager.h"
#include "Settings/JoyGlobalGameSettings.h"
#include "Utils/JoyCameraBlueprintLibrary.h"

UCameraConfig::UCameraConfig()
{
}

void UCameraConfig::LoadProto(const FJoyCameraConfigTable& Config)
{
	Description = Config.Description;
	Type = Config.Type;
	FadeInTime = Config.FadeInTime;
	FadeOutTime = Config.FadeOutTime;
	bIgnoreTimeDilation = Config.bIgnoreTimeDilation;
	bOverrideCameraInput = Config.bOverrideCameraInput;
	bArmCenterLagEnable = Config.bArmCenterLagEnable;
	Priority = Config.Priority;
	bFadeArmPitch = false;
	FadeTargetArmPitch = 0.f;
	BasicConfig = Config.Basic;
	InputConfig = Config.Input;
}

UJoyCameraConfigController::UJoyCameraConfigController()
{
}

void UJoyCameraConfigController::InitializeFor(AJoyPlayerCameraManager* PCMgr)
{
	Super::InitializeFor(PCMgr);

	if (CameraManager != nullptr)
	{
		LoadConfig();
	}
}

void UJoyCameraConfigController::MarkDirty()
{
	bConfigDirty = true;
}

void UJoyCameraConfigController::UpdateInternal(float DeltaSeconds)
{
	Super::UpdateInternal(DeltaSeconds);
	UpdateConfig();
}

UCameraConfig* LoadProtoConfig(const FJoyCameraConfigTable& ProtoConfig)
{
	UCameraConfig* CameraConfig = NewObject<UCameraConfig>();
	CameraConfig->LoadProto(ProtoConfig);
	return CameraConfig;
}

void UJoyCameraConfigController::LoadConfig()
{
	TMap<FName, FJoyCameraConfigTable> ConfigMap;
	UJoyCameraBlueprintLibrary::GetCameraConfigMap(this, ConfigMap);

	bool bHasBasicType = false;
	for (auto Iterator = ConfigMap.CreateConstIterator(); Iterator; ++Iterator)
	{
		FName CameraID = Iterator->Key;
		const auto& Config = Iterator->Value;

		if (Config.Type == EJoyCameraType::Basic)
		{
			if (bHasBasicType)
			{
				// 防止用户配置了两个基础镜头
				UE_LOG(LogJoyCamera, Error, TEXT("镜头配置表内存在多个基础类型相机配置，需要删减为一个. "));
			}
			else
			{
				// 基础镜头配置
				DefaultConfig = LoadProtoConfig(Config);
				bHasBasicType = true;
			}
		}
		else if (Config.Type == EJoyCameraType::Sub)
		{
			CachedConfigMap.Add(CameraID, LoadProtoConfig(Config));
		}
		else
		{
			// @COMMENT 有新增镜头类型就添加在这
		}
	}

	if (DefaultConfig == nullptr)
	{
		UE_LOG(LogJoyCamera, Error, TEXT("镜头参数表没有配置基础镜头"));
		return;
	}

	UpdateConfig(true);
}

void UJoyCameraConfigController::UpdateConfig(bool bForceUpdate)
{
	if (CameraManager == nullptr || DefaultConfig == nullptr)
	{
		UE_LOG(LogJoyCamera, Warning, TEXT("JoyCameraConfigController::UpdateConfig(): Camera manager is nullptr. "));
		return;
	}

	// 检查 GameplayTag 是否发生了变动，若有则更新镜头配置参数
	if (bConfigDirty || bForceUpdate)
	{
		TArray<UCameraConfig*> TmpConfigList;
		TmpConfigList.Add(DefaultConfig);

		const FCameraConfigDescription& CameraConfigDesc = CameraManager->GetCameraConfigDescription();
		for (auto CameraNameIterator = CameraConfigDesc.CameraStack.CreateConstIterator(); CameraNameIterator;
		     ++CameraNameIterator)
		{
			if (CachedConfigMap.Contains(*CameraNameIterator))
			{
				TmpConfigList.Add(CachedConfigMap[*CameraNameIterator]);
			}
		}

		TmpConfigList.Sort([](const UCameraConfig& A, const UCameraConfig& B)
		{
			return (A.Type == EJoyCameraType::Basic) || (A.Priority < B.Priority);
		});

		for (const auto* Config : TmpConfigList)
		{
			UpdateConfigData(Config);
			ConfigFadeDurationTime = Config->FadeOutTime;
			bFadeArmPitch = Config->bFadeArmPitch;
			FadeTargetArmPitch = Config->FadeTargetArmPitch;
			bArmCenterLagEnable = Config->bArmCenterLagEnable;
		}

		const int ConfigNum = TmpConfigList.Num();
		ConfigFadeDurationTime = TmpConfigList.IsEmpty() ? 0 : TmpConfigList[ConfigNum - 1]->FadeInTime;

		bool bOverrideCameraInput = false;
		if (!TmpConfigList.IsEmpty())
		{
			if (const UCameraConfig* TopCameraConfig = TmpConfigList[TmpConfigList.Num() - 1])
			{
				bOverrideCameraInput = TopCameraConfig->bOverrideCameraInput;
			}
		}

		CameraManager->ApplyConfig();
		CameraManager->StartFade(ConfigFadeDurationTime, true, true, bFadeArmPitch, false, true, true,
			true, bOverrideCameraInput);
	}

	bConfigDirty = false;
}

void UJoyCameraConfigController::UpdateConfigData(const UCameraConfig* Config)
{
	if (CameraManager == nullptr || Config == nullptr)
	{
		return;
	}

	CameraManager->SetConfigs(Config->BasicConfig);
	CameraManager->CameraInputController->SetConfigs(Config->InputConfig);
}
