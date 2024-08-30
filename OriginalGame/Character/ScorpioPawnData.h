// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ScorpioPawnData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Original Game Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class ORIGINALGAME_API UScorpioPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UScorpioPawnData(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scorpio|Pawn")
	TSubclassOf<APawn> PawnClass;
};
