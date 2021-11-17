// Copyright (c) 2021 LocalizeDirect AB

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "GridlyBPFunctionLibrary.generated.h"

UCLASS()
class UGridlyBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = Gridly, BlueprintPure, BlueprintCallable)
	static FString GetLocalizationPreviewCulture();

	UFUNCTION(Category = Gridly, BlueprintCallable)
	static void EnableLocalizationPreview(const FString& Culture);

	UFUNCTION(Category = Gridly, BlueprintCallable)
	static void UpdateLocalizationPreview(const TArray<FPolyglotTextData>& PolyglotTextDatas);
};
