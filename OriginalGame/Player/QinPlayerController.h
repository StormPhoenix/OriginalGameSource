// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerController.h"
#include "QinPlayerController.generated.h"

/**
 * 
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class ORIGINALGAME_API AQinPlayerController : public AModularPlayerController
{
	GENERATED_BODY()
	
public:
	AQinPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Qin|PlayerController")
	AQinPlayerState* GetQinPlayerState() const;
};
