// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "JoyPawnData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Original Game Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class ORIGINALGAME_API UJoyPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UJoyPawnData(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Joy|Pawn")
	TSubclassOf<APawn> PawnClass;
};
