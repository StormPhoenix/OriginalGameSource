// Copyright Epic Games, Inc. All Rights Reserved.

#include "JoyPlayerSpawningManagerComponent.h"

#include "JoyPlayerController.h"
#include "Engine/PlayerStartPIE.h"
#include "Gameplay/JoyCharacterControlManageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(JoyPlayerSpawningManagerComponent)

DEFINE_LOG_CATEGORY_STATIC(LogPlayerSpawning, Log, All);

UJoyPlayerSpawningManagerComponent::UJoyPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
	bAutoRegister = true;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UJoyPlayerSpawningManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);

	if (const UWorld* World = GetWorld())
	{
		World->AddOnActorSpawnedHandler(
			FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::HandleOnActorSpawned));
	}
}

void UJoyPlayerSpawningManagerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UJoyPlayerSpawningManagerComponent::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	OnFinishRestartPlayer(NewPlayer, StartRotation);
	K2_OnFinishRestartPlayer(NewPlayer, StartRotation);
}

void UJoyPlayerSpawningManagerComponent::OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation)
{
	auto* CharacterControlManager = UJoyCharacterControlManageSubsystem::GetCharacterControlManageSubsystem(this);
	auto* PlayerController = Cast<AJoyPlayerController>(Player);
	if (CharacterControlManager && PlayerController)
	{
		if (!PlayerController->OnPlayerTargetSwitchFinishedDelegate.IsBoundToObject(CharacterControlManager))
		{
			PlayerController->OnPlayerTargetSwitchFinishedDelegate.AddUObject(CharacterControlManager,
				&UJoyCharacterControlManageSubsystem::OnCharacterSwitchFinished);
		}
	}
}
