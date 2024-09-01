// Fill out your copyright notice in the Description page of Project Settings.


#include "JoyAICreationComponent.h"

#include "AIController.h"
#include "EngineUtils.h"
#include "JoyExperienceManagerComponent.h"
#include "JoyGameMode.h"
#include "Character/JoyAISpawner.h"
#include "Character/JoyPawnData.h"
#include "GameFramework/PlayerState.h"
#include "Player/JoyPlayerState.h"


UJoyAICreationComponent::UJoyAICreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UJoyAICreationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (auto* GameState = GetGameStateChecked<AGameStateBase>())
	{
		auto* ExperienceComponent =
			GameState->FindComponentByClass<UJoyExperienceManagerComponent>();
		check(ExperienceComponent);

		// Low Priority Execution When Experience Full Loaded.
		ExperienceComponent->CallOrRegister_OnExperienceLoaded_LowPriority(
			FOnJoyExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
}

void UJoyAICreationComponent::OnExperienceLoaded(const UJoyExperienceDefinition* Experience)
{
#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		ServerCreateAI();
	}
#endif
}

void UJoyAICreationComponent::ServerCreateAI() const
{
	for (TActorIterator<AJoyAISpawner> It(GetWorld()); It; ++It)
	{
		if (AJoyAISpawner* Spawner = *It)
		{
			SpawnFromAISpawner(Spawner);
		}
	}
}

void UJoyAICreationComponent::SpawnFromAISpawner(AJoyAISpawner* Spawner) const
{
	if (Spawner == nullptr || Spawner->PawnData == nullptr)
	{
		return;
	}

	const UJoyPawnData* SpawnData = Spawner->PawnData;

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = GetComponentLevel();
	SpawnInfo.ObjectFlags |= RF_Transient;
	AController* NewController = GetWorld()->SpawnActor<AAIController>(
		SpawnData->ControllerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);

	if (NewController != nullptr)
	{
		AJoyGameMode* GameMode = GetGameMode<AJoyGameMode>();
		check(GameMode);

		if (AJoyPlayerState* JoyPS = Cast<AJoyPlayerState>(NewController->PlayerState))
		{
			const FString PawnName = FString::Printf(TEXT("JoyBot %d"), NewController->PlayerState->GetPlayerId());
			JoyPS->SetPlayerName(PawnName);
			JoyPS->SetPawnData(SpawnData);
		}

		GameMode->GenericPlayerInitialization(NewController);
		GameMode->RestartPlayerAtPlayerStart(NewController, Spawner);
	}
}


void UJoyAICreationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
