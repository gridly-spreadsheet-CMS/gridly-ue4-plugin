// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "GridlyDataTable.h"

class FLocTextHelper;

class FGridlyExporter
{
public:
	static bool ConvertToJson(const TArray<FPolyglotTextData>& PolyglotTextDatas, bool bIncludeTargetTranslations,
		const TSharedPtr<FLocTextHelper>& LocTextHelperPtr, FString& OutJsonString);
	static bool ConvertToJson(const UGridlyDataTable* GridlyDataTable, FString& OutJsonString, size_t StartIndex, size_t MaxSize);
};
