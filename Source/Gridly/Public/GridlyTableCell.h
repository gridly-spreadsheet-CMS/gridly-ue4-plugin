// Copyright © 2020 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "GridlyTableCell.generated.h"

USTRUCT()
struct FGridlyTableCell
{
	GENERATED_BODY()

	UPROPERTY()
	FString ColumnId;

	UPROPERTY()
	FString DependencyStatus;

	UPROPERTY()
	FString Value;
};
