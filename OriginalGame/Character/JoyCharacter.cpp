// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyCharacter.h"

#include "Camera/JoyCameraComponent.h"
#include "JoyCharacterMovementComponent.h"
#include "JoyPawnExtensionComponent.h"
#include "OriginalGame/Player/JoyPlayerState.h"

AJoyCharacter::AJoyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UJoyCharacterMovementComponent>(
		  ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	UJoyCharacterMovementComponent* JoyMoveComp = CastChecked<UJoyCharacterMovementComponent>(GetCharacterMovement());
	JoyMoveComp->GravityScale = 1.0f;
	JoyMoveComp->MaxAcceleration = 2400.0f;
	JoyMoveComp->BrakingFrictionFactor = 1.0f;
	JoyMoveComp->BrakingFriction = 6.0f;
	JoyMoveComp->GroundFriction = 8.0f;
	JoyMoveComp->BrakingDecelerationWalking = 1400.0f;
	JoyMoveComp->bUseControllerDesiredRotation = false;
	JoyMoveComp->bOrientRotationToMovement = false;
	JoyMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	JoyMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	JoyMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	JoyMoveComp->bCanWalkOffLedgesWhenCrouching = true;
	JoyMoveComp->SetCrouchedHalfHeight(65.0f);

	PawnExtComponent = CreateDefaultSubobject<UJoyPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));
	
	JoyCameraComponent = CreateDefaultSubobject<UJoyCameraComponent>(TEXT("CameraComponent"));
}

void AJoyCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AJoyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AJoyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

AJoyPlayerState* AJoyCharacter::GetJoyPlayerState() const
{
	return CastChecked<AJoyPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

void AJoyCharacter::SetCustomTimeDilation(float const TimeDilation)
{
	if (IsActorBeingDestroyed())
	{
		return;
	}

	CustomTimeDilation = TimeDilation;
}
