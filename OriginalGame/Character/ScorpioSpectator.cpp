// Fill out your copyright notice in the Description page of Project Settings.


#include "ScorpioSpectator.h"


// Sets default values
AScorpioSpectator::AScorpioSpectator()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AScorpioSpectator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AScorpioSpectator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AScorpioSpectator::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

