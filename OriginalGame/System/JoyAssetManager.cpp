// Copyright Epic Games, Inc. All Rights Reserved.

#include "JoyAssetManager.h"

#include "AbilitySystem/JoyGameplayCueManager.h"
#include "AbilitySystemGlobals.h"
#include "Engine/Engine.h"
#include "JoyLogChannels.h"
#include "Misc/App.h"
#include "Misc/ScopedSlowTask.h"
#include "Stats/StatsMisc.h"
#include "System/JoyAssetManagerStartupJob.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(JoyAssetManager)

const FName FJoyBundles::Equipped("Equipped");

//////////////////////////////////////////////////////////////////////

static FAutoConsoleCommand CVarDumpLoadedAssets(TEXT("Joy.DumpLoadedAssets"),
	TEXT("Shows all assets that were loaded via the asset manager and are currently in memory."),
	FConsoleCommandDelegate::CreateStatic(UJoyAssetManager::DumpLoadedAssets));

//////////////////////////////////////////////////////////////////////

#define STARTUP_JOB_WEIGHTED(JobFunc, JobWeight)                                                                  \
	StartupJobs.Add(FJoyAssetManagerStartupJob(                                                                   \
		#JobFunc, [this](const FJoyAssetManagerStartupJob& StartupJob, TSharedPtr<FStreamableHandle>& LoadHandle) \
		{ JobFunc; }, JobWeight))
#define STARTUP_JOB(JobFunc) STARTUP_JOB_WEIGHTED(JobFunc, 1.f)

//////////////////////////////////////////////////////////////////////

UJoyAssetManager::UJoyAssetManager()
{
}

UJoyAssetManager& UJoyAssetManager::Get()
{
	check(GEngine);

	if (UJoyAssetManager* Singleton = Cast<UJoyAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	UE_LOG(
		LogJoy, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to JoyAssetManager!"));

	// Fatal error above prevents this from being called.
	return *NewObject<UJoyAssetManager>();
}

UObject* UJoyAssetManager::SynchronousLoadAsset(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsValid())
	{
		TUniquePtr<FScopeLogTime> LogTimePtr;

		if (ShouldLogAssetLoads())
		{
			LogTimePtr = MakeUnique<FScopeLogTime>(
				*FString::Printf(TEXT("Synchronously loaded asset [%s]"), *AssetPath.ToString()), nullptr,
				FScopeLogTime::ScopeLog_Seconds);
		}

		if (UAssetManager::IsValid())
		{
			return UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false);
		}

		// Use LoadObject if asset manager isn't ready yet.
		return AssetPath.TryLoad();
	}

	return nullptr;
}

bool UJoyAssetManager::ShouldLogAssetLoads()
{
	static bool bLogAssetLoads = FParse::Param(FCommandLine::Get(), TEXT("LogAssetLoads"));
	return bLogAssetLoads;
}

void UJoyAssetManager::AddLoadedAsset(const UObject* Asset)
{
	if (ensureAlways(Asset))
	{
		FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);
		LoadedAssets.Add(Asset);
	}
}

void UJoyAssetManager::DumpLoadedAssets()
{
	UE_LOG(LogJoy, Log, TEXT("========== Start Dumping Loaded Assets =========="));

	for (const UObject* LoadedAsset : Get().LoadedAssets)
	{
		UE_LOG(LogJoy, Log, TEXT("  %s"), *GetNameSafe(LoadedAsset));
	}

	UE_LOG(LogJoy, Log, TEXT("... %d assets in loaded pool"), Get().LoadedAssets.Num());
	UE_LOG(LogJoy, Log, TEXT("========== Finish Dumping Loaded Assets =========="));
}

void UJoyAssetManager::StartInitialLoading()
{
	SCOPED_BOOT_TIMING("UJoyAssetManager::StartInitialLoading");

	// This does all of the scanning, need to do this now even if loads are deferred
	Super::StartInitialLoading();

	STARTUP_JOB(InitializeAbilitySystem());
	STARTUP_JOB(InitializeGameplayCueManager());

	// Run all the queued up startup jobs
	DoAllStartupJobs();
}

void UJoyAssetManager::InitializeAbilitySystem()
{
	SCOPED_BOOT_TIMING("UJoyAssetManager::InitializeAbilitySystem");

	UAbilitySystemGlobals::Get().InitGlobalData();
}

void UJoyAssetManager::InitializeGameplayCueManager()
{
	SCOPED_BOOT_TIMING("UJoyAssetManager::InitializeGameplayCueManager");

	UJoyGameplayCueManager* GCM = UJoyGameplayCueManager::Get();
	check(GCM);
	GCM->LoadAlwaysLoadedCues();
}

void UJoyAssetManager::DoAllStartupJobs()
{
	SCOPED_BOOT_TIMING("UJoyAssetManager::DoAllStartupJobs");
	const double AllStartupJobsStartTime = FPlatformTime::Seconds();

	if (IsRunningDedicatedServer())
	{
		// No need for periodic progress updates, just run the jobs
		for (const FJoyAssetManagerStartupJob& StartupJob : StartupJobs)
		{
			StartupJob.DoJob();
		}
	}
	else
	{
		if (StartupJobs.Num() > 0)
		{
			float TotalJobValue = 0.0f;
			for (const FJoyAssetManagerStartupJob& StartupJob : StartupJobs)
			{
				TotalJobValue += StartupJob.JobWeight;
			}

			float AccumulatedJobValue = 0.0f;
			for (FJoyAssetManagerStartupJob& StartupJob : StartupJobs)
			{
				const float JobValue = StartupJob.JobWeight;
				StartupJob.SubstepProgressDelegate.BindLambda(
					[This = this, AccumulatedJobValue, JobValue, TotalJobValue](float NewProgress)
					{
						const float SubstepAdjustment = FMath::Clamp(NewProgress, 0.0f, 1.0f) * JobValue;
						const float OverallPercentWithSubstep =
							(AccumulatedJobValue + SubstepAdjustment) / TotalJobValue;

						This->UpdateInitialGameContentLoadPercent(OverallPercentWithSubstep);
					});

				StartupJob.DoJob();

				StartupJob.SubstepProgressDelegate.Unbind();

				AccumulatedJobValue += JobValue;

				UpdateInitialGameContentLoadPercent(AccumulatedJobValue / TotalJobValue);
			}
		}
		else
		{
			UpdateInitialGameContentLoadPercent(1.0f);
		}
	}

	StartupJobs.Empty();

	UE_LOG(LogJoy, Display, TEXT("All startup jobs took %.2f seconds to complete"),
		FPlatformTime::Seconds() - AllStartupJobsStartTime);
}

void UJoyAssetManager::UpdateInitialGameContentLoadPercent(float GameContentPercent)
{
	// Could route this to the early startup loading screen
}

#if WITH_EDITOR
void UJoyAssetManager::PreBeginPIE(bool bStartSimulate)
{
	Super::PreBeginPIE(bStartSimulate);

	{
		FScopedSlowTask SlowTask(0, NSLOCTEXT("JoyEditor", "BeginLoadingPIEData", "Loading PIE Data"));
		const bool bShowCancelButton = false;
		const bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);

		SCOPE_LOG_TIME_IN_SECONDS(TEXT("PreBeginPIE asset preloading complete"), nullptr);
	}
}
#endif
