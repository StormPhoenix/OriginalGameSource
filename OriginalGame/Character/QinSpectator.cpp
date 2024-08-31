// Fill out your copyright notice in the Description page of Project Settings.


#include "QinSpectator.h"


// Sets default values
AQinSpectator::AQinSpectator()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AQinSpectator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AQinSpectator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AQinSpectator::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

