// Copyright (c) 2021 LocalizeDirect AB

#include "GridlyBPFunctionLibrary.h"

#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/PolyglotTextData.h"

FString UGridlyBPFunctionLibrary::GetLocalizationPreviewCulture()
{
#if WITH_EDITOR
	return FTextLocalizationManager::Get().GetConfiguredGameLocalizationPreviewLanguage();
#else
	return FInternationalization::Get().GetCurrentLanguage()->GetName();
#endif
}

void UGridlyBPFunctionLibrary::EnableLocalizationPreview(const FString& Culture)
{
#if WITH_EDITOR
	FTextLocalizationManager::Get().EnableGameLocalizationPreview(Culture);
	FTextLocalizationManager::Get().ConfigureGameLocalizationPreviewLanguage(Culture);
#endif
}

void UGridlyBPFunctionLibrary::UpdateLocalizationPreview(const TArray<FPolyglotTextData>& PolyglotTextDatas)
{
	FTextLocalizationManager::Get().RegisterPolyglotTextData(PolyglotTextDatas);
	EnableLocalizationPreview(GetLocalizationPreviewCulture());
}
