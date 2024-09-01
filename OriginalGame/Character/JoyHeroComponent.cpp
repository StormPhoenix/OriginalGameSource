// Fill out your copyright notice in the Description page of Project Settings.


#include "JoyHeroComponent.h"


UJoyHeroComponent::UJoyHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UJoyHeroComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UJoyHeroComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
