// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterBulletCounterUI.generated.h"

/**
 *  Simple bullet counter UI widget for a first person shooter game
 */
UCLASS(abstract)
class FINAL_2026_API UShooterBulletCounterUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Allows Blueprint to update sub-widgets with the new bullet count */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "UpdateBulletCounter"))
	void BP_UpdateBulletCounter(int32 MagazineSize, int32 BulletCount);

	/** Allows Blueprint to update sub-widgets with the new life total and play a damage effect on the HUD */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "Damaged"))
	void BP_Damaged(float LifePercent);

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
