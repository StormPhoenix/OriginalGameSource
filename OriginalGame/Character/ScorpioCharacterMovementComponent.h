// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ScorpioCharacterMovementComponent.generated.h"

UCLASS(Config = Game)
class ORIGINALGAME_API UScorpioCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UScorpioCharacterMovementComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
