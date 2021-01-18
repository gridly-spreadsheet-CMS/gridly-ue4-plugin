// Copyright © 2020 LocalizeDirect AB

#include "GridlyLocalizationServiceProvider.h"

#include "GridlyEditor.h"
#include "GridlyGameSettings.h"
#include "GridlyLocalizedText.h"
#include "GridlyLocalizedTextConverter.h"
#include "GridlyTask_DownloadLocalizedTexts.h"
#include "HttpModule.h"
#include "ILocalizationServiceModule.h"
#include "LocalizationCommandletTasks.h"
#include "LocalizationModule.h"
#include "LocalizationTargetTypes.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IMainFrameModule.h"
#include "Internationalization/Culture.h"
#include "Misc/FeedbackContext.h"
#include "Misc/ScopedSlowTask.h"
#include "Serialization/JsonSerializer.h"

#if LOCALIZATION_SERVICES_WITH_SLATE
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#endif

#define LOCTEXT_NAMESPACE "Gridly"

static FName ProviderName("Gridly");

class FGridlyLocalizationTargetEditorCommands final : public TCommands<FGridlyLocalizationTargetEditorCommands>
{
public:
	FGridlyLocalizationTargetEditorCommands() :
		TCommands<FGridlyLocalizationTargetEditorCommands>("GridlyLocalizationTargetEditor",
			NSLOCTEXT("Gridly", "GridlyLocalizationTargetEditor", "Gridly Localization Target Editor"), NAME_None,
			FEditorStyle::GetStyleSetName())
	{
	}

	TSharedPtr<FUICommandInfo> ImportAllCulturesForTargetFromGridly;
	TSharedPtr<FUICommandInfo> ExportNativeCultureForTargetToGridly;

	/** Initialize commands */
	virtual void RegisterCommands() override;
};

void FGridlyLocalizationTargetEditorCommands::RegisterCommands()
{
	UI_COMMAND(ImportAllCulturesForTargetFromGridly, "Import from Gridly",
		"Imports translations for all cultures of this target to Gridly.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ExportNativeCultureForTargetToGridly, "Export to Gridly",
		"Exports native culture and source text of this target to Gridly.", EUserInterfaceActionType::Button, FInputChord());
}

FGridlyLocalizationServiceProvider::FGridlyLocalizationServiceProvider()
{
}

void FGridlyLocalizationServiceProvider::Init(bool bForceConnection)
{
	FGridlyLocalizationTargetEditorCommands::Register();
}

void FGridlyLocalizationServiceProvider::Close()
{
}

FText FGridlyLocalizationServiceProvider::GetStatusText() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Status"), LOCTEXT("Unknown", "Unknown / not implemented"));

	return FText::Format(LOCTEXT("GridlyStatusText", "Gridly status: {Status}"), Args);
}

bool FGridlyLocalizationServiceProvider::IsEnabled() const
{
	return true;
}

bool FGridlyLocalizationServiceProvider::IsAvailable() const
{
	return true; // Check for server availability
}

const FName& FGridlyLocalizationServiceProvider::GetName(void) const
{
	return ProviderName;
}

const FText FGridlyLocalizationServiceProvider::GetDisplayName() const
{
	return LOCTEXT("GridlyLocalizationService", "Gridly Localization Service");
}

ELocalizationServiceOperationCommandResult::Type FGridlyLocalizationServiceProvider::GetState(
	const TArray<FLocalizationServiceTranslationIdentifier>& InTranslationIds,
	TArray<TSharedRef<ILocalizationServiceState, ESPMode::ThreadSafe>>& OutState,
	ELocalizationServiceCacheUsage::Type InStateCacheUsage)
{
	return ELocalizationServiceOperationCommandResult::Succeeded;
}

