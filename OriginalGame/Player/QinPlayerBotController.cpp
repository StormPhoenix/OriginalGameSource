// Fill out your copyright notice in the Description page of Project Settings.


#include "QinPlayerBotController.h"


AQinPlayerBotController::AQinPlayerBotController(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsPlayerState = true;
	bStopAILogicOnUnposses = false;
}

void AQinPlayerBotController::BeginPlay()
{
	Super::BeginPlay();
}

void AQinPlayerBotController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
