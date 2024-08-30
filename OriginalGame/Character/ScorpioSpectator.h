// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "ScorpioSpectator.generated.h"

UCLASS()
class ORIGINALGAME_API AScorpioSpectator : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	AScorpioSpectator();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
