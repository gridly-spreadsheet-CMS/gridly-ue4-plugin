// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "GridlyDataTable.h"
#include "GridlyResult.h"
#include "GridlyTableRow.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "GridlyTask_ImportDataTableFromGridly.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FImportDataTableFromGridlyDelegate, const TArray<FGridlyTableRow>&, GridlyTableRows,
	float, Progress, const FGridlyResult&, Error);

DECLARE_DELEGATE_OneParam(FImportDataTableFromGridlySuccessDelegate, const TArray<FGridlyTableRow>&);
DECLARE_DELEGATE_TwoParams(FImportDataTableFromGridlyProgressDelegate, const TArray<FGridlyTableRow>&, float);
DECLARE_DELEGATE_TwoParams(FImportDataTableFromGridlyFailDelegate, const TArray<FGridlyTableRow>&, const FGridlyResult&);

UCLASS()
class GRIDLY_API UGridlyTask_ImportDataTableFromGridly : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UGridlyTask_ImportDataTableFromGridly();

	virtual void Activate() override;

	void RequestPage(const int ViewIdIndex, const int Offset);
	void OnProcessRequestComplete(FHttpRequestPtr HttpRequestPtr, FHttpResponsePtr HttpResponsePtr, bool bSuccess);

public:
	UFUNCTION(Category = Gridly, BlueprintCallable, meta = (BlueprintInternalUseOnly = true, WorldContext = "WorldContextObject"))
	static UGridlyTask_ImportDataTableFromGridly* ImportDataTableFromGridly(const UObject* WorldContextObject,
		UGridlyDataTable* GridlyDataTable);

public:
	UPROPERTY(BlueprintAssignable)
	FImportDataTableFromGridlyDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FImportDataTableFromGridlyDelegate OnProgress;

	UPROPERTY(BlueprintAssignable)
	FImportDataTableFromGridlyDelegate OnFail;

	FImportDataTableFromGridlySuccessDelegate OnSuccessDelegate;
	FImportDataTableFromGridlyProgressDelegate OnProgressDelegate;
	FImportDataTableFromGridlyFailDelegate OnFailDelegate;;

private:
	FHttpRequestPtr HttpRequest;
	const UObject* WorldContextObject;

	int Limit;
	int TotalCount;

	TArray<FString> ViewIds;
	int CurrentViewIdIndex;
	int CurrentOffset;

	TArray<FGridlyTableRow> GridlyTableRows;

	UPROPERTY()
	UGridlyDataTable* GridlyDataTable;
};
