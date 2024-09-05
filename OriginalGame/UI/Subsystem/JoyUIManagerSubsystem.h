// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "GameUIManagerSubsystem.h"

#include "JoyUIManagerSubsystem.generated.h"

class FSubsystemCollectionBase;
class UObject;

UCLASS()
class UJoyUIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:

	UJoyUIManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	bool Tick(float DeltaTime);
	void SyncRootLayoutVisibilityToShowHUD();
	
	FTSTicker::FDelegateHandle TickHandle;
};
