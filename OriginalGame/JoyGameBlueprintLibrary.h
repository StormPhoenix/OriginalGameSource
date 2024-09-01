// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JoyGameBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ORIGINALGAME_API UJoyGameBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Controller", meta = (WorldContext = "WorldContextObject"))
	static AJoyPlayerController* GetJoyPlayerController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Input", meta = (WorldContext = "WorldContextObject"))
	static void RegisterInputReceiver(const UObject* WorldContextObject, UObject* Receiver);
	
	UFUNCTION(BlueprintCallable, Category = "Input", meta = (WorldContext = "WorldContextObject"))
	static void UnregisterInputReceiver(const UObject* WorldContextObject, UObject* Receiver);
};
