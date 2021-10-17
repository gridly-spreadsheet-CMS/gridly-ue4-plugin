// Copyright (c) 2021 LocalizeDirect AB

#include "GridlyDataTableFactory.h"

#include "GridlyDataTable.h"

UGridlyDataTableFactory::UGridlyDataTableFactory(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	SupportedClass = UGridlyDataTable::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UDataTable* UGridlyDataTableFactory::MakeNewDataTable(UObject* InParent, FName Name, EObjectFlags Flags)
{
	return NewObject<UGridlyDataTable>(InParent, Name, Flags);
}
