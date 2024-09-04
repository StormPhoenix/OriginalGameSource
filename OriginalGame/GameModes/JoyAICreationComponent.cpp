// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyAICreationComponent.h"

#include "AIController.h"
#include "Character/JoyAISpawner.h"
#include "Character/JoyPawnData.h"
#include "EngineUtils.h"
#include "JoyExperienceDefinition.h"
#include "GameFramework/PlayerState.h"
#include "JoyExperienceManagerComponent.h"
#include "JoyGameMode.h"
#include "Character/JoyCharacter.h"
#include "Gameplay/JoyCharacterControlManageSubsystem.h"
#include "Player/JoyPlayerController.h"
#include "Player/JoyPlayerState.h"

UJoyAICreationComponent::UJoyAICreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UJoyAICreationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const auto* GameState = GetGameStateChecked<AGameStateBase>())
	{
		auto* ExperienceComponent = GameState->FindComponentByClass<UJoyExperienceManagerComponent>();
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

		if (Experience)
		{
			const AController* DefaultPawnController = SpawnFromPawnData(Experience->DefaultPawnData, nullptr);
			auto* ControlManager = UJoyCharacterControlManageSubsystem::GetCharacterControlManageSubsystem(this);
			if (DefaultPawnController && ControlManager)
			{
				FJoyCharacterSwitchExtraParam SwitchParam{};
				SwitchParam.bImmediately = true;
				SwitchParam.BlendType = EJoyCameraBlendType::KeepDirection;
				ControlManager->SwitchToCharacter(Cast<AJoyCharacter>(DefaultPawnController->GetPawn()), SwitchParam);
			}
		}
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

AController* UJoyAICreationComponent::SpawnFromAISpawner(AJoyAISpawner* Spawner) const
{
	if (Spawner == nullptr || Spawner->PawnData == nullptr)
	{
		return nullptr;
	}

	return SpawnFromPawnData(Spawner->PawnData, Spawner);
}

AController* UJoyAICreationComponent::SpawnFromPawnData(const UJoyPawnData* PawnData, AActor* StartSpot) const
{
	if (PawnData == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = GetComponentLevel();
	SpawnInfo.ObjectFlags |= RF_Transient;
	AController* NewController = GetWorld()->SpawnActor<AController>(
		PawnData->ControllerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);

	if (NewController != nullptr)
	{
		AJoyGameMode* GameMode = GetGameMode<AJoyGameMode>();
		check(GameMode);

		if (AJoyPlayerState* JoyPS = Cast<AJoyPlayerState>(NewController->PlayerState))
		{
			const FString PawnName = FString::Printf(TEXT("JoyPawn %d"), NewController->PlayerState->GetPlayerId());
			JoyPS->SetPlayerName(PawnName);
			JoyPS->SetPawnData(PawnData);
		}

		GameMode->GenericPlayerInitialization(NewController);
		if (StartSpot)
		{
			GameMode->RestartPlayerAtPlayerStart(NewController, StartSpot);
		}
		else
		{
			GameMode->RestartPlayer(NewController);
		}
	}

	return NewController;
}

void UJoyAICreationComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