ELocalizationServiceOperationCommandResult::Type FGridlyLocalizationServiceProvider::Execute(
	const TSharedRef<ILocalizationServiceOperation, ESPMode::ThreadSafe>& InOperation,
	const TArray<FLocalizationServiceTranslationIdentifier>& InTranslationIds,
	ELocalizationServiceOperationConcurrency::Type InConcurrency /*= ELocalizationServiceOperationConcurrency::Synchronous*/,
	const FLocalizationServiceOperationComplete& InOperationCompleteDelegate /*= FLocalizationServiceOperationComplete()*/)
{
	const TSharedRef<FDownloadLocalizationTargetFile, ESPMode::ThreadSafe> DownloadOperation =
		StaticCastSharedRef<FDownloadLocalizationTargetFile>(InOperation);
	const FString TargetCulture = DownloadOperation->GetInLocale();

	UGridlyTask_DownloadLocalizedTexts* Task = UGridlyTask_DownloadLocalizedTexts::DownloadLocalizedTexts(nullptr);

	// On success

	Task->OnSuccessDelegate.BindLambda(
		[DownloadOperation, InOperationCompleteDelegate, TargetCulture](const TArray<FPolyglotTextData>& PolyglotTextDatas)
		{
			if (PolyglotTextDatas.Num() > 0)
			{
				const FString AbsoluteFilePathAndName = FPaths::ConvertRelativePathToFull(
					FPaths::ProjectDir() / DownloadOperation->GetInRelativeOutputFilePathAndName());

				FGridlyLocalizedTextConverter::WritePoFile(PolyglotTextDatas, TargetCulture, AbsoluteFilePathAndName);

				// Callback

				InOperationCompleteDelegate.Execute(DownloadOperation, ELocalizationServiceOperationCommandResult::Succeeded);
			}
			else
			{
				DownloadOperation->SetOutErrorText(LOCTEXT("GridlyErrorParse", "Failed to parse downloaded content"));
				InOperationCompleteDelegate.Execute(DownloadOperation, ELocalizationServiceOperationCommandResult::Failed);
			}
		});

	// On fail

	Task->OnFailDelegate.BindLambda(
		[DownloadOperation, InOperationCompleteDelegate](const TArray<FPolyglotTextData>& PolyglotTextDatas,
		const FGridlyResult& Error)
		{
			DownloadOperation->SetOutErrorText(FText::FromString(Error.Message));
			InOperationCompleteDelegate.Execute(DownloadOperation, ELocalizationServiceOperationCommandResult::Failed);
		});

	Task->Activate();

	return ELocalizationServiceOperationCommandResult::Succeeded;
}

bool FGridlyLocalizationServiceProvider::CanCancelOperation(
	const TSharedRef<ILocalizationServiceOperation, ESPMode::ThreadSafe>& InOperation) const
{
	return false;
}

void FGridlyLocalizationServiceProvider::CancelOperation(
	const TSharedRef<ILocalizationServiceOperation, ESPMode::ThreadSafe>& InOperation)
{
}

void FGridlyLocalizationServiceProvider::Tick()
{
}

#if LOCALIZATION_SERVICES_WITH_SLATE
void FGridlyLocalizationServiceProvider::CustomizeSettingsDetails(IDetailCategoryBuilder& DetailCategoryBuilder) const
{
	const FText GridlySettingsInfoText = LOCTEXT("GridlySettingsInfo", "Use Project Settings to configure Gridly");
	FDetailWidgetRow& PublicKeyRow = DetailCategoryBuilder.AddCustomRow(GridlySettingsInfoText);
	PublicKeyRow.ValueContent()[SNew(STextBlock).Text(GridlySettingsInfoText)];
	PublicKeyRow.ValueContent().HAlign(EHorizontalAlignment::HAlign_Fill);
}

void FGridlyLocalizationServiceProvider::CustomizeTargetDetails(
	IDetailCategoryBuilder& DetailCategoryBuilder, TWeakObjectPtr<ULocalizationTarget> LocalizationTarget) const
{
	// Not implemented
}

void FGridlyLocalizationServiceProvider::CustomizeTargetToolbar(
	TSharedRef<FExtender>& MenuExtender, TWeakObjectPtr<ULocalizationTarget> LocalizationTarget) const
{
	const TSharedRef<FUICommandList> CommandList = MakeShareable(new FUICommandList());

	MenuExtender->AddToolBarExtension("LocalizationService", EExtensionHook::First, CommandList,
		FToolBarExtensionDelegate::CreateRaw(const_cast<FGridlyLocalizationServiceProvider*>(this),
			&FGridlyLocalizationServiceProvider::AddTargetToolbarButtons, LocalizationTarget, CommandList));
}

void FGridlyLocalizationServiceProvider::CustomizeTargetSetToolbar(
	TSharedRef<FExtender>& MenuExtender, TWeakObjectPtr<ULocalizationTargetSet> LocalizationTargetSet) const
{
	// Not implemented
}

