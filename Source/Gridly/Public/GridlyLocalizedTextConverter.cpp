// Copyright © 2020 LocalizeDirect AB

#include "GridlyLocalizedTextConverter.h"

#include "Gridly.h"
#include "GridlyCultureConverter.h"
#include "GridlyGameSettings.h"
#include "Internationalization/PolyglotTextData.h"
#include "Misc/FileHelper.h"

bool FGridlyLocalizedTextConverter::TableRowsToPolyglotTextDatas(const TArray<FGridlyTableRow>& TableRows,
	TMap<FString, FPolyglotTextData>& OutPolyglotTextDatas)
{
	UGridlyGameSettings* GameSettings = GetMutableDefault<UGridlyGameSettings>();
	const TArray<FString> TargetCultures = FGridlyCultureConverter::GetTargetCultures();

	const bool bUseCombinedNamespaceKey = GameSettings->bUseCombinedNamespaceId;
	const bool bUsePathAsNamespace = !bUseCombinedNamespaceKey && GameSettings->NamespaceColumnId == "path";

	for (int i = 0; i < TableRows.Num(); i++)
	{
		UE_LOG(LogGridly, Verbose, TEXT("Row %d: %s (%s)"), i, *TableRows[i].Id, *TableRows[i].Path);

		FString Key = TableRows[i].Id;
		FString Namespace = bUsePathAsNamespace ? TableRows[i].Path : TEXT("");
		FString SourceCulture;
		FString SourceText;
		TMap<FString, FString> Translations;

		for (int j = 0; j < TableRows[i].Cells.Num(); j++)
		{
			const FGridlyTableCell& GridlyTableCell = TableRows[i].Cells[j];

			// If special columns

			if (!bUsePathAsNamespace && GridlyTableCell.ColumnId == GameSettings->NamespaceColumnId)
			{
				Namespace = GridlyTableCell.Value;
				continue;
			}

			// If language column

			if (GridlyTableCell.ColumnId.StartsWith(GameSettings->SourceLanguageColumnIdPrefix))
			{
				const FString GridlyCulture = GridlyTableCell.ColumnId.RightChop(GameSettings->SourceLanguageColumnIdPrefix.Len());
				FString Culture;
				if (FGridlyCultureConverter::ConvertFromGridly(TargetCultures, GridlyCulture, Culture))
				{
					SourceCulture = Culture;
					SourceText = GridlyTableCell.Value;
				}
			}
			else if (GridlyTableCell.ColumnId.StartsWith(GameSettings->TargetLanguageColumnIdPrefix))
			{
				const FString GridlyCulture = GridlyTableCell.ColumnId.RightChop(GameSettings->TargetLanguageColumnIdPrefix.Len());
				FString Culture;
				if (FGridlyCultureConverter::ConvertFromGridly(TargetCultures, GridlyCulture, Culture))
				{
					Translations.Add(Culture, GridlyTableCell.Value);
				}
			}
		}

		// Namespace / key fixes

		Namespace = Namespace.Replace(TEXT(" "), TEXT(""));

		if (bUseCombinedNamespaceKey)
		{
			FString NewKey;
			if (Key.Split(",", &Namespace, &NewKey))
			{
				Key = NewKey;
			}
		}

		if (SourceText.IsEmpty() || SourceCulture.IsEmpty())
		{
			UE_LOG(LogGridly, Warning, TEXT("Could not find native culture/source string in imported text with key: %s,%s"),
				*Namespace, *Key);
			continue;
		}

		FPolyglotTextData PolyglotTextData(ELocalizedTextSourceCategory::Game, Namespace, Key, SourceText, SourceCulture);

		for (const TPair<FString, FString>& Pair : Translations)
		{
			if (!Pair.Value.IsEmpty())
			{
				PolyglotTextData.AddLocalizedString(Pair.Key, Pair.Value);
			}
		}

		OutPolyglotTextDatas.Add(Key, PolyglotTextData);
	}

	return OutPolyglotTextDatas.Num() > 0;
}

