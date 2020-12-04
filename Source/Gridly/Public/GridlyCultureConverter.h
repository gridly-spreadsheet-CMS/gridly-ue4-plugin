// Copyright © 2020 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

class GRIDLY_API FGridlyCultureConverter
{
public:
	static TArray<FString> GetTargetCultures();
	static bool ConvertFromGridlyCulture(const TArray<FString>& AvailableCultures, const FString& GridlyCulture, FString& OutCulture);
	static bool ConvertToGridlyCulture(const FString& Culture, FString& OutGridlyCulture);
};
