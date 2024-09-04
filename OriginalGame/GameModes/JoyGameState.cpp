// Copyright Epic Games, Inc. All Rights Reserved.

#include "JoyGameState.h"

#include "Async/TaskGraphInterfaces.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/JoyExperienceManagerComponent.h"
#include "Messages/JoyVerbMessage.h"
#include "Net/UnrealNetwork.h"
#include "Player/JoyPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(JoyGameState)

class APlayerState;
class FLifetimeProperty;

extern ENGINE_API float GAverageFPS;

AJoyGameState::AJoyGameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	ExperienceManagerComponent =
		CreateDefaultSubobject<UJoyExperienceManagerComponent>(TEXT("ExperienceManagerComponent"));
}

void AJoyGameState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AJoyGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AJoyGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AJoyGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
}

void AJoyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	//@TODO: This isn't getting called right now (only the 'rich' AGameMode uses it, not AGameModeBase)
	// Need to at least comment the engine code, and possibly move things around
	Super::RemovePlayerState(PlayerState);
}

void AJoyGameState::SeamlessTravelTransitionCheckpoint(bool bToTransitionMap)
{
	// Remove inactive and bots
	for (int32 i = PlayerArray.Num() - 1; i >= 0; i--)
	{
		APlayerState* PlayerState = PlayerArray[i];
		if (PlayerState && (PlayerState->IsABot() || PlayerState->IsInactive()))
		{
			RemovePlayerState(PlayerState);
		}
	}
}

void AJoyGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AJoyGameState::MulticastMessageToClients_Implementation(const FJoyVerbMessage Message)
{
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
}

void AJoyGameState::MulticastReliableMessageToClients_Implementation(const FJoyVerbMessage Message)
{
	MulticastMessageToClients_Implementation(Message);
}
