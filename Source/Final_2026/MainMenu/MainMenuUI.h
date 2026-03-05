// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuUI.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMainMenuSelectedDelegate);

/**
 * Runtime-built main menu widget with mode selection buttons.
 */
UCLASS()
class FINAL_2026_API UMainMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "Main Menu")
	FMainMenuSelectedDelegate OnKeyEscapeSelected;

	UPROPERTY(BlueprintAssignable, Category = "Main Menu")
	FMainMenuSelectedDelegate OnSurvivalSelected;

protected:

	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleKeyEscapeClicked();

	UFUNCTION()
	void HandleSurvivalClicked();

	void BuildRuntimeMenu();

	UPROPERTY(Transient)
	TObjectPtr<UButton> KeyEscapeButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SurvivalButton;
};
