// kata.codes
#pragma once

#include "CoreMinimal.h"

class FSessionsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};