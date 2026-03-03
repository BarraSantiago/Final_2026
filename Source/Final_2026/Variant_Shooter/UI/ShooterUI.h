// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterUI.generated.h"

/**
 *  Simple scoreboard UI for a first person shooter game
 */
UCLASS(abstract)
class FINAL_2026_API UShooterUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Allows Blueprint to update score sub-widgets */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "Update Score"))
	void BP_UpdateScore(uint8 TeamByte, int32 Score);
	
	
	/** Shows final ending results at end of run. */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "ShowEnding"))
	void BP_ShowEnding(const FName& EndingId, const FText& EndingText, bool bWon);
};
