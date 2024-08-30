// Fill out your copyright notice in the Description page of Project Settings.


#include "ScorpioCharacter.h"

#include "ScorpioCharacterMovementComponent.h"
#include "OriginalGame/Player/ScorpioPlayerState.h"

AScorpioCharacter::AScorpioCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UScorpioCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	
	UScorpioCharacterMovementComponent* LyraMoveComp = CastChecked<UScorpioCharacterMovementComponent>(GetCharacterMovement());
	LyraMoveComp->GravityScale = 1.0f;
	LyraMoveComp->MaxAcceleration = 2400.0f;
	LyraMoveComp->BrakingFrictionFactor = 1.0f;
	LyraMoveComp->BrakingFriction = 6.0f;
	LyraMoveComp->GroundFriction = 8.0f;
	LyraMoveComp->BrakingDecelerationWalking = 1400.0f;
	LyraMoveComp->bUseControllerDesiredRotation = false;
	LyraMoveComp->bOrientRotationToMovement = false;
	LyraMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	LyraMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	LyraMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	LyraMoveComp->bCanWalkOffLedgesWhenCrouching = true;
	LyraMoveComp->SetCrouchedHalfHeight(65.0f);
}

void AScorpioCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AScorpioCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AScorpioCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

AScorpioPlayerState* AScorpioCharacter::GetScorpioPlayerState() const
{
	return CastChecked<AScorpioPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}
