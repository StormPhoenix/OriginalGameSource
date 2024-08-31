// Fill out your copyright notice in the Description page of Project Settings.


#include "QinCharacter.h"

#include "QinCharacterMovementComponent.h"
#include "OriginalGame/Player/QinPlayerState.h"

AQinCharacter::AQinCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UQinCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	
	UQinCharacterMovementComponent* QinMoveComp = CastChecked<UQinCharacterMovementComponent>(GetCharacterMovement());
	QinMoveComp->GravityScale = 1.0f;
	QinMoveComp->MaxAcceleration = 2400.0f;
	QinMoveComp->BrakingFrictionFactor = 1.0f;
	QinMoveComp->BrakingFriction = 6.0f;
	QinMoveComp->GroundFriction = 8.0f;
	QinMoveComp->BrakingDecelerationWalking = 1400.0f;
	QinMoveComp->bUseControllerDesiredRotation = false;
	QinMoveComp->bOrientRotationToMovement = false;
	QinMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	QinMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	QinMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	QinMoveComp->bCanWalkOffLedgesWhenCrouching = true;
	QinMoveComp->SetCrouchedHalfHeight(65.0f);
}

void AQinCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AQinCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AQinCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

AQinPlayerState* AQinCharacter::GetQinPlayerState() const
{
	return CastChecked<AQinPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}
