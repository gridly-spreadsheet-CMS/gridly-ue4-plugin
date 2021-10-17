// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "Engine/DataTable.h"

#include "GridlyDataTable.generated.h"

/**
 * Data table that can sync with Gridly
 */
UCLASS(BlueprintType, HideCategories = Object)
class GRIDLY_API UGridlyDataTable : public UDataTable
{
	friend class FGridlyDataTableImporterJSON;
	
	GENERATED_BODY()

public:
	UPROPERTY(Category = Gridly, EditDefaultsOnly)
	FString ViewId;
};
