// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "GridlyResult.h"
#include "Interfaces/IHttpRequest.h"
#include "Internationalization/PolyglotTextData.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "GridlyTask_DownloadLocalizedTexts.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDownloadLocalizedTextsDelegate, const TArray<FPolyglotTextData>&, PolyglotTextDatas,
	float, Progress, const FGridlyResult&, Error);

DECLARE_DELEGATE_OneParam(FDownloadLocalizedTextsSuccessDelegate, const TArray<FPolyglotTextData>&);
DECLARE_DELEGATE_TwoParams(FDownloadLocalizedTextsProgressDelegate, const TArray<FPolyglotTextData>&, float);
DECLARE_DELEGATE_TwoParams(FDownloadLocalizedTextsFailDelegate, const TArray<FPolyglotTextData>&, const FGridlyResult&);

UCLASS()
class GRIDLY_API UGridlyTask_DownloadLocalizedTexts : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UGridlyTask_DownloadLocalizedTexts();

	virtual void Activate() override;

	void RequestPage(const int ViewIdIndex, const int Offset);
	void OnProcessRequestComplete(FHttpRequestPtr HttpRequestPtr, FHttpResponsePtr HttpResponsePtr, bool bSuccess);

public:
	UFUNCTION(Category = Gridly, BlueprintCallable, meta = (BlueprintInternalUseOnly = true, WorldContext = "WorldContextObject"))
	static UGridlyTask_DownloadLocalizedTexts* DownloadLocalizedTexts(const UObject* WorldContextObject);

public:
	UPROPERTY(BlueprintAssignable)
	FDownloadLocalizedTextsDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FDownloadLocalizedTextsDelegate OnProgress;

	UPROPERTY(BlueprintAssignable)
	FDownloadLocalizedTextsDelegate OnFail;

	FDownloadLocalizedTextsSuccessDelegate OnSuccessDelegate;
	FDownloadLocalizedTextsProgressDelegate OnProgressDelegate;
	FDownloadLocalizedTextsFailDelegate OnFailDelegate;;

private:
	FHttpRequestPtr HttpRequest;
	const UObject* WorldContextObject;

	int Limit;
	int TotalCount;

	TArray<FString> ViewIds;
	int CurrentViewIdIndex;
	int CurrentOffset;

	TArray<FPolyglotTextData> PolyglotTextDatas;
};
