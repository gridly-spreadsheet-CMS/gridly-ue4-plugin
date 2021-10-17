// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "GridlyResult.generated.h"

USTRUCT(BlueprintType)
struct GRIDLY_API FGridlyResult
{
	GENERATED_BODY()

	UPROPERTY(Category = Gridly, VisibleAnywhere, BlueprintReadWrite)
	FString Message;

	static const FGridlyResult Success;
};