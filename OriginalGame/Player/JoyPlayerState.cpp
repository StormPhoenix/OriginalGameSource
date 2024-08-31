// Fill out your copyright notice in the Description page of Project Settings.


#include "JoyPlayerState.h"

AJoyPlayerState::AJoyPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AJoyPlayerState::SetPawnData(const UJoyPawnData* InPawnData)
{
	PawnData = InPawnData;
}
