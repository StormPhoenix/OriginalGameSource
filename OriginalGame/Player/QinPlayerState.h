// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerState.h"
#include "QinPlayerState.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class ORIGINALGAME_API AQinPlayerState : public AModularPlayerState
{
	GENERATED_BODY()
	
public:
	AQinPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
