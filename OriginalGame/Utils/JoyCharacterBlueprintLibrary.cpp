// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyCharacterBlueprintLibrary.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

float UJoyCharacterBlueprintLibrary::GetCharacterRadiusXY(ACharacter* Avatar)
{
	if (Avatar)
	{
		if (UCapsuleComponent const* Capsule = Avatar->GetCapsuleComponent())
		{
			return Capsule->GetScaledCapsuleRadius();
		}
	}
	return 0.f;
}
