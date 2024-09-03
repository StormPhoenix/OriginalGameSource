// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JoyCameraMode_ThirdPerson.h"

#include "JoyCameraMode_PlayerSwitching.generated.h"

UCLASS(Blueprintable)
class ORIGINALGAME_API UJoyCameraMode_PlayerSwitching : public UJoyCameraMode_ThirdPerson
{
	GENERATED_BODY()

public:
	UJoyCameraMode_PlayerSwitching();

	virtual FRotator CalcPivotRotation() const override;

	virtual void UpdateView(float DeltaTime) override;

	virtual void PreventCameraPenetration(class AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc,
		float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly) override;

	virtual void OnActivation() override;
};
