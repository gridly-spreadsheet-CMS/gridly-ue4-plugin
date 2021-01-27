// Copyright © 2020 LocalizeDirect AB

#include "AssetTypeActions_GridlyDataTable.h"

#include "AssetTypeActions_CSVAssetBase.h"
#include "DataTableEditorUtils.h"
#include "DesktopPlatformModule.h"
#include "GridlyEditor.h"
#include "GridlyGameSettings.h"
#include "GridlyLocalizedTextConverter.h"
#include "GridlyStyle.h"
#include "GridlyTableRow.h"
#include "GridlyTask_ImportDataTableFromGridly.h"
#include "HttpModule.h"
#include "IDesktopPlatform.h"
#include "JsonObjectConverter.h"
#include "Slate.h"
#include "ToolMenus.h"
#include "Editor/DataTableEditor/Public/DataTableEditorModule.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

class FGridlyDataTableCommands final : public TCommands<FGridlyDataTableCommands>
{
public:
	FGridlyDataTableCommands() :
		TCommands<FGridlyDataTableCommands>("GridlyDataTableEditor",
			NSLOCTEXT("Gridly", "GridlyDataTableEditor", "Gridly Data Table Editor"), NAME_None,
			FEditorStyle::GetStyleSetName())
	{
	}

	TSharedPtr<FUICommandInfo> ImportFromGridly;
	TSharedPtr<FUICommandInfo> ExportToGridly;

	/** Initialize commands */
	virtual void RegisterCommands() override;
};

void FGridlyDataTableCommands::RegisterCommands()
{
	UI_COMMAND(ImportFromGridly, "Import from Gridly",
		"Imports data table from Gridly.", EUserInterfaceActionType::Button, FInputChord());

	UI_COMMAND(ExportToGridly, "Export to Gridly",
		"Exports data table to Gridly.", EUserInterfaceActionType::Button, FInputChord());
}

void FAssetTypeActions_GridlyDataTable::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	const auto Tables = GetTypedWeakObjectPtrs<UObject>(InObjects);

	TArray<FString> ImportPaths;
	for (auto TableIter = Tables.CreateConstIterator(); TableIter; ++TableIter)
	{
		const UDataTable* CurTable = Cast<UDataTable>((*TableIter).Get());
		if (CurTable)
		{
			CurTable->AssetImportData->ExtractFilenames(ImportPaths);
		}
	}

	Section.AddMenuEntry(
		"DataTable_ImportFromGridly",
		LOCTEXT("DataTable_ImportFromGridly", "Import from Gridly"),
		LOCTEXT("DataTable_ImportFromGridlyTooltip", "Import data table from Gridly"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_GridlyDataTable::ExecuteImportFromGridly, Tables),
			FCanExecuteAction()
			)
		);

	Section.AddMenuEntry(
		"DataTable_ExportAsCSV",
		LOCTEXT("DataTable_ExportAsCSV", "Export as CSV"),
		LOCTEXT("DataTable_ExportAsCSVTooltip", "Export the data table as a file containing CSV data."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_GridlyDataTable::ExecuteExportAsCSV, Tables),
			FCanExecuteAction()
			)
		);

	Section.AddMenuEntry(
		"DataTable_ExportAsJSON",
		LOCTEXT("DataTable_ExportAsJSON", "Export as JSON"),
		LOCTEXT("DataTable_ExportAsJSONTooltip", "Export the data table as a file containing JSON data."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_GridlyDataTable::ExecuteExportAsJSON, Tables),
			FCanExecuteAction()
			)
		);
}

