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
	
	/** Updates interaction prompt near the crosshair. */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "SetInteractionPrompt"))
	void BP_SetInteractionPrompt(bool bVisible, const FText& ObjectName, const FText& HintText);

	/** Updates current objective text. */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "SetObjectiveText"))
	void BP_SetObjectiveText(const FText& ObjectiveText);

	/** Updates objective countdown timer in seconds. */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "SetObjectiveTimer"))
	void BP_SetObjectiveTimer(float RemainingTimeSeconds);

	/** Updates run kill counter. */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "SetKillCount"))
	void BP_SetKillCount(int32 KillCount);
};
