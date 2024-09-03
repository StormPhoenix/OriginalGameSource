// Fill out your copyright notice in the Description page of Project Settings.


#include "JoySpectatorBase.h"

#include "EnhancedInputSubsystems.h"
#include "JoyGameplayTags.h"
#include "JoyPawnData.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/PlayerController.h"
#include "Input/JoyInputComponent.h"
#include "Player/JoyLocalPlayer.h"
#include "Player/JoyPlayerState.h"

namespace JoySpectator
{
	static constexpr float LookYawRate = 300.0f;
	static constexpr float LookPitchRate = 165.0f;
};

const FName AJoySpectatorBase::Name_BindInputsNow("BindInputsNow");
const FName AJoySpectatorBase::Name_ActorFeatureName("Spectator");

AJoySpectatorBase::AJoySpectatorBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AJoySpectatorBase::BeginPlay()
{
	Super::BeginPlay();
}

void AJoySpectatorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AJoySpectatorBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	auto* PC = GetController<APlayerController>();
	check(PC);

	const UJoyLocalPlayer* LP = Cast<UJoyLocalPlayer>(PC->GetLocalPlayer());
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	BindDefaultInputMappings(Subsystem, PlayerInputComponent);

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(PC, Name_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, Name_BindInputsNow);
}

void AJoySpectatorBase::BindDefaultInputMappings(UEnhancedInputLocalPlayerSubsystem* EnhanceInput,
                                                 UInputComponent* InInputComponent)
{
	AJoyPlayerState const* JoyPlayerState = Cast<AJoyPlayerState>(GetPlayerState());
	if (!JoyPlayerState)
	{
		return;
	}

	UJoyPawnData const* PawnData = JoyPlayerState->GetPawnData<UJoyPawnData>();
	if (!PawnData)
	{
		return;
	}

	const UJoyInputConfig* InputConfig = PawnData->InputConfig;
	if (!InputConfig)
	{
		return;
	}

	UJoyInputComponent* JoyIC = Cast<UJoyInputComponent>(InInputComponent);
	if (!(ensureMsgf(JoyIC,
	                 TEXT(
		                 "Unexpected Input Component class! Input component must be UJoyInputComponent or a subclass of it."
	                 ))))
	{
		return;
	}

	// Add the key mappings that may have been set by the player
	JoyIC->AddInputMappings(InputConfig, EnhanceInput);

	BindDefaultInputMappings_Impl(JoyIC, InputConfig);
}

void AJoySpectatorBase::BindDefaultInputMappings_Impl(UJoyInputComponent* JoyIC, const UJoyInputConfig* InputConfig)
{
	// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints
	// will be triggered directly by these input actions Triggered events.
	TArray<uint32> BindHandles;
	JoyIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed,
	                          &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

	JoyIC->BindNativeAction(InputConfig, JoyGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this,
	                        &ThisClass::Input_Move, /*bLogIfNotFound=*/false);
	JoyIC->BindNativeAction(InputConfig, JoyGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this,
	                        &ThisClass::Input_LookMove, /*bLogIfNotFound=*/false);
	JoyIC->BindNativeAction(InputConfig, JoyGameplayTags::InputTag_Look_Stick, ETriggerEvent::Triggered, this,
	                        &ThisClass::Input_LookStick, /*bLogIfNotFound=*/false);
}

void AJoySpectatorBase::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	// @TODO Waiting for GAS Framework
}

void AJoySpectatorBase::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	// @TODO Waiting for GAS Framework
}

void AJoySpectatorBase::Input_Move(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

	if (Value.X != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
		AddMovementInput(MovementDirection, Value.X);
	}

	if (Value.Y != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		AddMovementInput(MovementDirection, Value.Y);
	}
}

void AJoySpectatorBase::Input_LookMove(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	if (Value.X != 0.0f)
	{
		AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		AddControllerPitchInput(Value.Y);
	}
}

void AJoySpectatorBase::Input_LookStick(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	const UWorld* World = GetWorld();
	check(World);

	if (Value.X != 0.0f)
	{
		AddControllerYawInput(Value.X * JoySpectator::LookYawRate * World->GetDeltaSeconds());
	}

	if (Value.Y != 0.0f)
	{
		AddControllerPitchInput(Value.Y * JoySpectator::LookPitchRate * World->GetDeltaSeconds());
	}
}
