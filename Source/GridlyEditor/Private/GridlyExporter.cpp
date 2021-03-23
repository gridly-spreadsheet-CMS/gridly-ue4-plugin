#include "GridlyExporter.h"

#include "GridlyCultureConverter.h"
#include "GridlyDataTableImporterJSON.h"
#include "GridlyGameSettings.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Internationalization/PolyglotTextData.h"

bool FGridlyExporter::ConvertToJson(const TArray<FPolyglotTextData>& PolyglotTextDatas,
	bool bIncludeTargetTranslations, FString& OutJsonString)
{
	UGridlyGameSettings* GameSettings = GetMutableDefault<UGridlyGameSettings>();
	const TArray<FString> TargetCultures = FGridlyCultureConverter::GetTargetCultures();

	const bool bUseCombinedNamespaceKey = GameSettings->bUseCombinedNamespaceId;
	const bool bUsePathAsNamespace = GameSettings->NamespaceColumnId == "path";

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

bool FGridlyExporter::ConvertToJson(const UGridlyDataTable* GridlyDataTable, FString& OutJsonString)
{
	UGridlyGameSettings* GameSettings = GetMutableDefault<UGridlyGameSettings>();
	const TArray<FString> TargetCultures = FGridlyCultureConverter::GetTargetCultures();

	if (!GridlyDataTable->RowStruct)
	{
		return false;
	}

	FString KeyField = GridlyDataTableJSONUtils::GetKeyFieldName(*GridlyDataTable);

	auto JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutJsonString);

	JsonWriter->WriteArrayStart();

	// Iterate over rows
	for (auto RowIt = GridlyDataTable->GetRowMap().CreateConstIterator(); RowIt; ++RowIt)
	{
		JsonWriter->WriteObjectStart();
		{
			// RowName
			const FName RowName = RowIt.Key();
			JsonWriter->WriteValue("id", RowName.ToString());

			// Now the values
			JsonWriter->WriteArrayStart("cells");

			uint8* RowData = RowIt.Value();

			for (TFieldIterator<const FProperty> It(GridlyDataTable->GetRowStruct()); It; ++It)
			{
				const FProperty* BaseProp = *It;
				check(BaseProp);

				const EDataTableExportFlags DTExportFlags = EDataTableExportFlags::None;

				const FString Identifier = DataTableUtils::GetPropertyExportName(BaseProp, DTExportFlags);
				const void* Data = BaseProp->ContainerPtrToValuePtr<void>(RowData, 0);

				if (BaseProp->ArrayDim == 1)
				{
					JsonWriter->WriteObjectStart();

					const FString ExportId = DataTableUtils::GetPropertyExportName(BaseProp, DTExportFlags);
					JsonWriter->WriteValue("columnId", ExportId);

					if (const FEnumProperty* EnumProp = CastField<const FEnumProperty>(BaseProp))
					{
						const FString PropertyValue = DataTableUtils::GetPropertyValueAsString(EnumProp,
							static_cast<uint8*>(RowData), DTExportFlags);
						JsonWriter->WriteValue("value", PropertyValue);
					}
					else if (const FNumericProperty* NumProp = CastField<const FNumericProperty>(BaseProp))
					{
						if (NumProp->IsEnum())
						{
							const FString PropertyValue = DataTableUtils::GetPropertyValueAsString(BaseProp,
								static_cast<uint8*>(RowData), DTExportFlags);
							JsonWriter->WriteValue("value", PropertyValue);
						}
						else if (NumProp->IsInteger())
						{
							const int64 PropertyValue = NumProp->GetSignedIntPropertyValue(Data);
							JsonWriter->WriteValue("value", PropertyValue);
						}
						else
						{
							const double PropertyValue = NumProp->GetFloatingPointPropertyValue(Data);
							JsonWriter->WriteValue("value", PropertyValue);
						}
					}
					else if (const FBoolProperty* BoolProp = CastField<const FBoolProperty>(BaseProp))
					{
						const bool PropertyValue = BoolProp->GetPropertyValue(Data);
						JsonWriter->WriteValue("value", PropertyValue);
					}
					else if (const FArrayProperty* ArrayProp = CastField<const FArrayProperty>(BaseProp))
					{
						// Not supported
					}
					else if (const FSetProperty* SetProp = CastField<const FSetProperty>(BaseProp))
					{
						// Not supported
					}
					else if (const FMapProperty* MapProp = CastField<const FMapProperty>(BaseProp))
					{
						// Not supported
					}
					else if (const FStructProperty* StructProp = CastField<const FStructProperty>(BaseProp))
					{
						// Not supported
					}
					else
					{
						const FString PropertyValue = DataTableUtils::GetPropertyValueAsString(BaseProp,
							static_cast<uint8*>(RowData), DTExportFlags);
						JsonWriter->WriteValue("value", PropertyValue);
					}

					JsonWriter->WriteObjectEnd();
				}
			}

			JsonWriter->WriteArrayEnd();
		}
		JsonWriter->WriteObjectEnd();
	}

	JsonWriter->WriteArrayEnd();

	if (JsonWriter->Close())
	{
		return true;
	}

	return false;
}
