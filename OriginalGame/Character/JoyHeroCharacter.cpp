// Fill out your copyright notice in the Description page of Project Settings.


#include "JoyHeroCharacter.h"


AJoyHeroCharacter::AJoyHeroCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	HeroComponent = CreateDefaultSubobject<UJoyHeroComponent>("HeroComponent");
}

void AJoyHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AJoyHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AJoyHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
