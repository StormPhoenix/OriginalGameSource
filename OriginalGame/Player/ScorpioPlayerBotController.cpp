// Fill out your copyright notice in the Description page of Project Settings.


#include "ScorpioPlayerBotController.h"


AScorpioPlayerBotController::AScorpioPlayerBotController(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsPlayerState = true;
	bStopAILogicOnUnposses = false;
}

void AScorpioPlayerBotController::BeginPlay()
{
	Super::BeginPlay();
}

void AScorpioPlayerBotController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
