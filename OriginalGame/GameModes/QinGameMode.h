// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "QinGameMode.generated.h"

class UQinExperienceDefinition;

UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class ORIGINALGAME_API AQinGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	AQinGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AGameModeBase interface
	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	void InitGameState();
	//~End of AGameModeBase interface

protected:
	void OnExperienceLoaded(const UQinExperienceDefinition* CurrentExperience);
	bool IsExperienceLoaded() const;
	
	void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);

	void HandleMatchAssignmentIfNotExpectingOne();
};
