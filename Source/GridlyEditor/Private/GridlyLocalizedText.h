// Copyright © 2020 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "LocalizationTargetTypes.h"

class FGridlyLocalizedText
{
public:
	static bool GetSourceStringsAsPolyglotTextDatas(ULocalizationTarget* LocalizationTarget,
		TArray<FPolyglotTextData>& OutPolyglotTextDatas);
};
