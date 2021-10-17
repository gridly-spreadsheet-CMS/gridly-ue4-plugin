// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

class GRIDLY_API FGridlyCultureConverter
{
public:
	static TArray<FString> GetTargetCultures();
	static bool ConvertFromGridly(const TArray<FString>& AvailableCultures, const FString& GridlyCulture,
		FString& OutCulture);
	static bool ConvertToGridly(const FString& Culture, FString& OutGridlyCulture);
};
