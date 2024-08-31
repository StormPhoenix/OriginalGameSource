// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularAIController.h"
#include "QinPlayerBotController.generated.h"

UCLASS(Blueprintable)
class ORIGINALGAME_API AQinPlayerBotController : public AModularAIController
{
	GENERATED_BODY()

public:
	AQinPlayerBotController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
