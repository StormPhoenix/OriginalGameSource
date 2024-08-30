// Fill out your copyright notice in the Description page of Project Settings.


#include "ScorpioPlayerController.h"

#include "ScorpioPlayerState.h"

AScorpioPlayerController::AScorpioPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

AScorpioPlayerState* AScorpioPlayerController::GetScorpioPlayerState() const
{
	return CastChecked<AScorpioPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}
