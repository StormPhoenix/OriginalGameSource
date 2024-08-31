// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerController.h"
#include "JoyPlayerController.generated.h"

/**
 * 
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class ORIGINALGAME_API AJoyPlayerController : public AModularPlayerController
{
	GENERATED_BODY()
	
public:
	AJoyPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Joy|PlayerController")
	AJoyPlayerState* GetJoyPlayerState() const;
};
