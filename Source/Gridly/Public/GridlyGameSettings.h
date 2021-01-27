// Copyright © 2020 LocalizeDirect AB

#pragma once

#include "GridlyGameSettings.generated.h"

UCLASS(BlueprintType, Config = Game, DefaultConfig,
	AutoExpandCategories=("Gridly|Import Settings", "Gridly|Export Settings", "Gridly|Options"))
class GRIDLY_API UGridlyGameSettings final : public UObject
{
	GENERATED_BODY()

public:
	/** The API key can be retrieved from your Gridly dashboard */
	UPROPERTY(Category = "Gridly|Import Settings", BlueprintReadOnly, EditAnywhere, Config)
	FString ImportApiKey;

	/** The view IDs to fetch from Gridly. Record IDs will be combined. Duplicate keys will be ignored. */
	UPROPERTY(Category = "Gridly|Import Settings", BlueprintReadOnly, EditAnywhere, Config)
	TArray<FString> ImportFromViewIds;

	/** The view ID to export the source strings to. */
	UPROPERTY(Category = "Gridly|Export Settings", BlueprintReadOnly, EditAnywhere, Transient)
	FString ExportApiKey;

	/** The view ID to export the source strings to. */
	UPROPERTY(Category = "Gridly|Export Settings", BlueprintReadOnly, EditAnywhere, Transient)
	FString ExportViewId;

	/** Use combined comma-separated "{namespace},{key}" as record ID. WARNING! This should not be changed after a project has already been exported */
	UPROPERTY(Category = "Gridly|Options", BlueprintReadOnly, EditAnywhere, Config)
	bool bUseCombinedNamespaceId = false;

	/** Set to "path" to use Gridly's path tag functionality for namespaces. This can also be mapped to any other column of the string type.  */
	UPROPERTY(Category = "Gridly|Options", BlueprintReadOnly, EditAnywhere, Config,
		meta = (EditCondition="!bUseCombinedNamespaceId"))
	FString NamespaceColumnId = "path";

	/** Column ID prefix for source language columns on Gridly */
	UPROPERTY(Category = "Gridly|Options", BlueprintReadOnly, EditAnywhere, Config)
	FString SourceLanguageColumnIdPrefix = "src_";

	/** Column ID prefix for target language columns on Gridly */
	UPROPERTY(Category = "Gridly|Options", BlueprintReadOnly, EditAnywhere, Config)
	FString TargetLanguageColumnIdPrefix = "tg_";

	/** By default during import and export, Gridly will try to automatically convert to and from Unreal's culture format. This behaviour can be overriden with custom mapping */
	UPROPERTY(Category = "Gridly|Options", BlueprintReadOnly, EditAnywhere, Config)
	bool bUseCustomCultureMapping = true;

	/** This will remap locale settings from Unreal to Gridly. Unreal uses "en-US", while Gridly generally uses "enUS". However, this mapping can be modified to suit the project's needs. */
	UPROPERTY(Category = "Gridly|Options", BlueprintReadOnly, EditAnywhere, Config,
		meta = (EditCondition="bUseCustomCultureMapping"))
	TMap<FString, FString> CustomCultureMapping;

public:
	UGridlyGameSettings();

public:
	static bool OnSettingsSaved();
};
