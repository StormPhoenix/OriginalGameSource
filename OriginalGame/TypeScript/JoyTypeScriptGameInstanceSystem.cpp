#include "JoyTypeScriptGameInstanceSystem.h"

#include "PuertsModule.h"
#include "PuertsModule.h"

void UJoyTypeScriptGameInstanceSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UJoyTypeScriptGameInstanceSystem::Deinitialize()
{
	JsEnv.Reset();
	Super::Deinitialize();
}

UJoyTypeScriptGameInstanceSystem* UJoyTypeScriptGameInstanceSystem::Get(const UWorld* World)
{
	if (World)
	{
		return UGameInstance::GetSubsystem<UJoyTypeScriptGameInstanceSystem>(World->GetGameInstance());
	}

	return nullptr;
}

void UJoyTypeScriptGameInstanceSystem::Start()
{
	auto& Module = IPuertsModule::Get();
	Module.MakeSharedJsEnv();

	JsEnv = Module.GetJsEnv();

	TArray<TPair<FString, UObject*>> Arguments;

	Arguments.Add({TEXT("World"), GetWorld()});
	Arguments.Add({TEXT("GameInstance"), GetGameInstance()});

	JsEnv->Start(TEXT("Launch/EngineInit"), Arguments);
}
