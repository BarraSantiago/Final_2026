// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

class UMainMenuUI;
enum class EShooterRunMode : uint8;

/**
 * Player controller used by the main menu map.
 */
UCLASS()
class FINAL_2026_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Main menu widget blueprint class. */
	UPROPERTY(EditAnywhere, Category = "Main Menu")
	TSubclassOf<UMainMenuUI> MainMenuUIClass;

	/** Target level to load for key + escape mode. */
	UPROPERTY(EditAnywhere, Category = "Main Menu")
	FName KeyEscapeLevelName = FName("Lvl_Shooter");

	/** Target level to load for survival mode. */
	UPROPERTY(EditAnywhere, Category = "Main Menu")
	FName SurvivalLevelName = FName("Lvl_FirstPerson");

	/** Shooter game mode override passed while opening gameplay levels. */
	UPROPERTY(EditAnywhere, Category = "Main Menu")
	FString GameplayGameModeOverride = TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterGameMode.BP_ShooterGameMode_C");

	/** Spawned menu widget instance. */
	UPROPERTY(Transient)
	TObjectPtr<UMainMenuUI> MainMenuUI;

	virtual void BeginPlay() override;

	void LaunchSelectedRun(EShooterRunMode SelectedMode, FName TargetLevelName);

	UFUNCTION()
	void HandleKeyEscapeSelected();

	UFUNCTION()
	void HandleSurvivalSelected();

	UFUNCTION()
	void HandleOptionsSelected();

	UFUNCTION()
	void HandleQuitSelected();
};
