// Copyright Epic Games, Inc. All Rights Reserved.

#include "JoySettingsShared.h"

#include "Framework/Application/SlateApplication.h"
#include "Internationalization/Culture.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Player/JoyLocalPlayer.h"
#include "Rendering/SlateRenderer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(JoySettingsShared)

static FString SHARED_SETTINGS_SLOT_NAME = TEXT("SharedGameSettings");

namespace JoySettingsSharedCVars
{
static float DefaultGamepadLeftStickInnerDeadZone = 0.25f;
static FAutoConsoleVariableRef CVarGamepadLeftStickInnerDeadZone(TEXT("gpad.DefaultLeftStickInnerDeadZone"),
	DefaultGamepadLeftStickInnerDeadZone, TEXT("Gamepad left stick inner deadzone"));

static float DefaultGamepadRightStickInnerDeadZone = 0.25f;
static FAutoConsoleVariableRef CVarGamepadRightStickInnerDeadZone(TEXT("gpad.DefaultRightStickInnerDeadZone"),
	DefaultGamepadRightStickInnerDeadZone, TEXT("Gamepad right stick inner deadzone"));
}	 // namespace JoySettingsSharedCVars

UJoySettingsShared::UJoySettingsShared()
{
	FInternationalization::Get().OnCultureChanged().AddUObject(this, &ThisClass::OnCultureChanged);

	GamepadMoveStickDeadZone = JoySettingsSharedCVars::DefaultGamepadLeftStickInnerDeadZone;
	GamepadLookStickDeadZone = JoySettingsSharedCVars::DefaultGamepadRightStickInnerDeadZone;
}

void UJoySettingsShared::Initialize(UJoyLocalPlayer* LocalPlayer)
{
	check(LocalPlayer);

	OwningPlayer = LocalPlayer;
}

void UJoySettingsShared::SaveSettings()
{
	check(OwningPlayer);
	UGameplayStatics::SaveGameToSlot(this, SHARED_SETTINGS_SLOT_NAME, OwningPlayer->GetLocalPlayerIndex());
}

/*static*/ UJoySettingsShared* UJoySettingsShared::LoadOrCreateSettings(const UJoyLocalPlayer* LocalPlayer)
{
	UJoySettingsShared* SharedSettings = nullptr;

	// If the save game exists, load it.
	if (UGameplayStatics::DoesSaveGameExist(SHARED_SETTINGS_SLOT_NAME, LocalPlayer->GetLocalPlayerIndex()))
	{
		USaveGame* Slot =
			UGameplayStatics::LoadGameFromSlot(SHARED_SETTINGS_SLOT_NAME, LocalPlayer->GetLocalPlayerIndex());
		SharedSettings = Cast<UJoySettingsShared>(Slot);
	}

	if (SharedSettings == nullptr)
	{
		SharedSettings =
			Cast<UJoySettingsShared>(UGameplayStatics::CreateSaveGameObject(UJoySettingsShared::StaticClass()));
	}

	SharedSettings->Initialize(const_cast<UJoyLocalPlayer*>(LocalPlayer));
	SharedSettings->ApplySettings();

	return SharedSettings;
}

void UJoySettingsShared::ApplySettings()
{
	ApplyBackgroundAudioSettings();
	ApplyCultureSettings();
}

void UJoySettingsShared::SetColorBlindStrength(int32 InColorBlindStrength)
{
	InColorBlindStrength = FMath::Clamp(InColorBlindStrength, 0, 10);
	if (ColorBlindStrength != InColorBlindStrength)
	{
		ColorBlindStrength = InColorBlindStrength;
		FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
			(EColorVisionDeficiency) (int32) ColorBlindMode, (int32) ColorBlindStrength, true, false);
	}
}

int32 UJoySettingsShared::GetColorBlindStrength() const
{
	return ColorBlindStrength;
}

void UJoySettingsShared::SetColorBlindMode(EColorBlindMode InMode)
{
	if (ColorBlindMode != InMode)
	{
		ColorBlindMode = InMode;
		FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
			(EColorVisionDeficiency) (int32) ColorBlindMode, (int32) ColorBlindStrength, true, false);
	}
}

EColorBlindMode UJoySettingsShared::GetColorBlindMode() const
{
	return ColorBlindMode;
}

//////////////////////////////////////////////////////////////////////

void UJoySettingsShared::SetAllowAudioInBackgroundSetting(EJoyAllowBackgroundAudioSetting NewValue)
{
	if (ChangeValueAndDirty(AllowAudioInBackground, NewValue))
	{
		ApplyBackgroundAudioSettings();
	}
}

void UJoySettingsShared::ApplyBackgroundAudioSettings()
{
	if (OwningPlayer && OwningPlayer->IsPrimaryPlayer())
	{
		FApp::SetUnfocusedVolumeMultiplier(
			(AllowAudioInBackground != EJoyAllowBackgroundAudioSetting::Off) ? 1.0f : 0.0f);
	}
}

//////////////////////////////////////////////////////////////////////

void UJoySettingsShared::ApplyCultureSettings()
{
	if (bResetToDefaultCulture)
	{
		const FCulturePtr SystemDefaultCulture = FInternationalization::Get().GetDefaultCulture();
		check(SystemDefaultCulture.IsValid());

		const FString CultureToApply = SystemDefaultCulture->GetName();
		if (FInternationalization::Get().SetCurrentCulture(CultureToApply))
		{
			// Clear this string
			GConfig->RemoveKey(TEXT("Internationalization"), TEXT("Culture"), GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);
		}
		bResetToDefaultCulture = false;
	}
	else if (!PendingCulture.IsEmpty())
	{
		// SetCurrentCulture may trigger PendingCulture to be cleared (if a culture change is broadcast) so we take a
		// copy of it to work with
		const FString CultureToApply = PendingCulture;
		if (FInternationalization::Get().SetCurrentCulture(CultureToApply))
		{
			// Note: This is intentionally saved to the users config
			// We need to localize text before the player logs in and very early in the loading screen
			GConfig->SetString(TEXT("Internationalization"), TEXT("Culture"), *CultureToApply, GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);
		}
		ClearPendingCulture();
	}
}

void UJoySettingsShared::ResetCultureToCurrentSettings()
{
	ClearPendingCulture();
	bResetToDefaultCulture = false;
}

const FString& UJoySettingsShared::GetPendingCulture() const
{
	return PendingCulture;
}

void UJoySettingsShared::SetPendingCulture(const FString& NewCulture)
{
	PendingCulture = NewCulture;
	bResetToDefaultCulture = false;
	bIsDirty = true;
}

void UJoySettingsShared::OnCultureChanged()
{
	ClearPendingCulture();
	bResetToDefaultCulture = false;
}

void UJoySettingsShared::ClearPendingCulture()
{
	PendingCulture.Reset();
}

bool UJoySettingsShared::IsUsingDefaultCulture() const
{
	FString Culture;
	GConfig->GetString(TEXT("Internationalization"), TEXT("Culture"), Culture, GGameUserSettingsIni);

	return Culture.IsEmpty();
}

void UJoySettingsShared::ResetToDefaultCulture()
{
	ClearPendingCulture();
	bResetToDefaultCulture = true;
	bIsDirty = true;
}

//////////////////////////////////////////////////////////////////////

void UJoySettingsShared::ApplyInputSensitivity()
{
}
