// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularCharacter.h"
#include "JoyCharacter.generated.h"

UCLASS(Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class ORIGINALGAME_API AJoyCharacter : public AModularCharacter
{
	GENERATED_BODY()

public:
	AJoyCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "OriginalGame|Character")
	AJoyPlayerState* GetJoyPlayerState() const;
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
