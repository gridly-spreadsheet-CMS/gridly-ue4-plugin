// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"

#include "ILocalizationServiceOperation.h"
#include "ILocalizationServiceProvider.h"
#include "ILocalizationServiceState.h"
#include "Interfaces/IHttpRequest.h"

class FGridlyLocalizationServiceProvider final : public ILocalizationServiceProvider
{
public:
	FGridlyLocalizationServiceProvider();

public:
	/* ILocalizationServiceProvider implementation */

	virtual void Init(bool bForceConnection = true) override;
	virtual void Close() override;
	virtual FText GetStatusText() const override;
	virtual bool IsEnabled() const override;
	virtual bool IsAvailable() const override;
	virtual const FName& GetName(void) const override;
	virtual const FText GetDisplayName() const override;
	virtual ELocalizationServiceOperationCommandResult::Type GetState(
		const TArray<FLocalizationServiceTranslationIdentifier>& InTranslationIds,
		TArray<TSharedRef<ILocalizationServiceState, ESPMode::ThreadSafe>>& OutState,
		ELocalizationServiceCacheUsage::Type InStateCacheUsage) override;
	virtual ELocalizationServiceOperationCommandResult::Type Execute(
		const TSharedRef<ILocalizationServiceOperation, ESPMode::ThreadSafe>& InOperation,
		const TArray<FLocalizationServiceTranslationIdentifier>& InTranslationIds,
		ELocalizationServiceOperationConcurrency::Type InConcurrency = ELocalizationServiceOperationConcurrency::Synchronous,
		const FLocalizationServiceOperationComplete& InOperationCompleteDelegate =
			FLocalizationServiceOperationComplete()) override;
	virtual bool CanCancelOperation(
		const TSharedRef<ILocalizationServiceOperation, ESPMode::ThreadSafe>& InOperation) const override;
	virtual void CancelOperation(const TSharedRef<ILocalizationServiceOperation, ESPMode::ThreadSafe>& InOperation) override;
	virtual void Tick() override;

#if LOCALIZATION_SERVICES_WITH_SLATE
	virtual void CustomizeSettingsDetails(IDetailCategoryBuilder& DetailCategoryBuilder) const override;
	virtual void CustomizeTargetDetails(
		IDetailCategoryBuilder& DetailCategoryBuilder, TWeakObjectPtr<ULocalizationTarget> LocalizationTarget) const override;
	virtual void CustomizeTargetToolbar(
		TSharedRef<FExtender>& MenuExtender, TWeakObjectPtr<ULocalizationTarget> LocalizationTarget) const override;
	virtual void CustomizeTargetSetToolbar(
		TSharedRef<FExtender>& MenuExtender, TWeakObjectPtr<ULocalizationTargetSet> LocalizationTargetSet) const override;
	void AddTargetToolbarButtons(FToolBarBuilder& ToolbarBuilder, TWeakObjectPtr<ULocalizationTarget> LocalizationTarget,
		TSharedRef<FUICommandList> CommandList);
#endif	  // LOCALIZATION_SERVICES_WITH_SLATE

	// functions to run export/import from commandlet
	FHttpRequestCompleteDelegate CreateExportNativeCultureDelegate();
	bool HasRequestsPending() const;

	void ExportForTargetToGridly(ULocalizationTarget* LocalizationTarget, FHttpRequestCompleteDelegate& ReqDelegate, const FText& SlowTaskText, bool bIncTargetTranslation = false);
	
private:
	// Import

	void ImportAllCulturesForTargetFromGridly(TWeakObjectPtr<ULocalizationTarget> LocalizationTarget, bool bIsTargetSet);
	void OnImportCultureForTargetFromGridly(const FLocalizationServiceOperationRef& Operation,
		ELocalizationServiceOperationCommandResult::Type Result, bool bIsTargetSet);
	TSharedPtr<FScopedSlowTask> ImportAllCulturesForTargetFromGridlySlowTask;
	TArray<FString> CurrentCultureDownloads;
	int SuccessfulDownloads;

	// Export

	size_t ExportForTargetEntriesUpdated;
	TSharedPtr<FScopedSlowTask> ExportForTargetToGridlySlowTask;
	TQueue<TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> ExportFromTargetRequestQueue;
	bool bExportRequestInProgress = false;

	void ExportNativeCultureForTargetToGridly(TWeakObjectPtr<ULocalizationTarget> LocalizationTarget, bool bIsTargetSet);
	void OnExportNativeCultureForTargetToGridly(FHttpRequestPtr HttpRequestPtr, FHttpResponsePtr HttpResponsePtr, bool bSuccess);

	// Export all

	void ExportTranslationsForTargetToGridly(TWeakObjectPtr<ULocalizationTarget> LocalizationTarget, bool bIsTargetSet);
	void OnExportTranslationsForTargetToGridly(FHttpRequestPtr HttpRequestPtr, FHttpResponsePtr HttpResponsePtr, bool bSuccess);
};
