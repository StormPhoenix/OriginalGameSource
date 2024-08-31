// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "JoyPawnCreationComponent.generated.h"

class UJoyPawnData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ORIGINALGAME_API UJoyPawnCreationComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UJoyPawnCreationComponent(const FObjectInitializer& ObjectInitializer);

	void SpawnPawnFromPawnData(const UJoyPawnData* PawnData) const;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
