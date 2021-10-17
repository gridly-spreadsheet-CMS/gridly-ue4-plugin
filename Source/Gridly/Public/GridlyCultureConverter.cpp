// Copyright (c) 2021 LocalizeDirect AB

#include "GridlyCultureConverter.h"

#include "Gridly.h"
#include "GridlyGameSettings.h"

#if WITH_EDITOR
#include "LocalizationSettings.h"
#include "LocalizationTargetTypes.h"
#endif

#include "Internationalization/Regex.h"
#include "Kismet/KismetInternationalizationLibrary.h"

TArray<FString> FGridlyCultureConverter::GetTargetCultures()
{
	TArray<FString> TargetCultures;

#if WITH_EDITOR
	TArray<ULocalizationTarget*> LocalizationTargets = ULocalizationSettings::GetGameTargetSet()->TargetObjects;

	FString CulturesString;

	for (int i = 0; i < LocalizationTargets.Num(); i++)
	{
		TArray<FCultureStatistics> CultureStatistics = LocalizationTargets[i]->Settings.SupportedCulturesStatistics;
		for (int j = 0; j < CultureStatistics.Num(); j++)
		{
			const FString& Culture = CultureStatistics[j].CultureName;
			TargetCultures.Add(Culture);

			if (CulturesString.Len() > 0)
			{
				CulturesString.Append(", ");
			}
			CulturesString.Append(Culture);
		}
	}

	UE_LOG(LogGridly, Verbose, TEXT("Available cultures: %s"), *CulturesString);
#else
	TargetCultures = FTextLocalizationManager::Get().GetLocalizedCultureNames(ELocalizationLoadFlags::Game);

	for (int i = 0; i < TargetCultures.Num(); i++)
	{
		UE_LOG(LogGridly, Log, TEXT("Culture: %s"), *TargetCultures[i]);
	}
#endif

	return TargetCultures;
}

bool FGridlyCultureConverter::ConvertFromGridly(
	const TArray<FString>& AvailableCultures, const FString& GridlyCulture, FString& OutCulture)
{
	if (GridlyCulture.Len() > 0)
	{
		// Use custom mapping if it is available

		UGridlyGameSettings* GameSettings = GetMutableDefault<UGridlyGameSettings>();

		if (GameSettings->bUseCustomCultureMapping)
		{
			const FString* CustomCulture = GameSettings->CustomCultureMapping.FindKey(GridlyCulture);

			if (CustomCulture != nullptr)
			{
				OutCulture = *CustomCulture;
				return true;
			}
		}

		// Otherwise follow rules of "enUS" -> "en-US"
		
		FString Culture = GridlyCulture;
		const FRegexPattern RegexPattern("([a-z]+)([A-Z]+)");
		FRegexMatcher RegexMatcher(RegexPattern, GridlyCulture);
		if (RegexMatcher.FindNext())
		{
			Culture = RegexMatcher.GetCaptureGroup(1) + "-" + RegexMatcher.GetCaptureGroup(2);
			OutCulture = UKismetInternationalizationLibrary::GetSuitableCulture(AvailableCultures, Culture, TEXT(""));
			return true;
		}
	}

	return false;
}

bool FGridlyCultureConverter::ConvertToGridly(const FString& Culture, FString& OutGridlyCulture)
{
	if (Culture.Len() > 0)
	{
		// Use custom mapping if it is available

		UGridlyGameSettings* GameSettings = GetMutableDefault<UGridlyGameSettings>();

		if (GameSettings->bUseCustomCultureMapping)
		{
			const FString* CustomCulture = GameSettings->CustomCultureMapping.Find(Culture);

			if (CustomCulture != nullptr)
			{
				OutGridlyCulture = *CustomCulture;
				return true;
			}
		}

		// Otherwise follow rules of "en-US" -> "enUS"

		FString Left, Right;
		if (Culture.Split("-", &Left, &Right))
		{
			OutGridlyCulture = Left + Right;
			return true;
		}
	}

	return false;
}
