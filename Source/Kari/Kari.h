#pragma once
#include "EngineMinimal.h"
#include "CoreMinimal.h"

class FKariModule : public IModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};