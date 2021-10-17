// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "Factories/DataTableFactory.h"

#include "GridlyDataTableFactory.generated.h"

UCLASS(HideCategories = Object)
class UGridlyDataTableFactory : public UDataTableFactory
{
	GENERATED_UCLASS_BODY()

protected:
	virtual UDataTable* MakeNewDataTable(UObject* InParent, FName Name, EObjectFlags Flags) override;
};
