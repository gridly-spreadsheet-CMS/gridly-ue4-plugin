// Copyright © 2020 LocalizeDirect AB

#include "GridlyTask_DownloadLocalizedTexts.h"

#include "Gridly.h"
#include "GridlyGameSettings.h"
#include "GridlyLocalizedTextConverter.h"
#include "GridlyTableRow.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpResponse.h"

UGridlyTask_DownloadLocalizedTexts::UGridlyTask_DownloadLocalizedTexts()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AddToRoot();
	}
}

void UGridlyTask_DownloadLocalizedTexts::Activate()
{
	UGridlyGameSettings* GameSettings = GetMutableDefault<UGridlyGameSettings>();

	Limit = 150; // TODO Change this default
	TotalCount = 0;

	ViewIds.Reset();
	ViewIds.Append(GameSettings->ImportFromViewIds);

	PolyglotTextDatas.Reset();

	RequestPage(0, 0);
}

void UGridlyTask_DownloadLocalizedTexts::RequestPage(const int ViewIdIndex, const int Offset)
{
	CurrentViewIdIndex = ViewIdIndex;
	CurrentOffset = Offset;

	if (ViewIdIndex < ViewIds.Num())
	{
		const FString& ViewId = ViewIds[ViewIdIndex];

		UGridlyGameSettings* GameSettings = GetMutableDefault<UGridlyGameSettings>();
		const FString ApiKey = GameSettings->ImportApiKey;

		const FString PaginationSettings = FGenericPlatformHttp::UrlEncode(FString::Printf(TEXT("{\"offset\":%d,\"limit\":%d}"),
			Offset,
			Limit));

		FStringFormatNamedArguments Args;
		Args.Add(TEXT("ViewId"), *ViewId);
		Args.Add(TEXT("PaginationSettings"), *PaginationSettings);
		const FString Url = FString::Format(TEXT("https://api.gridly.com/v1/views/{ViewId}/records?page={PaginationSettings}"),
			Args);

		HttpRequest = FHttpModule::Get().CreateRequest();
		HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("ApiKey %s"), *ApiKey));

		HttpRequest->SetVerb(TEXT("GET"));
		HttpRequest->SetURL(Url);

		HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGridlyTask_DownloadLocalizedTexts::OnProcessRequestComplete);

		OnProgress.Broadcast(PolyglotTextDatas, .1f, FGridlyResult::Success);
		OnProgressDelegate.ExecuteIfBound(PolyglotTextDatas, .1f);

		// Throttles number of requests by sleeping between each

		UWorld* World = WorldContextObject != nullptr ? WorldContextObject->GetWorld() : nullptr;
		if (World)
		{
			FTimerHandle TimerHandle;
			World->GetTimerManager().SetTimer(TimerHandle, [this, ViewId, Offset]()
			{
				HttpRequest->ProcessRequest();
				UE_LOG(LogGridly, Log, TEXT("Requesting ViewId: %s, with offset: %d, limit: %d"), *ViewId, Offset, Limit);
			}, 1.f, false);
		}
		else
		{
			HttpRequest->ProcessRequest();
			UE_LOG(LogGridly, Log, TEXT("Requesting ViewId: %s, with offset: %d, limit: %d"), *ViewId, Offset, Limit);
			FPlatformProcess::Sleep(1.f);
		}
	}
	else
	{
		OnSuccess.Broadcast(PolyglotTextDatas, 1.f, FGridlyResult::Success);
		OnSuccessDelegate.ExecuteIfBound(PolyglotTextDatas);
	}
}

void UGridlyTask_DownloadLocalizedTexts::OnProcessRequestComplete(FHttpRequestPtr HttpRequestPtr,
	FHttpResponsePtr HttpResponsePtr, bool bSuccess)
{
	if (bSuccess)
	{
		// Header

		TArray<FString> Headers = HttpResponsePtr->GetAllHeaders();
		for (int i = 0; i < Headers.Num(); i++)
		{
			UE_LOG(LogGridly, Verbose, TEXT("%s"), *Headers[i]);
		}

		// Convert from JSON to texts

		const FString Content = HttpResponsePtr->GetContentAsString();
		UE_LOG(LogGridly, Verbose, TEXT("%s"), *Content);

		TMap<FString, FPolyglotTextData> PolyglotTextDataMap;
		TArray<FGridlyTableRow> TableRows;

		if (FJsonObjectConverter::JsonArrayStringToUStruct(Content, &TableRows, 0, 0)
		    && FGridlyLocalizedTextConverter::TableRowsToPolyglotTextDatas(TableRows, PolyglotTextDataMap))
		{
			TArray<FPolyglotTextData> CurrentPolyglotTextDatas;
			PolyglotTextDataMap.GenerateValueArray(CurrentPolyglotTextDatas);
			PolyglotTextDatas.Append(CurrentPolyglotTextDatas);

			const int ViewIdTotalCount = FCString::Atoi(*HttpResponsePtr->GetHeader("X-Total-Count"));
			TotalCount += CurrentOffset == 0 ? ViewIdTotalCount : 0;
			const float EstimatedProgressViewIds = static_cast<float>(CurrentViewIdIndex) / FMath::Max(1, ViewIds.Num());
			const float EstimatedProgressPagination = PolyglotTextDatas.Num() / TotalCount;
			const float EstimatedProgress = (EstimatedProgressViewIds + EstimatedProgressPagination) / 2.f;

			OnProgress.Broadcast(PolyglotTextDatas, EstimatedProgress, FGridlyResult::Success);
			OnProgressDelegate.ExecuteIfBound(PolyglotTextDatas, EstimatedProgress);

			if (PolyglotTextDatas.Num() < TotalCount)
			{
				RequestPage(CurrentViewIdIndex, CurrentOffset + Limit);
			}
			else
			{
				RequestPage(CurrentViewIdIndex + 1, 0);
			}
		}
		else
		{
			const FGridlyResult FailResult = FGridlyResult{"Failed to parse downloaded content"};
			OnFail.Broadcast(PolyglotTextDatas, 1.f, FailResult);
			OnFailDelegate.ExecuteIfBound(PolyglotTextDatas, FailResult);
		}
	}
	else
	{
		const FGridlyResult FailResult = FGridlyResult{"Failed to connect to gridly"};
		OnFail.Broadcast(PolyglotTextDatas, 1.f, FailResult);
		OnFailDelegate.ExecuteIfBound(PolyglotTextDatas, FailResult);
	}
}

UGridlyTask_DownloadLocalizedTexts* UGridlyTask_DownloadLocalizedTexts::DownloadLocalizedTexts(const UObject* WorldContextObject)
{
	auto DownloadLocalizedTexts = NewObject<UGridlyTask_DownloadLocalizedTexts>();
	DownloadLocalizedTexts->WorldContextObject = WorldContextObject;
	return DownloadLocalizedTexts;
}
