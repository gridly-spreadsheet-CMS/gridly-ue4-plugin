// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "GridlyTableCell.generated.h"

USTRUCT(BlueprintType)
struct FGridlyTableCell
{
	GENERATED_BODY()

	UPROPERTY(Category = Gridly, BlueprintReadOnly)
	FString ColumnId;

	UPROPERTY(Category = Gridly, BlueprintReadOnly)
	FString DependencyStatus;

	UPROPERTY(Category = Gridly, BlueprintReadOnly)
	FString Value;
};
