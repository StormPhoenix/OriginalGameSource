// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyCameraData.h"

#define LOCTEXT_NAMESPACE "UFGCameraData"

UJoyCameraData::UJoyCameraData(const FObjectInitializer& ObjectInitializer)
{
}

void UJoyCameraData::CacheCameraData()
{
	CacheCameraConfigs.Reset();

	for (const auto& CameraTable : CameraTables)
	{
		CameraTable->ForeachRow<FJoyCameraConfigTable>(TEXT("FJoyCameraConfigTable::ForeachRow"),
			[this, &CameraTable](const FName& Key, const FJoyCameraConfigTable& Value) mutable
			{
				if (!CacheCameraConfigs.Contains(Key))
				{
					CacheCameraConfigs.Add(Key, Value);
				}
				else
				{
					FFormatOrderedArguments Args;
					Args.Add(FText::FromString(Key.ToString()));
					Args.Add(FText::FromString(CameraTable.GetPathName()));
					FMessageDialog::Debugf(
						FText::Format(LOCTEXT("UJoyCameraData", "检测到用户在 %s 内配置了重复的子镜头 ID: %s"), Args));
				}
			});
	}
}

#undef LOCTEXT_NAMESPACE
