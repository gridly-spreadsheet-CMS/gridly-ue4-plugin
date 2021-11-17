// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "Engine/DataTable.h"

namespace GridlyDataTableJSONUtils
{
	FString GRIDLY_API GetKeyFieldName(const UDataTable& InDataTable);
}

class GRIDLY_API FGridlyDataTableImporterJSON
{
public:
	FGridlyDataTableImporterJSON(UDataTable& InDataTable, const FString& InJSONData, TArray<FString>& OutProblems);
	~FGridlyDataTableImporterJSON();

	bool ReadTable();

private:
	bool ReadRow(const TSharedRef<FJsonObject>& InParsedTableRowObject, const int32 InRowIdx);
	bool ReadStruct(const TSharedRef<FJsonObject>& InParsedObject, UScriptStruct* InStruct, const FName InRowName,
		void* InStructData);
	bool ReadStructEntry(const TSharedRef<FJsonValue>& InParsedPropertyValue, const FName InRowName, const FString& InColumnName,
		const void* InRowData, FProperty* InProperty, void* InPropertyData);
	bool ReadContainerEntry(const TSharedRef<FJsonValue>& InParsedPropertyValue, const FName InRowName, const FString& InColumnName,
		const int32 InArrayEntryIndex, FProperty* InProperty, void* InPropertyData);

	UDataTable* DataTable;
	const FString& JSONData;
	TArray<FString>& ImportProblems;
};