void FAssetTypeActions_GridlyDataTable::OpenAssetEditor(const TArray<UObject*>& InObjects,
	TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	TArray<UDataTable*> DataTablesToOpen;
	TArray<UDataTable*> InvalidDataTables;

	for (UObject* Obj : InObjects)
	{
		UDataTable* Table = Cast<UDataTable>(Obj);
		if (Table)
		{
			if (Table->GetRowStruct())
			{
				DataTablesToOpen.Add(Table);
			}
			else
			{
				InvalidDataTables.Add(Table);
			}
		}
	}

	if (InvalidDataTables.Num() > 0)
	{
		FTextBuilder DataTablesListText;
		DataTablesListText.Indent();
		for (UDataTable* Table : InvalidDataTables)
		{
			const FName ResolvedRowStructName = Table->GetRowStructName();
			DataTablesListText.AppendLineFormat(LOCTEXT("DataTable_MissingRowStructListEntry", "* {0} (Row Structure: {1})"),
				FText::FromString(Table->GetName()), FText::FromName(ResolvedRowStructName));
		}

		FText Title = LOCTEXT("DataTable_MissingRowStructTitle", "Continue?");
		const EAppReturnType::Type DlgResult = FMessageDialog::Open(
			EAppMsgType::YesNoCancel,
			FText::Format(LOCTEXT("DataTable_MissingRowStructMsg",
					"The following Data Tables are missing their row structure and will not be editable.\n\n{0}\n\nDo you want to open these data tables?"),
				DataTablesListText.ToText()),
			&Title
			);

		switch (DlgResult)
		{
			case EAppReturnType::Yes:
				DataTablesToOpen.Append(InvalidDataTables);
				break;
			case EAppReturnType::Cancel:
				return;
			default:
				break;
		}
	}

	FDataTableEditorModule& DataTableEditorModule = FModuleManager::LoadModuleChecked<FDataTableEditorModule>("DataTableEditor");

	FGridlyDataTableCommands::Register();

	for (UDataTable* Table : DataTablesToOpen)
	{
		UGridlyDataTable* GridlyDataTable = Cast<UGridlyDataTable>(Table);

		TSharedPtr<FExtender> Extender = MakeShareable(new FExtender);
		TSharedPtr<FUICommandList> CommandList = MakeShareable(new FUICommandList);
		CommandList->MapAction(FGridlyDataTableCommands::Get().ImportFromGridly,
			FExecuteAction::CreateRaw(this, &FAssetTypeActions_GridlyDataTable::ImportFromGridly, GridlyDataTable));
		CommandList->MapAction(FGridlyDataTableCommands::Get().ExportToGridly,
			FExecuteAction::CreateRaw(this, &FAssetTypeActions_GridlyDataTable::ExportToGridly, GridlyDataTable));

		Extender->AddToolBarExtension("DataTableCommands", EExtensionHook::Before, CommandList,
			FToolBarExtensionDelegate::CreateRaw(this, &FAssetTypeActions_GridlyDataTable::AddToolbarButton));
		DataTableEditorModule.GetToolBarExtensibilityManager()->AddExtender(Extender);

		DataTableEditorModule.CreateDataTableEditor(EToolkitMode::Standalone, EditWithinLevelEditor, Table);

		DataTableEditorModule.GetToolBarExtensibilityManager()->RemoveExtender(Extender);
	}

	//FGridlyDataTableCommands::Unregister();
}

void FAssetTypeActions_GridlyDataTable::ExecuteImportFromGridly(TArray<TWeakObjectPtr<UObject>> Objects)
{
	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UGridlyDataTable* GridlyDataTable = Cast<UGridlyDataTable>((*ObjIt).Get());
		ImportFromGridly(GridlyDataTable);
	}
}

void FAssetTypeActions_GridlyDataTable::ExecuteExportAsCSV(TArray<TWeakObjectPtr<UObject>> Objects)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto DataTable = Cast<UDataTable>((*ObjIt).Get());
		if (DataTable)
		{
			const FText Title = FText::Format(LOCTEXT("DataTable_ExportCSVDialogTitle", "Export '{0}' as CSV..."),
				FText::FromString(*DataTable->GetName()));
			const FString CurrentFilename = DataTable->AssetImportData->GetFirstFilename();
			const FString FileTypes = TEXT("Data Table CSV (*.csv)|*.csv");

			TArray<FString> OutFilenames;
			DesktopPlatform->SaveFileDialog(
				ParentWindowWindowHandle,
				Title.ToString(),
				(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetPath(CurrentFilename),
				(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetBaseFilename(CurrentFilename) + TEXT(".csv"),
				FileTypes,
				EFileDialogFlags::None,
				OutFilenames
				);

			if (OutFilenames.Num() > 0)
			{
				FFileHelper::SaveStringToFile(DataTable->GetTableAsCSV(), *OutFilenames[0]);
			}
		}
	}
}

void FAssetTypeActions_GridlyDataTable::ExecuteExportAsJSON(TArray<TWeakObjectPtr<UObject>> Objects)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto DataTable = Cast<UDataTable>((*ObjIt).Get());
		if (DataTable)
		{
			const FText Title = FText::Format(LOCTEXT("DataTable_ExportJSONDialogTitle", "Export '{0}' as JSON..."),
				FText::FromString(*DataTable->GetName()));
			const FString CurrentFilename = DataTable->AssetImportData->GetFirstFilename();
			const FString FileTypes = TEXT("Data Table JSON (*.json)|*.json");

			TArray<FString> OutFilenames;
			DesktopPlatform->SaveFileDialog(
				ParentWindowWindowHandle,
				Title.ToString(),
				(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetPath(CurrentFilename),
				(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetBaseFilename(CurrentFilename) + TEXT(".json"),
				FileTypes,
				EFileDialogFlags::None,
				OutFilenames
				);

			if (OutFilenames.Num() > 0)
			{
				FFileHelper::SaveStringToFile(DataTable->GetTableAsJSON(EDataTableExportFlags::UseJsonObjectsForStructs),
					*OutFilenames[0]);
			}
		}
	}
}

