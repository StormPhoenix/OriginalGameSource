#pragma once
#include "JsEnv.h"
#include "JoyTypeScriptGameInstanceSystem.generated.h"


UCLASS()
class UJoyTypeScriptGameInstanceSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	static UJoyTypeScriptGameInstanceSystem* Get(const UWorld* World);

	void Start();

private:
	TSharedPtr<puerts::FJsEnv> JsEnv;
};
