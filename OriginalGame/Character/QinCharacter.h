// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularCharacter.h"
#include "QinCharacter.generated.h"

UCLASS(Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class ORIGINALGAME_API AQinCharacter : public AModularCharacter
{
	GENERATED_BODY()

public:
	AQinCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "OriginalGame|Character")
	AQinPlayerState* GetQinPlayerState() const;
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
