// Copyright Epic Games, Inc. All Rights Reserved.

#include "JoyAssetManagerStartupJob.h"

#include "JoyLogChannels.h"

TSharedPtr<FStreamableHandle> FJoyAssetManagerStartupJob::DoJob() const
{
	const double JobStartTime = FPlatformTime::Seconds();

	TSharedPtr<FStreamableHandle> Handle;
	UE_LOG(LogJoy, Display, TEXT("Startup job \"%s\" starting"), *JobName);
	JobFunc(*this, Handle);

	if (Handle.IsValid())
	{
		Handle->BindUpdateDelegate(FStreamableUpdateDelegate::CreateRaw(
			this, &FJoyAssetManagerStartupJob::UpdateSubstepProgressFromStreamable));
		Handle->WaitUntilComplete(0.0f, false);
		Handle->BindUpdateDelegate(FStreamableUpdateDelegate());
	}

	UE_LOG(LogJoy, Display, TEXT("Startup job \"%s\" took %.2f seconds to complete"), *JobName,
		FPlatformTime::Seconds() - JobStartTime);

	return Handle;
}