void FGridlyLocalizationServiceProvider::AddTargetToolbarButtons(FToolBarBuilder& ToolbarBuilder,
	TWeakObjectPtr<ULocalizationTarget> LocalizationTarget, TSharedRef<FUICommandList> CommandList)
{
	// Don't add toolbar buttons if target is engine

	if (!LocalizationTarget->IsMemberOfEngineTargetSet())
	{
		const bool bIsTargetSet = false;
		CommandList->MapAction(FGridlyLocalizationTargetEditorCommands::Get().ImportAllCulturesForTargetFromGridly,
			FExecuteAction::CreateRaw(this, &FGridlyLocalizationServiceProvider::ImportAllCulturesForTargetFromGridly,
				LocalizationTarget, bIsTargetSet));
		ToolbarBuilder.AddToolBarButton(FGridlyLocalizationTargetEditorCommands::Get().ImportAllCulturesForTargetFromGridly,
			NAME_None,
			TAttribute<FText>(), TAttribute<FText>(),
			FSlateIcon(FEditorStyle::GetStyleSetName(), "TranslationEditor.ImportLatestFromLocalizationService"));

		CommandList->MapAction(FGridlyLocalizationTargetEditorCommands::Get().ExportNativeCultureForTargetToGridly,
			FExecuteAction::CreateRaw(this, &FGridlyLocalizationServiceProvider::ExportNativeCultureForTargetToGridly,
				LocalizationTarget, bIsTargetSet));
		ToolbarBuilder.AddToolBarButton(
			FGridlyLocalizationTargetEditorCommands::Get().ExportNativeCultureForTargetToGridly, NAME_None,
			TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FEditorStyle::GetStyleSetName(),
				"TranslationEditor.ImportLatestFromLocalizationService"));
	}
}
#endif	  // LOCALIZATION_SERVICES_WITH_SLATE

void FGridlyLocalizationServiceProvider::ImportAllCulturesForTargetFromGridly(
	TWeakObjectPtr<ULocalizationTarget> LocalizationTarget, bool bIsTargetSet)
{
	check(LocalizationTarget.IsValid());

	const EAppReturnType::Type MessageReturn = FMessageDialog::Open(EAppMsgType::YesNo,
		LOCTEXT("ConfirmText",
			"All local translations to non-native languages will be overwritten. Are you sure you wish to update?"));

	if (!bIsTargetSet && MessageReturn == EAppReturnType::Yes)
	{
		for (int i = 0; i < LocalizationTarget->Settings.SupportedCulturesStatistics.Num(); i++)
		{
			if (i != LocalizationTarget->Settings.NativeCultureIndex)
			{
				const FCultureStatistics CultureStats = LocalizationTarget->Settings.SupportedCulturesStatistics[i];
				CurrentCultureDownloads.Add(CultureStats.CultureName);
			}
		}

		const float AmountOfWork = CurrentCultureDownloads.Num();
		ImportAllCulturesForTargetFromGridlySlowTask = MakeShareable(new FScopedSlowTask(AmountOfWork,
			LOCTEXT("ImportAllCulturesForTargetFromGridlyText", "Importing all cultures for target from Gridly")));

		ImportAllCulturesForTargetFromGridlySlowTask->MakeDialog();

		for (const FString& CultureName : CurrentCultureDownloads)
		{
			UE_LOG(LogGridlyEditor, Log, TEXT("Culture: %s"), *CultureName);

			ILocalizationServiceProvider& Provider = ILocalizationServiceModule::Get().GetProvider();
			TSharedRef<FDownloadLocalizationTargetFile, ESPMode::ThreadSafe> DownloadTargetFileOp =
				ILocalizationServiceOperation::Create<FDownloadLocalizationTargetFile>();
			DownloadTargetFileOp->SetInTargetGuid(LocalizationTarget->Settings.Guid);
			DownloadTargetFileOp->SetInLocale(CultureName);

			FString Path = FPaths::ProjectSavedDir() / "Temp" / "Game" / LocalizationTarget->Settings.Name / CultureName /
			               LocalizationTarget->Settings.Name + ".po";
			FPaths::MakePathRelativeTo(Path, *FPaths::ProjectDir());
			DownloadTargetFileOp->SetInRelativeOutputFilePathAndName(Path);

			auto OperationCompleteDelegate = FLocalizationServiceOperationComplete::CreateRaw(this,
				&FGridlyLocalizationServiceProvider::OnImportCultureForTargetFromGridly, bIsTargetSet);

			Provider.Execute(DownloadTargetFileOp, TArray<FLocalizationServiceTranslationIdentifier>(),
				ELocalizationServiceOperationConcurrency::Synchronous, OperationCompleteDelegate);

			FPlatformProcess::Sleep(1.f); // Throttles number of requests by sleeping between each
		}
	}
}

