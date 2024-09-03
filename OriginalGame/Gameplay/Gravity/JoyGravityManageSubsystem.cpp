// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyGravityManageSubsystem.h"

#include "Kismet/KismetMathLibrary.h"

UJoyGravityManageSubsystem* UJoyGravityManageSubsystem::Get(const UWorld* World)
{
	if (World)
	{
		return UGameInstance::GetSubsystem<UJoyGravityManageSubsystem>(World->GetGameInstance());
	}

	return nullptr;
}

UJoyGravityManageSubsystem* UJoyGravityManageSubsystem::GetGravityManageSubsystem(const UObject* WorldContextObject)
{
	if (UWorld const* World =
			GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return UJoyGravityManageSubsystem::Get(World);
	}

	return nullptr;
}

UWorld* UJoyGravityManageSubsystem::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

bool UJoyGravityManageSubsystem::IsTickable() const
{
	return !IsTemplate();
}

ETickableTickType UJoyGravityManageSubsystem::GetTickableTickType() const
{
	return (HasAnyFlags(RF_ClassDefaultObject) ? ETickableTickType::Never : ETickableTickType::Conditional);
}

TStatId UJoyGravityManageSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UJoyGravityManageSubsystem, STATGROUP_Tickables);
}

void UJoyGravityManageSubsystem::Tick(float DeltaTime)
{
	UpdateGravityDirection();
}

void UJoyGravityManageSubsystem::UpdateGravityDirection()
{
	if (!bGravityChanged)
	{
		return;
	}

	BaseCoordinateZ = -CacheGravityDirection.GetSafeNormal();
	if ((BaseCoordinateZ - FVector(0., 0., 1.)).IsNearlyZero())
	{
		BaseCoordinateX = FVector(1.0f, 0.0f, 0.0f);
		BaseCoordinateY = FVector(0.0f, 1.0f, 0.0f);
	}
	else if ((BaseCoordinateZ - FVector(0., 0., -1.)).IsNearlyZero())
	{
		BaseCoordinateX = FVector(-1.0f, 0.0f, 0.0f);
		BaseCoordinateY = FVector(0.0f, 1.0f, 0.0f);
	}
	else
	{
		const FVector WorldUp = FVector(0., 0., 1.);
		BaseCoordinateX = BaseCoordinateZ.Cross(WorldUp).GetSafeNormal();
		BaseCoordinateY = BaseCoordinateZ.Cross(BaseCoordinateX).GetSafeNormal();
	}

	BaseSpaceTransform = UKismetMathLibrary::MakeRotationFromAxes(BaseCoordinateX, BaseCoordinateY, BaseCoordinateZ);
	InverseBaseSpaceTransform = BaseSpaceTransform.GetInverse();
	bGravityChanged = false;
}

void UJoyGravityManageSubsystem::SetGravityDirection(const FVector& GravityDirection)
{
	CacheGravityDirection = GravityDirection;
	bGravityChanged = true;
}

FRotator UJoyGravityManageSubsystem::GetGravitySpaceTransform() const
{
	return BaseSpaceTransform;
}

FRotator UJoyGravityManageSubsystem::GetInverseGravitySpaceTransform() const
{
	return InverseBaseSpaceTransform;
}

FVector UJoyGravityManageSubsystem::GetGravitySpaceX() const
{
	return BaseCoordinateX;
}

FVector UJoyGravityManageSubsystem::GetGravitySpaceY() const
{
	return BaseCoordinateY;
}

FVector UJoyGravityManageSubsystem::GetGravitySpaceZ() const
{
	return BaseCoordinateZ;
}

FVector UJoyGravityManageSubsystem::LocalVectorToWorld(const FVector& LocalVector) const
{
	return BaseSpaceTransform.RotateVector(LocalVector);
}

FVector UJoyGravityManageSubsystem::WorldVectorToLocal(const FVector& WorldVector) const
{
	return InverseBaseSpaceTransform.RotateVector(WorldVector);
}

FRotator UJoyGravityManageSubsystem::WorldRotatorToLocal(const FRotator& WorldRotator) const
{
	return UKismetMathLibrary::ComposeRotators(WorldRotator, InverseBaseSpaceTransform);
}

FRotator UJoyGravityManageSubsystem::LocalRotatorToWorld(const FRotator& LocalRotator) const
{
	return UKismetMathLibrary::ComposeRotators(LocalRotator, BaseSpaceTransform);
}
