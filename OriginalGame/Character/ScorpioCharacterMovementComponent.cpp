// Fill out your copyright notice in the Description page of Project Settings.


#include "ScorpioCharacterMovementComponent.h"


UScorpioCharacterMovementComponent::UScorpioCharacterMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UScorpioCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UScorpioCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

