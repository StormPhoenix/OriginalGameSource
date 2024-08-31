// Fill out your copyright notice in the Description page of Project Settings.


#include "JoyPlayerController.h"

#include "JoyPlayerState.h"

AJoyPlayerController::AJoyPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

AJoyPlayerState* AJoyPlayerController::GetJoyPlayerState() const
{
	return CastChecked<AJoyPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}
