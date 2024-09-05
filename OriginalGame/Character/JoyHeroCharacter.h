// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JoyCharacter.h"
#include "JoyHeroComponent.h"
#include "JoyHeroCharacter.generated.h"

UCLASS()
class ORIGINALGAME_API AJoyHeroCharacter : public AJoyCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AJoyHeroCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(BlueprintReadOnly, Meta=(AllowPrivateAccess = true))
	UJoyHeroComponent* HeroComponent{nullptr};
};
