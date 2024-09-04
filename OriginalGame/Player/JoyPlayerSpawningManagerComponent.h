// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/GameStateComponent.h"

#include "JoyPlayerSpawningManagerComponent.generated.h"

class AController;
class APlayerController;
class APlayerState;
class APlayerStart;
class ALyraPlayerStart;
class AActor;

/**
 * @class UJoyPlayerSpawningManagerComponent
 */
UCLASS()
class ORIGINALGAME_API UJoyPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UJoyPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeComponent() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);

protected:
	virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName=OnFinishRestartPlayer))
	void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

private:
	void OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
	{
	}

	void HandleOnActorSpawned(AActor* SpawnedActor)
	{
	}
};
