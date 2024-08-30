// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerController.h"
#include "ScorpioPlayerController.generated.h"

/**
 * 
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class ORIGINALGAME_API AScorpioPlayerController : public AModularPlayerController
{
	GENERATED_BODY()
	
public:
	AScorpioPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerController")
	AScorpioPlayerState* GetScorpioPlayerState() const;
};
