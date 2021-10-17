// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "LocalizationTargetTypes.h"

class FLocTextHelper;
class FGridlyLocalizedText
{
public:
	static bool GetAllTextAsPolyglotTextDatas(ULocalizationTarget* LocalizationTarget,
		TArray<FPolyglotTextData>& OutPolyglotTextDatas, TSharedPtr<FLocTextHelper>& LocTextHelper);
};
