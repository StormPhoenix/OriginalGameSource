// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "JoyGravityManageSubsystem.generated.h"

/**
 *
 */
UCLASS()
class ORIGINALGAME_API UJoyGravityManageSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	static UJoyGravityManageSubsystem* Get(const UWorld* World);
	static UJoyGravityManageSubsystem* GetGravityManageSubsystem(const UObject* WorldContextObject);

	//~FTickableGameObject begin
	virtual UWorld* GetTickableGameObjectWorld() const override;

	virtual void Tick(float DeltaTime) override;

	virtual bool IsTickable() const override;

	virtual ETickableTickType GetTickableTickType() const override;

	virtual TStatId GetStatId() const override;
	//~FTickableGameObject end

	FVector LocalVectorToWorld(const FVector& LocalVector) const;

	FRotator WorldRotatorToLocal(const FRotator& WorldRotator) const;

	FRotator LocalRotatorToWorld(const FRotator& LocalRotator) const;

	FVector WorldVectorToLocal(const FVector& WorldVector) const;

	void SetGravityDirection(const FVector& GravityDirection);

	FRotator GetGravitySpaceTransform() const;

	FRotator GetInverseGravitySpaceTransform() const;

	FVector GetGravitySpaceX() const;

	FVector GetGravitySpaceY() const;

	FVector GetGravitySpaceZ() const;

protected:
	void UpdateGravityDirection();

	// 缓存当前的重力方向
	UPROPERTY()
	FVector CacheGravityDirection{0., 0., -1.};

	UPROPERTY()
	bool bGravityChanged{true};

	UPROPERTY()
	FVector BaseCoordinateX{1.0, 0.0, 0.0};

	UPROPERTY()
	FVector BaseCoordinateY{0.0, 1.0, 0.0};

	UPROPERTY()
	FVector BaseCoordinateZ{0.0, 0.0, 1.0};

	UPROPERTY()
	FRotator BaseSpaceTransform = FRotator::ZeroRotator;

	UPROPERTY()
	FRotator InverseBaseSpaceTransform = FRotator::ZeroRotator;
};
