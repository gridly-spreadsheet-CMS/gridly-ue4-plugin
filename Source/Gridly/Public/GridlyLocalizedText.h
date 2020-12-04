// Copyright © 2020 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "GridlyLocalizedText.generated.h"

USTRUCT(BlueprintType)
struct GRIDLY_API FGridlyLocalizedText
{
	GENERATED_BODY()

	UPROPERTY()
	FString Key;

	UPROPERTY()
	FString Namespace;

	UPROPERTY()
	FString SourceCulture;

	UPROPERTY()
	FString SourceText;

	UPROPERTY()
	FString TargetCulture;

	UPROPERTY()
	FString TargetText;
};