void FAssetTypeActions_GridlyDataTable::ImportFromGridly(UGridlyDataTable* DataTable)
{
	TSharedPtr<FScopedSlowTask, ESPMode::Fast> ImportDataTableFromGridlySlowTask = MakeShareable(new FScopedSlowTask(1.f,
		LOCTEXT("ImportGridlyDataTableSlowTask", "Importing data table from Gridly")));

	ImportDataTableFromGridlySlowTask->MakeDialog();

	UGridlyDataTable* GridlyDataTable = Cast<UGridlyDataTable>(DataTable);
	check(GridlyDataTable);

	UGridlyTask_ImportDataTableFromGridly* Task =
		UGridlyTask_ImportDataTableFromGridly::ImportDataTableFromGridly(nullptr, GridlyDataTable);

	ImportDataTableFromGridlySlowTask->EnterProgressFrame(.5f);

	FDataTableEditorUtils::BroadcastPreChange(GridlyDataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowList);

	Task->OnSuccessDelegate.BindLambda(
		[GridlyDataTable, ImportDataTableFromGridlySlowTask](const TArray<FGridlyTableRow>& GridlyTableRows) mutable
		{
			ImportDataTableFromGridlySlowTask->EnterProgressFrame(.5f);
			ImportDataTableFromGridlySlowTask.Reset();
			FDataTableEditorUtils::BroadcastPostChange(GridlyDataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowList);
		});

	Task->Activate();
}

void FAssetTypeActions_GridlyDataTable::ExportToGridly(UGridlyDataTable* DataTable)
{
	TSharedPtr<FScopedSlowTask, ESPMode::Fast> ExportDataTableToGridlySlowTask = MakeShareable(new FScopedSlowTask(1.f,
		LOCTEXT("ExportGridlyDataTableSlowTask", "Exporting data table to Gridly")));

	ExportDataTableToGridlySlowTask->MakeDialog();

	FString JsonString;
	if (FGridlyLocalizedTextConverter::ConvertToJson(DataTable, JsonString))
	{
		UE_LOG(LogGridlyEditor, Log, TEXT("%s"), *JsonString);

		UGridlyGameSettings* GameSettings = GetMutableDefault<UGridlyGameSettings>();
		const FString ApiKey = GameSettings->ExportApiKey;
		const FString ViewId = DataTable->ViewId;

		FStringFormatNamedArguments Args;
		Args.Add(TEXT("ViewId"), *ViewId);
		const FString Url = FString::Format(TEXT("https://api.gridly.com/v1/views/{ViewId}/records"), Args);

		auto HttpRequest = FHttpModule::Get().CreateRequest();
		HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("ApiKey %s"), *ApiKey));
		HttpRequest->SetContentAsString(JsonString);
		HttpRequest->SetVerb(TEXT("PATCH"));
		HttpRequest->SetURL(Url);
		
		ExportDataTableToGridlySlowTask->EnterProgressFrame(.5f);

		HttpRequest->OnProcessRequestComplete().
		             BindLambda([ExportDataTableToGridlySlowTask](FHttpRequestPtr HttpRequest,
			             FHttpResponsePtr HttpResponse, bool bSuccess) mutable
			             {
				             ExportDataTableToGridlySlowTask->EnterProgressFrame(.5f);
				             ExportDataTableToGridlySlowTask.Reset();
			             });

		HttpRequest->ProcessRequest();
	}
}

void FAssetTypeActions_GridlyDataTable::AddToolbarButton(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(
		FGridlyDataTableCommands::Get().ImportFromGridly,
		NAME_None,
		LOCTEXT("ImportFromGridly", "Import from Gridly"),
		LOCTEXT("ImportFromGridlyTooltip", "Import data table from Gridly"),
		FSlateIcon(FGridlyStyle::GetStyleSetName(), "Gridly.ImportAction")
		);

	Builder.AddToolBarButton(
		FGridlyDataTableCommands::Get().ExportToGridly,
		NAME_None,
		LOCTEXT("ExportToGridly", "Export to Gridly"),
		LOCTEXT("ExportToGridlyTooltip", "Export data table to Gridly"),
		FSlateIcon(FGridlyStyle::GetStyleSetName(), "Gridly.ExportAction")
		);
}

#undef LOCTEXT_NAMESPACE
