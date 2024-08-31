// Fill out your copyright notice in the Description page of Project Settings.


#include "JoyPawnCreationComponent.h"


UJoyPawnCreationComponent::UJoyPawnCreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UJoyPawnCreationComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UJoyPawnCreationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
