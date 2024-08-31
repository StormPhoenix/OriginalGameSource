// Copyright Epic Games, Inc. All Rights Reserved.

#include "QinGameState.h"

#include "Async/TaskGraphInterfaces.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/QinExperienceManagerComponent.h"
#include "Messages/QinVerbMessage.h"
#include "Player/QinPlayerState.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(QinGameState)

class APlayerState;
class FLifetimeProperty;

extern ENGINE_API float GAverageFPS;


AQinGameState::AQinGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	ExperienceManagerComponent = CreateDefaultSubobject<UQinExperienceManagerComponent>(TEXT("ExperienceManagerComponent"));
}

void AQinGameState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AQinGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AQinGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AQinGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
}

void AQinGameState::RemovePlayerState(APlayerState* PlayerState)
{
	//@TODO: This isn't getting called right now (only the 'rich' AGameMode uses it, not AGameModeBase)
	// Need to at least comment the engine code, and possibly move things around
	Super::RemovePlayerState(PlayerState);
}

void AQinGameState::SeamlessTravelTransitionCheckpoint(bool bToTransitionMap)
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

void AQinGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AQinGameState::MulticastMessageToClients_Implementation(const FQinVerbMessage Message)
{
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
}

void AQinGameState::MulticastReliableMessageToClients_Implementation(const FQinVerbMessage Message)
{
	MulticastMessageToClients_Implementation(Message);
}
