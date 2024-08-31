// Fill out your copyright notice in the Description page of Project Settings.


#include "JoySpectator.h"


AJoySpectator::AJoySpectator()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AJoySpectator::BeginPlay()
{
	Super::BeginPlay();
}

void AJoySpectator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AJoySpectator::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

