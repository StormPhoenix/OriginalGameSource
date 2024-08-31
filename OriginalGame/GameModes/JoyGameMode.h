// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "JoyGameMode.generated.h"

class UJoyExperienceDefinition;

UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class ORIGINALGAME_API AJoyGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	AJoyGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AGameModeBase interface
	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	void InitGameState();
	//~End of AGameModeBase interface

protected:
	void OnExperienceLoaded(const UJoyExperienceDefinition* CurrentExperience);
	bool IsExperienceLoaded() const;
	
	void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);

	void HandleMatchAssignmentIfNotExpectingOne();
};
