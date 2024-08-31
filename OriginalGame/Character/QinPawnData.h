// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "QinPawnData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Original Game Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class ORIGINALGAME_API UQinPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UQinPawnData(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Qin|Pawn")
	TSubclassOf<APawn> PawnClass;
};
