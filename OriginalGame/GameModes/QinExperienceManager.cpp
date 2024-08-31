// Copyright Epic Games, Inc. All Rights Reserved.

#include "QinExperienceManager.h"
#include "QinExperienceManager.h"
#include "Engine/Engine.h"
#include "Subsystems/SubsystemCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(QinExperienceManager)

#if WITH_EDITOR

void UQinExperienceManager::OnPlayInEditorBegun()
{
	ensure(GameFeaturePluginRequestCountMap.IsEmpty());
	GameFeaturePluginRequestCountMap.Empty();
}

void UQinExperienceManager::NotifyOfPluginActivation(const FString PluginURL)
{
	if (GIsEditor)
	{
		UQinExperienceManager* ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<UQinExperienceManager>();
		check(ExperienceManagerSubsystem);

		// Track the number of requesters who activate this plugin. Multiple load/activation requests are always allowed because concurrent requests are handled.
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindOrAdd(PluginURL);
		++Count;
	}
}

bool UQinExperienceManager::RequestToDeactivatePlugin(const FString PluginURL)
{
	if (GIsEditor)
	{
		UQinExperienceManager* ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<UQinExperienceManager>();
		check(ExperienceManagerSubsystem);

		// Only let the last requester to get this far deactivate the plugin
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindChecked(PluginURL);
		--Count;

		if (Count == 0)
		{
			ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.Remove(PluginURL);
			return true;
		}

		return false;
	}

	return true;
}

#endif
