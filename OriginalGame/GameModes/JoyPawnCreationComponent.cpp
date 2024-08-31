// Fill out your copyright notice in the Description page of Project Settings.


#include "JoyPawnCreationComponent.h"

#include "AIController.h"
#include "JoyGameMode.h"
#include "Character/JoyPawnData.h"
#include "GameFramework/PlayerState.h"
#include "Player/JoyPlayerState.h"


UJoyPawnCreationComponent::UJoyPawnCreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UJoyPawnCreationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UJoyPawnCreationComponent::SpawnPawnFromPawnData(const UJoyPawnData* PawnData) const
{
	if (PawnData == nullptr)
	{
		return;
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = GetComponentLevel();
	SpawnInfo.ObjectFlags |= RF_Transient;
	AController* NewController = GetWorld()->SpawnActor<AAIController>(
		PawnData->ControllerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);

	if (NewController != nullptr)
	{
		AJoyGameMode* GameMode = GetGameMode<AJoyGameMode>();
		check(GameMode);

		if (AJoyPlayerState* JoyPS = Cast<AJoyPlayerState>(NewController->PlayerState))
		{
			const FString PawnName = FString::Printf(TEXT("Tinplate %d"), NewController->PlayerState->GetPlayerId());
			JoyPS->SetPlayerName(PawnName);
			JoyPS->SetPawnData(PawnData);
		}

		GameMode->GenericPlayerInitialization(NewController);
		GameMode->RestartPlayer(NewController);
	}
}


void UJoyPawnCreationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