void FGridlyLocalizationServiceProvider::OnImportCultureForTargetFromGridly(const FLocalizationServiceOperationRef& Operation,
	ELocalizationServiceOperationCommandResult::Type Result, bool bIsTargetSet)
{
	TSharedPtr<FDownloadLocalizationTargetFile, ESPMode::ThreadSafe> DownloadLocalizationTargetOp = StaticCastSharedRef<
		FDownloadLocalizationTargetFile>(Operation);

	CurrentCultureDownloads.Remove(DownloadLocalizationTargetOp->GetInLocale());

	ImportAllCulturesForTargetFromGridlySlowTask->EnterProgressFrame(1.f);

	if (CurrentCultureDownloads.Num() == 0)
	{
		const FString TargetName = FPaths::GetBaseFilename(DownloadLocalizationTargetOp->GetInRelativeOutputFilePathAndName());

		const auto Target = ILocalizationModule::Get().GetLocalizationTargetByName(TargetName, false);
		const FString AbsoluteFilePathAndName = FPaths::ConvertRelativePathToFull(
			FPaths::ProjectDir() / DownloadLocalizationTargetOp->GetInRelativeOutputFilePathAndName());

		UE_LOG(LogGridlyEditor, Log, TEXT("Loading from file: %s"), *AbsoluteFilePathAndName);

		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
		if (!bIsTargetSet)
		{
			LocalizationCommandletTasks::ImportTextForTarget(MainFrameParentWindow.ToSharedRef(), Target,
				FPaths::GetPath(FPaths::GetPath(AbsoluteFilePathAndName)));

			Target->UpdateWordCountsFromCSV();
			Target->UpdateStatusFromConflictReport();
		}

		ImportAllCulturesForTargetFromGridlySlowTask.Reset();
	}
}

void FGridlyLocalizationServiceProvider::ExportNativeCultureForTargetToGridly(
	TWeakObjectPtr<ULocalizationTarget> LocalizationTarget, bool bIsTargetSet)
{
	TArray<FPolyglotTextData> PolyglotTextDatas;

	ExportNativeCultureFromTargetToGridlySlowTask = MakeShareable(new FScopedSlowTask(1.f,
		LOCTEXT("ExportNativeCultureForTargetToGridlyText", "Exporting native culture for target to Gridly")));

	ExportNativeCultureFromTargetToGridlySlowTask->MakeDialog();

	ULocalizationTarget* InLocalizationTarget = LocalizationTarget.Get();
	if (InLocalizationTarget)
	{
		if (FGridlyLocalizedText::GetSourceStringsAsPolyglotTextDatas(InLocalizationTarget, PolyglotTextDatas))
		{
			FString JsonString;
			FGridlyLocalizedTextConverter::ConvertToJson(PolyglotTextDatas, JsonString);
			UE_LOG(LogGridlyEditor, Log, TEXT("%s"), *JsonString);
			UE_LOG(LogGridlyEditor, Log, TEXT("Number of entries: %d"), PolyglotTextDatas.Num());

			UGridlyGameSettings* GameSettings = GetMutableDefault<UGridlyGameSettings>();
			const FString ApiKey = GameSettings->ExportApiKey;
			const FString ViewId = GameSettings->ExportViewId;

			FStringFormatNamedArguments Args;
			Args.Add(TEXT("ViewId"), *ViewId);
			const FString Url = FString::Format(TEXT("https://api.gridly.com/v1/views/{ViewId}/records"),
				Args);

			auto HttpRequest = FHttpModule::Get().CreateRequest();
			HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));
			HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
			HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("ApiKey %s"), *ApiKey));
			HttpRequest->SetContentAsString(JsonString);

			HttpRequest->SetVerb(TEXT("PATCH"));
			HttpRequest->SetURL(Url);

			HttpRequest->OnProcessRequestComplete().
			             BindRaw(this, &FGridlyLocalizationServiceProvider::OnExportNativeCultureForTargetToGridly);

			HttpRequest->ProcessRequest();

			ExportNativeCultureFromTargetToGridlySlowTask->EnterProgressFrame(.3f);
		}
	}
}

void FGridlyLocalizationServiceProvider::OnExportNativeCultureForTargetToGridly(FHttpRequestPtr HttpRequestPtr,
	FHttpResponsePtr HttpResponsePtr, bool bSuccess)
{
	ExportNativeCultureFromTargetToGridlySlowTask->EnterProgressFrame(.3f);

	if (bSuccess)
	{
		if (HttpResponsePtr->GetResponseCode() == EHttpResponseCodes::Ok)
		{
			const FString Content = HttpResponsePtr->GetContentAsString();
			const auto JsonStringReader = TJsonReaderFactory<TCHAR>::Create(Content);
			TArray<TSharedPtr<FJsonValue>> JsonValueArray;
			FJsonSerializer::Deserialize(JsonStringReader, JsonValueArray);

			const FString Message = FString::Printf(TEXT("Number of entries updated: %d"), JsonValueArray.Num());
			UE_LOG(LogGridlyEditor, Log, TEXT("%s"), *Message);
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
		}
		else
		{
			const FString Content = HttpResponsePtr->GetContentAsString();
			const FString ErrorReason = FString::Printf(TEXT("Error: %d, reason: %s"), HttpResponsePtr->GetResponseCode(),
				*Content);
			UE_LOG(LogGridlyEditor, Error, TEXT("%s"), *ErrorReason);
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ErrorReason));
		}
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("GridlyConnectionError", "ERROR: Unable to connect to Gridly"));
	}

	ExportNativeCultureFromTargetToGridlySlowTask.Reset();
}

#undef LOCTEXT_NAMESPACE
