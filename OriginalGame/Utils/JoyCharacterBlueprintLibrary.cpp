// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyCharacterBlueprintLibrary.h"

#include "Character/JoyHeroCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Gameplay/JoyCharacterControlManageSubsystem.h"

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

bool UJoyCharacterBlueprintLibrary::CheckCharacterControlled(AJoyHeroCharacter* HeroCharacter)
{
	if (const auto* ControlManager = UJoyCharacterControlManageSubsystem::GetCharacterControlManageSubsystem(HeroCharacter);
		ControlManager != nullptr && HeroCharacter)
	{
		return ControlManager->GetCurrentControlCharacter() == HeroCharacter;
	}

	return false;
}
