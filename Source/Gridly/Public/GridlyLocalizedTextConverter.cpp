// Copyright (c) 2021 LocalizeDirect AB

#include "GridlyLocalizedTextConverter.h"

#include "Gridly.h"
#include "GridlyCultureConverter.h"
#include "GridlyDataTableImporterJSON.h"
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

		if (bUseCombinedNamespaceKey)
		{
			FString NewKey;
			if (Key.Split(",", &Namespace, &NewKey))
			{
				Key = NewKey;
			}
		}

		Namespace = Namespace.Replace(TEXT(" "), TEXT(""));

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

			FString NativeString = PolyglotTextDatas[i].GetNativeString().ReplaceCharWithEscapedChar();
			Lines.Add(FString::Printf(TEXT("msgid \"%s\""), *NativeString));

			TargetString = TargetString.ReplaceCharWithEscapedChar();
			Lines.Add(FString::Printf(TEXT("msgstr \"%s\""), *TargetString));

			Lines.Add(TEXT(""));
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
