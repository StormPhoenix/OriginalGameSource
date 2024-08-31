// Fill out your copyright notice in the Description page of Project Settings.


#include "QinPlayerController.h"

#include "QinPlayerState.h"

AQinPlayerController::AQinPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

AQinPlayerState* AQinPlayerController::GetQinPlayerState() const
{
	return CastChecked<AQinPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}