bool FGridlyLocalizedTextConverter::WritePoFile(const TArray<FPolyglotTextData>& PolyglotTextDatas, const FString& TargetCulture,
	const FString& Path)
{
	TArray<FString> Lines;

	for (int i = 0; i < PolyglotTextDatas.Num(); i++)
	{
		FString TargetString;
		if (PolyglotTextDatas[i].GetLocalizedString(TargetCulture, TargetString))
		{
			Lines.Add(FString::Printf(TEXT("msgctxt \"%s,%s\""), *PolyglotTextDatas[i].GetNamespace(),
				*PolyglotTextDatas[i].GetKey()));
			Lines.Add(FString::Printf(TEXT("msgid \"%s\""), *PolyglotTextDatas[i].GetNativeString()));
			Lines.Add(FString::Printf(TEXT("msgstr \"%s\""), *TargetString));
			Lines.Add("");
		}
	}

	if (FFileHelper::SaveStringArrayToFile(Lines, *Path))
	{
		UE_LOG(LogGridly, Log, TEXT("Exported .po file (%d lines): %s"), Lines.Num(), *Path);
	}
	else
	{
		UE_LOG(LogGridly, Error, TEXT("Failed to export .po file to path: %s"), *Path);
	}

	return false;
}

bool FGridlyLocalizedTextConverter::ConvertToJson(const TArray<FPolyglotTextData>& PolyglotTextDatas,
	bool bIncludeTargetTranslations, FString& OutJsonString)
{
	UGridlyGameSettings* GameSettings = GetMutableDefault<UGridlyGameSettings>();
	const TArray<FString> TargetCultures = FGridlyCultureConverter::GetTargetCultures();

	const bool bUseCombinedNamespaceKey = GameSettings->bUseCombinedNamespaceId;
	const bool bUsePathAsNamespace = !bUseCombinedNamespaceKey && GameSettings->NamespaceColumnId == "path";

	TArray<TSharedPtr<FJsonValue>> Rows;

	for (int i = 0; i < PolyglotTextDatas.Num(); i++)
	{
		TSharedPtr<FJsonObject> RowJsonObject = MakeShareable(new FJsonObject);
		TArray<TSharedPtr<FJsonValue>> CellsJsonArray;

		const FString Key = PolyglotTextDatas[i].GetKey();
		const FString Namespace = PolyglotTextDatas[i].GetNamespace();

		// Set record id

		if (bUseCombinedNamespaceKey)
		{
			RowJsonObject->SetStringField("id", FString::Printf(TEXT("%s,%s"), *Namespace, *Key));
		}
		else
		{
			RowJsonObject->SetStringField("id", Key);
		}

		// Set namespace/path

		if (bUsePathAsNamespace)
		{
			RowJsonObject->SetStringField("path", Namespace);
		}
		else if (!GameSettings->NamespaceColumnId.IsEmpty())
		{
			TSharedPtr<FJsonObject> CellJsonObject = MakeShareable(new FJsonObject);
			CellJsonObject->SetStringField("columnId", GameSettings->NamespaceColumnId);
			CellJsonObject->SetStringField("value", Namespace);
			CellsJsonArray.Add(MakeShareable(new FJsonValueObject(CellJsonObject)));
		}

		// Set source language text

		{
			const FString NativeCulture = PolyglotTextDatas[i].GetNativeCulture();
			const FString NativeString = PolyglotTextDatas[i].GetNativeString();

			FString GridlyCulture;
			if (FGridlyCultureConverter::ConvertToGridly(NativeCulture, GridlyCulture))
			{
				TSharedPtr<FJsonObject> CellJsonObject = MakeShareable(new FJsonObject);
				CellJsonObject->SetStringField("columnId", GameSettings->SourceLanguageColumnIdPrefix + GridlyCulture);
				CellJsonObject->SetStringField("value", NativeString);
				CellsJsonArray.Add(MakeShareable(new FJsonValueObject(CellJsonObject)));
			}

			if (bIncludeTargetTranslations)
			{
				for (int j = 0; j < TargetCultures.Num(); j++)
				{
					const FString CultureName = TargetCultures[j];
					FString LocalizedString;

					if (CultureName != NativeCulture
					    && PolyglotTextDatas[i].GetLocalizedString(CultureName, LocalizedString)
					    && FGridlyCultureConverter::ConvertToGridly(CultureName, GridlyCulture))
					{
						TSharedPtr<FJsonObject> CellJsonObject = MakeShareable(new FJsonObject);
						CellJsonObject->SetStringField("columnId", GameSettings->TargetLanguageColumnIdPrefix + GridlyCulture);
						CellJsonObject->SetStringField("value", LocalizedString);
						CellsJsonArray.Add(MakeShareable(new FJsonValueObject(CellJsonObject)));
					}
				}
			}
		}

		// Assign array

		RowJsonObject->SetArrayField("cells", CellsJsonArray);

		Rows.Add(MakeShareable(new FJsonValueObject(RowJsonObject)));
	}

	const TSharedRef<TJsonWriter<>> JsonWriter = TJsonStringWriter<>::Create(&OutJsonString);
	if (FJsonSerializer::Serialize(Rows, JsonWriter))
	{
		return true;
	}

	return false;
}
