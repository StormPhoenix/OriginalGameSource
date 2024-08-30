// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerState.h"
#include "ScorpioPlayerState.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class ORIGINALGAME_API AScorpioPlayerState : public AModularPlayerState
{
	GENERATED_BODY()
	
public:
	AScorpioPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
