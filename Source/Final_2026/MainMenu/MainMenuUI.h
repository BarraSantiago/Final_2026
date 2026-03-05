// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuUI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMainMenuSelectedDelegate);

/**
 * Blueprint-driven main menu widget.
 * Button clicks should call the Notify* functions below.
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

	UPROPERTY(BlueprintAssignable, Category = "Main Menu")
	FMainMenuSelectedDelegate OnOptionsSelected;

	UPROPERTY(BlueprintAssignable, Category = "Main Menu")
	FMainMenuSelectedDelegate OnQuitSelected;

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void NotifyKeyEscapeSelected();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void NotifySurvivalSelected();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void NotifyOptionsSelected();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void NotifyQuitSelected();
};
