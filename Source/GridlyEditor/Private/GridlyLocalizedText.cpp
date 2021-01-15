// Copyright © 2020 LocalizeDirect AB

#include "GridlyLocalizedText.h"

#include "GridlyEditor.h"
#include "LocalizationConfigurationScript.h"
#include "LocTextHelper.h"
#include "Internationalization/PolyglotTextData.h"

bool FGridlyLocalizedText::GetSourceStringsAsPolyglotTextDatas(ULocalizationTarget* LocalizationTarget, TArray<FPolyglotTextData>& OutPolyglotTextDatas)
{
	const FString ConfigFilePath = LocalizationConfigurationScript::GetRegenerateResourcesConfigPath(LocalizationTarget);

	FInternationalization& I18N = FInternationalization::Get();

	const FString SectionName = TEXT("RegenerateResources");

	// Get native culture.
	const int NativeCultureIndex = LocalizationTarget->Settings.NativeCultureIndex;
	FString NativeCulture = LocalizationTarget->Settings.SupportedCulturesStatistics[NativeCultureIndex].CultureName;

	// Get source path.
	FString SourcePath;
	if (!GConfig->GetString(*SectionName, TEXT("SourcePath"), SourcePath, ConfigFilePath))
	{
		UE_LOG(LogGridlyEditor, Error, TEXT("No source path specified."));
		return false;
	}

	// Get destination path.
	FString DestinationPath;
	if (!GConfig->GetString(*SectionName, TEXT("DestinationPath"), DestinationPath, ConfigFilePath))
	{
		UE_LOG(LogGridlyEditor, Error, TEXT("No destination path specified."));
		return false;
	}

	// Get manifest name.
	FString ManifestName;
	if (!GConfig->GetString(*SectionName, TEXT("ManifestName"), ManifestName, ConfigFilePath))
	{
		UE_LOG(LogGridlyEditor, Error, TEXT("No manifest name specified."));
		return false;
	}

	// Get archive name.
	FString ArchiveName;
	if (!GConfig->GetString(*SectionName, TEXT("ArchiveName"), ArchiveName, ConfigFilePath))
	{
		UE_LOG(LogGridlyEditor, Error, TEXT("No archive name specified."));
		return false;
	}

	// Get resource name.
	FString ResourceName;
	if (!GConfig->GetString(*SectionName, TEXT("ResourceName"), ResourceName, ConfigFilePath))
	{
		UE_LOG(LogGridlyEditor, Error, TEXT("No resource name specified."));
		return false;
	}

	// Source path needs to be relative to Engine or Game directory
	const FString ConfigFullPath = FPaths::ConvertRelativePathToFull(ConfigFilePath);
	const FString EngineFullPath = FPaths::ConvertRelativePathToFull(FPaths::EngineConfigDir());
	const bool IsEngineManifest = ConfigFullPath.StartsWith(EngineFullPath);

	if (IsEngineManifest)
	{
		SourcePath = FPaths::Combine(*FPaths::EngineDir(), *SourcePath);
		DestinationPath = FPaths::Combine(*FPaths::EngineDir(), *DestinationPath);
	}
	else
	{
		SourcePath = FPaths::Combine(*FPaths::ProjectDir(), *SourcePath);
		DestinationPath = FPaths::Combine(*FPaths::ProjectDir(), *DestinationPath);
	}

	TArray<FString> CulturesToGenerate;
	CulturesToGenerate.Add(NativeCulture);

	// Load the manifest and all archives
	FLocTextHelper LocTextHelper(SourcePath, ManifestName, ArchiveName, NativeCulture, CulturesToGenerate, nullptr);
	{
		FText LoadError;
		if (!LocTextHelper.LoadAll(ELocTextHelperLoadFlags::LoadOrCreate, &LoadError))
		{
			UE_LOG(LogGridlyEditor, Error, TEXT("%s"), *LoadError.ToString());
			return false;
		}
	}

	LocTextHelper.EnumerateSourceTexts(
		[&LocTextHelper, &OutPolyglotTextDatas, &NativeCulture](TSharedRef<FManifestEntry> InManifestEntry)
		{
			for (const FManifestContext& Context : InManifestEntry->Contexts)
			{
				FLocItem TranslationText;
				LocTextHelper.GetRuntimeText(NativeCulture, InManifestEntry->Namespace, Context.Key,
					Context.KeyMetadataObj, ELocTextExportSourceMethod::NativeText, InManifestEntry->Source, TranslationText, true);

				const FString SourceKey = Context.Key.GetString();
				const FString SourceNamespace = InManifestEntry->Namespace.GetString();
				const FString SourceText = TranslationText.Text;

				FPolyglotTextData PolyglotTextData(ELocalizedTextSourceCategory::Game, SourceNamespace, SourceKey, SourceText,
					NativeCulture);
				OutPolyglotTextDatas.Add(PolyglotTextData);

				//UE_LOG(LogGridlyEditor, Verbose, TEXT("%s,%s: %s"), *SourceNamespace, *SourceKey, *SourceText);
			}
			return true;
		}, true);

	return true;
}

