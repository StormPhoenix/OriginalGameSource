// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerState.h"
#include "JoyPlayerState.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class ORIGINALGAME_API AJoyPlayerState : public AModularPlayerState
{
	GENERATED_BODY()
	
public:
	AJoyPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	void SetPawnData(const UJoyPawnData* InPawnData);
	
protected:
	TObjectPtr<const UJoyPawnData> PawnData;
};
