// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyHeroComponent.h"

#include "Components/GameFrameworkComponentDelegates.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/JoyCharacterControlManageSubsystem.h"
#include "InputActionValue.h"
#include "JoyCharacter.h"
#include "JoyGameBlueprintLibrary.h"
#include "JoyGameplayTags.h"
#include "JoyHeroCharacter.h"
#include "JoyPawnExtensionComponent.h"
#include "Player/JoyPlayerController.h"
#include "Utils/JoyCameraBlueprintLibrary.h"
#include "Utils/JoyCharacterBlueprintLibrary.h"

const FName UJoyHeroComponent::NAME_ActorFeatureName("Hero");

UJoyHeroComponent::UJoyHeroComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UJoyHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	BindOnActorInitStateChanged(UJoyPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(JoyGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UJoyHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UJoyPawnExtensionComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == JoyGameplayTags::InitState_DataInitialized)
		{
			CheckDefaultInitialization();
		}
	}
}

void UJoyHeroComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = {JoyGameplayTags::InitState_Spawned,
		JoyGameplayTags::InitState_DataAvailable, JoyGameplayTags::InitState_DataInitialized,
		JoyGameplayTags::InitState_GameplayReady};
	ContinueInitStateChain(StateChain);
}

void UJoyHeroComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UJoyHeroComponent::OnRegister()
{
	Super::OnRegister();

	if (GetPawn<AJoyHeroCharacter>())
	{
		RegisterInitStateFeature();
	}
}

bool UJoyHeroComponent::CanChangeInitState(
	UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();
	if (!CurrentState.IsValid() && DesiredState == JoyGameplayTags::InitState_Spawned)
	{
		if (Pawn)
		{
			return true;
		}
	}
	else if (CurrentState == JoyGameplayTags::InitState_Spawned &&
			 DesiredState == JoyGameplayTags::InitState_DataAvailable)
	{
		if (!GetPlayerState<AJoyPlayerState>())
		{
			return false;
		}

		// If we're authority or autonomous, we need to wait for a controller with registered ownership of the player
		// state.
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			const AController* Controller = GetController<AController>();
			const bool bHasControllerPairedWithPS = (Controller != nullptr) && (Controller->PlayerState != nullptr) &&
													(Controller->PlayerState->GetOwner() == Controller);

			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();

		if (bIsLocallyControlled && !bIsBot)
		{
			const AJoyPlayerController* JoyPC = GetController<AJoyPlayerController>();
			if (!Pawn->InputComponent || !JoyPC || !JoyPC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == JoyGameplayTags::InitState_DataAvailable &&
			 DesiredState == JoyGameplayTags::InitState_DataInitialized)
	{
		// Wait for player state and extension component
		const AJoyPlayerState* JoyPS = GetPlayerState<AJoyPlayerState>();
		return JoyPS && Manager->HasFeatureReachedInitState(Pawn, UJoyPawnExtensionComponent::NAME_ActorFeatureName,
							JoyGameplayTags::InitState_DataInitialized);
	}
	else if (CurrentState == JoyGameplayTags::InitState_DataInitialized &&
			 DesiredState == JoyGameplayTags::InitState_GameplayReady)
	{
		// TODO add ability initialization checks?
		return true;
	}

	return false;
}

void UJoyHeroComponent::HandleChangeInitState(
	UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == JoyGameplayTags::InitState_DataAvailable &&
		DesiredState == JoyGameplayTags::InitState_DataInitialized)
	{
		const APawn* Pawn = GetPawn<APawn>();
		const AJoyPlayerState* JoyPS = GetPlayerState<AJoyPlayerState>();
		if (!ensure(Pawn && JoyPS))
		{
			return;
		}

		UJoyGameBlueprintLibrary::RegisterInputReceiver(this, this);
	}
}

void UJoyHeroComponent::ReceiveMoveInput_Implementation(
	UObject* InputReceiver, const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	auto* Controller = UJoyGameBlueprintLibrary::GetJoyPlayerController(GetOwner());
	auto* HeroCharacter = Cast<AJoyHeroCharacter>(GetOwner());
	if (Controller && HeroCharacter && UJoyCharacterBlueprintLibrary::CheckCharacterControlled(HeroCharacter))
	{
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			HeroCharacter->AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			HeroCharacter->AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void UJoyHeroComponent::ReceiveAbilityTagPressInput_Implementation(UObject* InputReceiver, FGameplayTag const& InputTag)
{
	// @TODO GAS
}

void UJoyHeroComponent::ReceiveAbilityTagReleaseInput_Implementation(
	UObject* InputReceiver, FGameplayTag const& InputTag)
{
	// @TODO GAS
}

void UJoyHeroComponent::ReceiveLookMoveInput_Implementation(
	UObject* InputReceiver, const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	auto* HeroCharacter = Cast<AJoyHeroCharacter>(GetOwner());

	auto* PlayerCameraManager = UJoyCameraBlueprintLibrary::GetJoyPlayerCameraManager(GetOwner());
	if (UJoyCharacterBlueprintLibrary::CheckCharacterControlled(HeroCharacter) && PlayerCameraManager != nullptr)
	{
		// @TODO 此处的输入交给 CameraInputController 控制
		if (Value.X != 0.0f)
		{
			PlayerCameraManager->AddDeviceYawInput(Value.X);
		}

		if (Value.Y != 0.0f)
		{
			PlayerCameraManager->AddDevicePitchInput(Value.Y);
		}
	}
}
