// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "Core/Public/Logging/LogMacros.h"
#include "Core/Public/Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGridly, Log, Log);

class FGridlyModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
