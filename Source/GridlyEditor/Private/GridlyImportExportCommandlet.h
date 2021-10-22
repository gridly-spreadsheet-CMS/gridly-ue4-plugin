// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Commandlets/GatherTextCommandletBase.h"
#include "LocalizationCommandletExecution.h"
#include "ILocalizationServiceProvider.h"
#include "GridlyImportExportCommandlet.generated.h"

/**
 *	GridlyImportExportCommandlet: Commandlet to Export Native Texts to Gridy and Import translations from Gridly. 
 */
UCLASS()
class UGridlyImportExportCommandlet : public UGatherTextCommandletBase
{
	GENERATED_BODY()

public:
	UGridlyImportExportCommandlet(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{}

	//~ Begin UCommandlet Interface
	virtual int32 Main(const FString& Params) override;
	//~ End UCommandlet Interface

private:
	TArray<FString> CulturesToDownload;
	TArray<FString> DownloadedFiles;

private:
	void OnDownloadComplete(const FLocalizationServiceOperationRef& Operation, ELocalizationServiceOperationCommandResult::Type Result, bool bIsTargetSet);
	void BlockingRunLocCommandletTask(const TArray<LocalizationCommandletExecution::FTask>& LocTasks);
};
