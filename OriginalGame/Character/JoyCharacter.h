// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularCharacter.h"

#include "JoyCharacter.generated.h"

class UJoyCameraComponent;
class UJoyPawnExtensionComponent;

UCLASS(Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class ORIGINALGAME_API AJoyCharacter : public AModularCharacter
{
	GENERATED_BODY()

public:
	AJoyCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "OriginalGame|Character")
	AJoyPlayerState* GetJoyPlayerState() const;

	void SetCustomTimeDilation(float const TimeDilation);

protected:
	virtual void BeginPlay() override;

	virtual void OnAbilitySystemInitialized()
	{
	}
	virtual void OnAbilitySystemUninitialized()
	{
	}

public:
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Joy|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UJoyCameraComponent> JoyCameraComponent{nullptr};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Joy|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UJoyPawnExtensionComponent> PawnExtComponent;
};
