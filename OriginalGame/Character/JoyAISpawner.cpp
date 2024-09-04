// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyAISpawner.h"

#include "Components/BillboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "JoyPawnData.h"
#include "UObject/ConstructorHelpers.h"

AJoyAISpawner::AJoyAISpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));

#if WITH_EDITORONLY_DATA
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	Mesh->SetupAttachment(RootComponent);

	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> DecalTexture;
			FName ID_Formation;
			FText NAME_Formation;

			FConstructorStatics()
				: DecalTexture(TEXT("/Engine/EditorResources/Spawn_Point"))
				, ID_Formation(TEXT("Formation"))
				, NAME_Formation(NSLOCTEXT("SpriteCategory", "Formation", "Formation"))
			{
			}
		};

		static FConstructorStatics ConstructorStatics;
		if (SpriteComponent)
		{
			SpriteComponent->Sprite = ConstructorStatics.DecalTexture.Get();
			SpriteComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));

			SpriteComponent->bHiddenInGame = true;
			SpriteComponent->SetVisibleFlag(true);
			SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Formation;
			SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Formation;
			SpriteComponent->SetupAttachment(RootComponent);
			SpriteComponent->SetAbsolute(false, false, true);
			SpriteComponent->bIsScreenSizeScaled = true;
		}
	}
#endif	  // WITH_EDITORONLY_DATA
}

void AJoyAISpawner::BeginPlay()
{
	Super::BeginPlay();
}

void AJoyAISpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AJoyAISpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ReloadPawnData();
}

void AJoyAISpawner::ReloadPawnData() const
{
	if (PawnData == nullptr || PawnData->PawnClass == nullptr)
	{
		return;
	}

	UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(RootComponent);
	if (ACharacter const* CharacterCDO = Cast<ACharacter>(PawnData->PawnClass->GetDefaultObject()))
	{
		CapsuleComponent->InitCapsuleSize(CharacterCDO->GetCapsuleComponent()->GetScaledCapsuleRadius(),
			CharacterCDO->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

#if WITH_EDITOR
		if (GetWorld() && !GetWorld()->HasBegunPlay())
		{
			Mesh->SetRelativeTransform(CharacterCDO->GetMesh()->GetRelativeTransform());
			Mesh->SetSkeletalMesh(CharacterCDO->GetMesh()->GetSkeletalMeshAsset());
		}
#endif
	}
}

#if WITH_EDITOR
void AJoyAISpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName const PropertyName =
		PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, PawnData))
	{
		ReloadPawnData();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
