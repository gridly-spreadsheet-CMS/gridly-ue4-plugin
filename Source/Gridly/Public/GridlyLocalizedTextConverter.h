﻿// Copyright © 2020 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "GridlyTableRow.h"

class GRIDLY_API FGridlyLocalizedTextConverter
{
public:
	static bool TableRowsToPolyglotTextDatas(const TArray<FGridlyTableRow>& TableRows,
		TMap<FString, FPolyglotTextData>& OutPolyglotTextDatas);
	static bool WritePoFile(const TArray<FPolyglotTextData>& PolyglotTextDatas, const FString& TargetCulture, const FString& Path);
	
	static bool ConvertToJson(const TArray<FPolyglotTextData>& PolyglotTextDatas, FString& OutJsonString);
};
