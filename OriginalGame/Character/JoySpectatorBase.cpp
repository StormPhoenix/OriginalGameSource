// Fill out your copyright notice in the Description page of Project Settings.


#include "JoySpectatorBase.h"


AJoySpectatorBase::AJoySpectatorBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AJoySpectatorBase::BeginPlay()
{
	Super::BeginPlay();
}

void AJoySpectatorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AJoySpectatorBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

