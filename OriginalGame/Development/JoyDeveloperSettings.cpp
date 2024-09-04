// Copyright Epic Games, Inc. All Rights Reserved.

#include "JoyDeveloperSettings.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Misc/App.h"
#include "Widgets/Notifications/SNotificationList.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(JoyDeveloperSettings)

#define LOCTEXT_NAMESPACE "JoyCheats"

UJoyDeveloperSettings::UJoyDeveloperSettings()
{
}

FName UJoyDeveloperSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

#if WITH_EDITOR
void UJoyDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	ApplySettings();
}

void UJoyDeveloperSettings::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
	Super::PostReloadConfig(PropertyThatWasLoaded);

	ApplySettings();
}

void UJoyDeveloperSettings::PostInitProperties()
{
	Super::PostInitProperties();

	ApplySettings();
}

void UJoyDeveloperSettings::ApplySettings()
{
}

void UJoyDeveloperSettings::OnPlayInEditorStarted() const
{
	// Show a notification toast to remind the user that there's an experience override set
	if (ExperienceOverride.IsValid())
	{
		FNotificationInfo Info(
			FText::Format(LOCTEXT("ExperienceOverrideActive", "Developer Settings Override\nExperience {0}"),
				FText::FromName(ExperienceOverride.PrimaryAssetName)));
		Info.ExpireDuration = 2.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
}
#endif

#undef LOCTEXT_NAMESPACE
