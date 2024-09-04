// Copyright Epic Games, Inc. All Rights Reserved.

#include "JoyGameMode.h"

#include "Character/JoyPawnData.h"
#include "Character/JoySpectatorBase.h"
#include "Development/JoyDeveloperSettings.h"
#include "JoyExperienceDefinition.h"
#include "JoyExperienceManagerComponent.h"
#include "JoyGameState.h"
#include "JoyLogChannels.h"
#include "JoyWorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Player/JoyPlayerController.h"
#include "Player/JoyPlayerSpawningManagerComponent.h"
#include "Player/JoyPlayerState.h"
#include "System/JoyAssetManager.h"

AJoyGameMode::AJoyGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = AJoyGameState::StaticClass();
	PlayerControllerClass = AJoyPlayerController::StaticClass();
	PlayerStateClass = AJoyPlayerState::StaticClass();
	DefaultPawnClass = AJoySpectatorBase::StaticClass();
	SpectatorClass = AJoySpectatorBase::StaticClass();
}

void AJoyGameMode::InitGameState()
{
	Super::InitGameState();

	// Listen for the experience load to complete
	auto* ExperienceComponent = GameState->FindComponentByClass<UJoyExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded(
		FOnJoyExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

UClass* AJoyGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UJoyPawnData* PawnData = GetPawnDataForController(InController))
	{
		if (PawnData->PawnClass)
		{
			return PawnData->PawnClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AJoyGameMode::OnExperienceLoaded(const UJoyExperienceDefinition* CurrentExperience)
{
	// Spawn any players that are already attached
	//@TODO: Here we're handling only *player* controllers, but in GetDefaultPawnClassForController_Implementation we
	//skipped all controllers
	// GetDefaultPawnClassForController_Implementation might only be getting called for players anyways
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (auto* PC = Cast<APlayerController>(*Iterator))
		{
			auto* JoyPS = PC->GetPlayerState<AJoyPlayerState>();
			if (PC->GetPawnOrSpectator() == nullptr || JoyPS == nullptr ||
			    JoyPS->GetPawnData<UJoyPawnData>() == nullptr)
			{
				JoyPS->SetPawnData(CurrentExperience->DefaultPawnData);
				if (PlayerCanRestart(PC))
				{
					RestartPlayer(PC);
				}
			}
		}
	}
}

bool AJoyGameMode::IsExperienceLoaded() const
{
	check(GameState);
	const auto* ExperienceComponent = GameState->FindComponentByClass<UJoyExperienceManagerComponent>();
	check(ExperienceComponent);

	return ExperienceComponent->IsExperienceLoaded();
}

void AJoyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Wait for the next frame to give time to initialize startup settings
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::HandleMatchAssignmentIfNotExpectingOne);
}

void AJoyGameMode::HandleMatchAssignmentIfNotExpectingOne()
{
	FPrimaryAssetId ExperienceId;
	FString ExperienceIdSource;

	// Precedence order (highest wins)
	//  - Matchmaking assignment (if present)
	//  - URL Options override
	//  - Developer Settings (PIE only)
	//  - Command Line override
	//  - World Settings
	//  - Dedicated server
	//  - Default experience

	UWorld* World = GetWorld();

	if (!ExperienceId.IsValid() && World->IsPlayInEditor())
	{
		ExperienceId = GetDefault<UJoyDeveloperSettings>()->ExperienceOverride;
		ExperienceIdSource = TEXT("DeveloperSettings");
	}

	// see if the command line wants to set the experience
	if (!ExperienceId.IsValid())
	{
		FString ExperienceFromCommandLine;
		if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
		{
			ExperienceId = FPrimaryAssetId::ParseTypeAndName(ExperienceFromCommandLine);
			if (!ExperienceId.PrimaryAssetType.IsValid())
			{
				ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UJoyExperienceDefinition::StaticClass()->GetFName()),
					FName(*ExperienceFromCommandLine));
			}
			ExperienceIdSource = TEXT("CommandLine");
		}
	}

	// see if the world settings has a default experience
	if (!ExperienceId.IsValid())
	{
		if (auto* TypedWorldSettings = Cast<AJoyWorldSettings>(GetWorldSettings()))
		{
			ExperienceId = TypedWorldSettings->GetDefaultGameplayExperience();
			ExperienceIdSource = TEXT("WorldSettings");
		}
	}

	UJoyAssetManager& AssetManager = UJoyAssetManager::Get();
	FAssetData Dummy;
	if (ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetData(ExperienceId, /*out*/ Dummy))
	{
		UE_LOG(LogJoyExperience, Error,
			TEXT("EXPERIENCE: Wanted to use %s but couldn't find it, falling back to the default)"),
			*ExperienceId.ToString());
		ExperienceId = FPrimaryAssetId();
	}

	// Final fallback to the default experience
	if (!ExperienceId.IsValid())
	{
		//@TODO: Pull this from a config setting or something
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType("JoyExperienceDefinition"), FName("B_JoyExpDef_Default"));
		ExperienceIdSource = TEXT("Default");
	}

	OnMatchAssignmentGiven(ExperienceId, ExperienceIdSource);
}

void AJoyGameMode::OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource)
{
	if (ExperienceId.IsValid())
	{
		UE_LOG(LogJoyExperience, Log, TEXT("Identified experience %s (Source: %s)"), *ExperienceId.ToString(),
			*ExperienceIdSource);

		auto* ExperienceComponent = GameState->FindComponentByClass<UJoyExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->SetCurrentExperience(ExperienceId);
	}
	else
	{
		UE_LOG(LogJoyExperience, Error, TEXT("Failed to identify experience, loading screen will stay up forever"));
	}
}

void AJoyGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	if (UJoyPlayerSpawningManagerComponent* PlayerSpawningComponent =
		GameState->FindComponentByClass<UJoyPlayerSpawningManagerComponent>())
	{
		PlayerSpawningComponent->FinishRestartPlayer(NewPlayer, StartRotation);
	}

	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

void AJoyGameMode::GenericPlayerInitialization(AController* NewPlayer)
{
	Super::GenericPlayerInitialization(NewPlayer);

	OnGameModePlayerInitialized.Broadcast(this, NewPlayer);
}

const UJoyPawnData* AJoyGameMode::GetPawnDataForController(const AController* InController) const
{
	if (InController != nullptr)
	{
		if (const AJoyPlayerState* JoyPS = InController->GetPlayerState<AJoyPlayerState>())
		{
			if (const UJoyPawnData* PawnData = JoyPS->GetPawnData<UJoyPawnData>())
			{
				return PawnData;
			}
		}
	}

	return nullptr;
}
