// Copyright © 2020 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "GridlyTableCell.h"

#include "GridlyTableRow.generated.h"

USTRUCT()
struct GRIDLY_API FGridlyTableRow
{
	GENERATED_BODY()

	UPROPERTY()
	FString Id;

	UPROPERTY()
	FString Path;

	UPROPERTY()
	TArray<FGridlyTableCell> Cells;
};
