// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "JoyCameraControllerBase.generated.h"

class AJoyPlayerCameraManager;

UCLASS()
class ORIGINALGAME_API UJoyCameraControllerBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void InitializeFor(AJoyPlayerCameraManager* PCMgr);

	virtual void Update(float DeltaSeconds);

	bool IsActive() const
	{
		return bIsActive;
	}

	void Lock(UJoyCameraControllerBase* Other);

	void Unlock(UJoyCameraControllerBase* Other);

	class AJoyPlayerCameraManager* GetPlayerCameraManager() const;

protected:
	virtual void OnEnable();

	virtual void OnDisable();

	virtual void UpdateInternal(float DeltaSeconds);

	virtual void UpdateDeactivateInternal(float DeltaSeconds);

	UPROPERTY()
	TObjectPtr<class AJoyPlayerCameraManager> CameraManager;

	bool bIsActive = true;
};
