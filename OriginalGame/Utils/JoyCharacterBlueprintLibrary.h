// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JoyCharacterBlueprintLibrary.generated.h"

class ACharacter;

/**
 * 
 */
UCLASS()
class ORIGINALGAME_API UJoyCharacterBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Camera)
	static float GetCharacterRadiusXY(ACharacter* Avatar);
};
