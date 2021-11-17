// Copyright (c) 2021 LocalizeDirect AB

#include "GridlyTask_ImportDataTableFromGridly.h"

#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GridlyDataTableImporterJSON.h"
#include "Gridly.h"
#include "GridlyGameSettings.h"
#include "GridlyTableRow.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpResponse.h"

UGridlyTask_ImportDataTableFromGridly::UGridlyTask_ImportDataTableFromGridly()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AddToRoot();
	}
}

void UGridlyTask_ImportDataTableFromGridly::Activate()
{
	const UGridlyGameSettings* GameSettings = GetMutableDefault<UGridlyGameSettings>();

	Limit = GameSettings->ImportMaxRecordsPerRequest;
	TotalCount = 0;

	ViewIds.Reset();
	if (GridlyDataTable && !GridlyDataTable->ViewId.IsEmpty())
	{
		ViewIds.Add(GridlyDataTable->ViewId);
	}

	GridlyTableRows.Reset();

	RequestPage(0, 0);
}

void UGridlyTask_ImportDataTableFromGridly::RequestPage(const int ViewIdIndex, const int Offset)
{
	CurrentViewIdIndex = ViewIdIndex;
	CurrentOffset = Offset;

	if (ViewIds.Num() == 0)
	{
		const FGridlyResult FailResult = FGridlyResult{"Unable to import data table: no view IDs were specified"};
		UE_LOG(LogGridly, Error, TEXT("%s"), *FailResult.Message);
		OnFail.Broadcast(GridlyTableRows, 1.f, FailResult);
		if (OnFailDelegate.IsBound())
			OnFailDelegate.Execute(GridlyTableRows, FailResult);
		return;
	}

	if (ViewIdIndex < ViewIds.Num())
	{
		const FString& ViewId = ViewIds[ViewIdIndex];

		const UGridlyGameSettings* GameSettings = GetMutableDefault<UGridlyGameSettings>();
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

		HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGridlyTask_ImportDataTableFromGridly::OnProcessRequestComplete);

		OnProgress.Broadcast(GridlyTableRows, .1f, FGridlyResult::Success);
		if (OnProgressDelegate.IsBound())
			OnProgressDelegate.Execute(GridlyTableRows, .1f);

		// Throttles number of requests by sleeping between each

		UWorld* World = WorldContextObject != nullptr ? WorldContextObject->GetWorld() : nullptr;
		if (World)
		{
			FTimerHandle TimerHandle;
			World->GetTimerManager().SetTimer(TimerHandle, [this, ViewId, Offset]()
			{
				HttpRequest->ProcessRequest();
				UE_LOG(LogGridly, Log, TEXT("Requesting view ID: %s, with offset: %d, limit: %d"), *ViewId, Offset, Limit);
			}, 1.f, false);
		}
		else
		{
			HttpRequest->ProcessRequest();
			UE_LOG(LogGridly, Log, TEXT("Requesting view ID: %s, with offset: %d, limit: %d"), *ViewId, Offset, Limit);
			FPlatformProcess::Sleep(1.f);
		}
	}
	else
	{
		TArray<TSharedPtr<FJsonValue>> JsonValues;

		for (int i = 0; i < GridlyTableRows.Num(); i++)
		{
			const TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
			JsonObject->SetStringField("name", GridlyTableRows[i].Id);

			for (int j = 0; j < GridlyTableRows[i].Cells.Num(); j++)
			{
				JsonObject->SetStringField(GridlyTableRows[i].Cells[j].ColumnId, GridlyTableRows[i].Cells[j].Value);
			}

			JsonValues.Add(MakeShareable(new FJsonValueObject(JsonObject)));
		}

		GridlyDataTable->EmptyTable();

		FString JsonString;
		const TSharedRef<TJsonWriter<>> JsonWriter = TJsonStringWriter<>::Create(&JsonString);
		FJsonSerializer::Serialize(JsonValues, JsonWriter);

		TArray<FString> OutProblems;
		if (FGridlyDataTableImporterJSON(*GridlyDataTable, JsonString, OutProblems).ReadTable())
		{
			UE_LOG(LogGridly, Log, TEXT("Imported data table from Gridly: %s"), *GridlyDataTable->GetName());
			OnSuccess.Broadcast(GridlyTableRows, 1.f, FGridlyResult::Success);
			if (OnSuccessDelegate.IsBound())
				OnSuccessDelegate.Execute(GridlyTableRows);
		}
		else
		{
			for (int i = 0; i < OutProblems.Num(); i++)
			{
				UE_LOG(LogGridly, Error, TEXT("%s"), *OutProblems[i]);
			}

			const FGridlyResult FailResult = FGridlyResult{"Failed to parse downloaded content"};
			OnFail.Broadcast(GridlyTableRows, 1.f, FailResult);
			if (OnFailDelegate.IsBound())
				OnFailDelegate.Execute(GridlyTableRows, FailResult);
		}
	}
}

void UGridlyTask_ImportDataTableFromGridly::OnProcessRequestComplete(FHttpRequestPtr HttpRequestPtr,
	FHttpResponsePtr HttpResponsePtr, bool bSuccess)
{
	if (bSuccess && HttpResponsePtr->GetResponseCode() == EHttpResponseCodes::Ok)
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

		TArray<FGridlyTableRow> TableRows;

		if (FJsonObjectConverter::JsonArrayStringToUStruct(Content, &TableRows, 0, 0))
		{
			GridlyTableRows.Append(TableRows);

			const int ViewIdTotalCount = FCString::Atoi(*HttpResponsePtr->GetHeader("X-Total-Count"));
			TotalCount += CurrentOffset == 0 ? ViewIdTotalCount : 0;
			const float EstimatedProgressViewIds =
				static_cast<float>(CurrentViewIdIndex) / static_cast<float>(FMath::Max(1, ViewIds.Num()));
			const float EstimatedProgressPagination = static_cast<float>(GridlyTableRows.Num()) / static_cast<float>(TotalCount);
			const float EstimatedProgress = (EstimatedProgressViewIds + EstimatedProgressPagination) / 2.f;

			OnProgress.Broadcast(GridlyTableRows, EstimatedProgress, FGridlyResult::Success);
			if (OnProgressDelegate.IsBound())
				OnProgressDelegate.Execute(GridlyTableRows, EstimatedProgress);

			if ((CurrentOffset + Limit) < TotalCount)
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
			OnFail.Broadcast(GridlyTableRows, 1.f, FailResult);
			if (OnFailDelegate.IsBound())
				OnFailDelegate.Execute(GridlyTableRows, FailResult);
		}
	}
	else
	{
		const FGridlyResult FailResult = FGridlyResult{"Failed to connect to Gridly"};
		OnFail.Broadcast(GridlyTableRows, 1.f, FailResult);
		if (OnFailDelegate.IsBound())
			OnFailDelegate.Execute(GridlyTableRows, FailResult);
	}
}

UGridlyTask_ImportDataTableFromGridly* UGridlyTask_ImportDataTableFromGridly::ImportDataTableFromGridly(
	const UObject* WorldContextObject, UGridlyDataTable* GridlyDataTable)
{
	UGridlyTask_ImportDataTableFromGridly* ImportDataTableFromGridly = NewObject<UGridlyTask_ImportDataTableFromGridly>();
	ImportDataTableFromGridly->WorldContextObject = WorldContextObject;
	ImportDataTableFromGridly->GridlyDataTable = GridlyDataTable;
	return ImportDataTableFromGridly;
}
