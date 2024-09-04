// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "JoyAISpawner.generated.h"

class UJoyPawnData;

UCLASS()
class ORIGINALGAME_API AJoyAISpawner : public AActor
{
	GENERATED_BODY()

public:
	AJoyAISpawner();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual void BeginPlay() override;

#if WITH_EDITORONLY_DATA
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> Mesh{nullptr};

	UPROPERTY()
	TObjectPtr<UBillboardComponent> SpriteComponent{nullptr};
#endif

public:
	UPROPERTY(EditInstanceOnly)
	TObjectPtr<const UJoyPawnData> PawnData{nullptr};

private:
	void ReloadPawnData() const;
};
