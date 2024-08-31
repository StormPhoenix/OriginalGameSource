// Copyright Epic Games, Inc. All Rights Reserved.

#include "QinLogChannels.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogQin);
DEFINE_LOG_CATEGORY(LogQinExperience);
DEFINE_LOG_CATEGORY(LogQinAbilitySystem);
DEFINE_LOG_CATEGORY(LogQinTeams);

FString GetClientServerContextString(UObject* ContextObject)
{
	ENetRole Role = ROLE_None;

	if (AActor* Actor = Cast<AActor>(ContextObject))
	{
		Role = Actor->GetLocalRole();
	}
	else if (UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return (Role == ROLE_Authority) ? TEXT("Server") : TEXT("Client");
	}
	else
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			return GPlayInEditorContextString;
		}
#endif
	}

	return TEXT("[]");
}
