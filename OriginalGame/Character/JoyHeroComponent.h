// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "CoreMinimal.h"
#include "Input/JoyInputReceiver.h"

#include "JoyHeroComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ORIGINALGAME_API UJoyHeroComponent
	: public UPawnComponent, public IGameFrameworkInitStateInterface,
	  public IJoyInputReceiver
{
	GENERATED_BODY()

public:
	static const FName NAME_ActorFeatureName;

	UJoyHeroComponent(const FObjectInitializer& ObjectInitializer);

	//~ Begin IJoyInputReceiver interface
	virtual void
	ReceiveMoveInput_Implementation(UObject* InputReceiver, const FInputActionValue& InputActionValue) override;

	virtual void
	ReceiveAbilityTagPressInput_Implementation(UObject* InputReceiver, FGameplayTag const& InputTag) override;

	virtual void
	ReceiveAbilityTagReleaseInput_Implementation(UObject* InputReceiver, FGameplayTag const& InputTag) override;

	virtual void
	ReceiveLookMoveInput_Implementation(UObject* InputReceiver, const FInputActionValue& InputActionValue) override;
	//~ End IJoyInputReceiver interface

	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override
	{
		return NAME_ActorFeatureName;
	}

	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
		FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
		FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

protected:
	virtual void BeginPlay() override;

	virtual void OnRegister() override;

public:
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
