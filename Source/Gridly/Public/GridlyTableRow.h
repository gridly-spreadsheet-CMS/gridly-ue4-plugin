// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "GridlyTableCell.h"

#include "GridlyTableRow.generated.h"

USTRUCT(BlueprintType)
struct GRIDLY_API FGridlyTableRow
{
	GENERATED_BODY()

	UPROPERTY(Category = Gridly, BlueprintReadOnly)
	FString Id;

	UPROPERTY(Category = Gridly, BlueprintReadOnly)
	FString Path;

	UPROPERTY(Category = Gridly, BlueprintReadOnly)
	TArray<FGridlyTableCell> Cells;
};
