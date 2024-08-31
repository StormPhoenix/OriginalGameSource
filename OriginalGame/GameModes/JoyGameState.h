// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "ModularGameState.h"

#include "JoyGameState.generated.h"

struct FJoyVerbMessage;

class APlayerState;
class UAbilitySystemComponent;
class UJoyAbilitySystemComponent;
class UJoyExperienceManagerComponent;
class UObject;
struct FFrame;

/**
 * AJoyGameState
 *
 *	The base game state class used by this project.
 */
UCLASS(Config = Game)
class ORIGINALGAME_API AJoyGameState : public AModularGameStateBase
{
	GENERATED_BODY()

public:

	AJoyGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~AGameStateBase interface
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void SeamlessTravelTransitionCheckpoint(bool bToTransitionMap) override;
	//~End of AGameStateBase interface

	// Send a message that all clients will (probably) get
	// (use only for client notifications like eliminations, server join messages, etc... that can handle being lost)
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category = "Joy|GameState")
	void MulticastMessageToClients(const FJoyVerbMessage Message);

	// Send a message that all clients will be guaranteed to get
	// (use only for client notifications that cannot handle being lost)
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Joy|GameState")
	void MulticastReliableMessageToClients(const FJoyVerbMessage Message);

private:
	UPROPERTY()
	TObjectPtr<UJoyExperienceManagerComponent> ExperienceManagerComponent;

protected:

	virtual void Tick(float DeltaSeconds) override;
};
