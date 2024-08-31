// Fill out your copyright notice in the Description page of Project Settings.


#include "JoyPlayerBotController.h"


AJoyPlayerBotController::AJoyPlayerBotController(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsPlayerState = true;
	bStopAILogicOnUnposses = false;
}

void AJoyPlayerBotController::BeginPlay()
{
	Super::BeginPlay();
}

void AJoyPlayerBotController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
