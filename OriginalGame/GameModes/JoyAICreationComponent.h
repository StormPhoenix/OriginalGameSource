// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/GameStateComponent.h"
#include "CoreMinimal.h"
#include "Character/JoyPawnData.h"

#include "JoyAICreationComponent.generated.h"

class AController;
class AJoyAISpawner;
class UJoyExperienceDefinition;
class UJoyPawnData;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ORIGINALGAME_API UJoyAICreationComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UJoyAICreationComponent(const FObjectInitializer& ObjectInitializer);

	AController* SpawnFromAISpawner(AJoyAISpawner* Spawner) const;

	AController* SpawnFromPawnData(const UJoyPawnData* PawnData, AActor* StartSpot) const;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void OnExperienceLoaded(const UJoyExperienceDefinition* Experience);

	void ServerCreateAI() const;
};
