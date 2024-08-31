// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "JoyGameMode.generated.h"

class UJoyPawnData;
class UJoyExperienceDefinition;

/**
 * Post login event, triggered when a player or bot joins the game as well as after seamless and non seamless travel
 *
 * This is called after the player has finished initialization
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLyraGameModePlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);

UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class ORIGINALGAME_API AJoyGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	AJoyGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AGameModeBase interface
	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	void InitGameState() override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	virtual void GenericPlayerInitialization(AController* NewPlayer) override;
	//~End of AGameModeBase interface

	FOnLyraGameModePlayerInitialized OnGameModePlayerInitialized;
protected:
	const UJoyPawnData* GetPawnDataForController(const AController* InController) const;
	
	void OnExperienceLoaded(const UJoyExperienceDefinition* CurrentExperience);
	
	bool IsExperienceLoaded() const;
	
	void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);

	void HandleMatchAssignmentIfNotExpectingOne();
};
