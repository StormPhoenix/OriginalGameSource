// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controller/JoyCameraMeta.h"
#include "Engine/DataAsset.h"

#include "JoyCameraData.generated.h"

UCLASS(BlueprintType, Const,
	Meta = (DisplayName = "Joy Camera Data", ShortTooltip = "Data asset used to define camera configs."))
class ORIGINALGAME_API UJoyCameraData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UJoyCameraData(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = "Joy|Camera",
		meta = (RowType = "/Script/OriginalGame.JoyCameraConfigTable",
			RequiredAssetDataTags = "RowStructure=/Script/OriginalGame.JoyCameraConfigTable"))
	TArray<TObjectPtr<UDataTable>> CameraTables{};

	void CacheCameraData();
	TMap<FName, FJoyCameraConfigTable> CacheCameraConfigs{};
};
