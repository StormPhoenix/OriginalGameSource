// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonGameInstance.h"
#include "JoyGameInstance.generated.h"

class AJoyPlayerController;
class UObject;

UCLASS(Config = Game)
class ORIGINALGAME_API UJoyGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public:
	UJoyGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	AJoyPlayerController* GetPrimaryPlayerController() const;
	
	virtual bool CanJoinRequestedSession() const override;

protected:
	virtual void OnStart() override;
	virtual void Init() override;
	virtual void Shutdown() override;
};
