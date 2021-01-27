// Copyright © 2020 LocalizeDirect AB

#pragma once

#include "GridlyDataTable.h"

class FGridlyExporter
{
public:
	static bool ConvertToJson(const TArray<FPolyglotTextData>& PolyglotTextDatas, bool bIncludeTargetTranslations,
		FString& OutJsonString);
	static bool ConvertToJson(const UGridlyDataTable* GridlyDataTable, FString& OutJsonString);
};
