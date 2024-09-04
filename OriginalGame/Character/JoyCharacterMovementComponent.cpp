// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyCharacterMovementComponent.h"

UJoyCharacterMovementComponent::UJoyCharacterMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UJoyCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

// Called every frame
void UJoyCharacterMovementComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
