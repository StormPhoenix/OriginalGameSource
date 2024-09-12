// Copyright Epic Games, Inc. All Rights Reserved.

#include "JoyGameInstance.h"

#include "Components/GameFrameworkComponentManager.h"
#include "JoyGameplayTags.h"
#include "Player/JoyPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "..\TypeScript\JoyTypeScriptGameInstanceSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(JoyGameInstance)

UJoyGameInstance::UJoyGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UJoyGameInstance::OnStart()
{
	if (auto* TsSystem = UJoyTypeScriptGameInstanceSystem::Get(GetWorld()))
	{
		TsSystem->Start();
	}

	Super::OnStart();
}

void UJoyGameInstance::Init()
{
	Super::Init();

	// Register our custom init states
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(JoyGameplayTags::InitState_Spawned, false, FGameplayTag());
		ComponentManager->RegisterInitState(JoyGameplayTags::InitState_DataAvailable, false,
			JoyGameplayTags::InitState_Spawned);
		ComponentManager->RegisterInitState(JoyGameplayTags::InitState_DataInitialized, false,
			JoyGameplayTags::InitState_DataAvailable);
		ComponentManager->RegisterInitState(JoyGameplayTags::InitState_GameplayReady, false,
			JoyGameplayTags::InitState_DataInitialized);
	}
}

void UJoyGameInstance::Shutdown()
{
	Super::Shutdown();
}

AJoyPlayerController* UJoyGameInstance::GetPrimaryPlayerController() const
{
	return Cast<AJoyPlayerController>(Super::GetPrimaryPlayerController(false));
}

bool UJoyGameInstance::CanJoinRequestedSession() const
{
	return Super::CanJoinRequestedSession();
}
