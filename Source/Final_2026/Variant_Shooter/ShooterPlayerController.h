// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShooterUI.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class UInputMappingContext;
class AShooterCharacter;
class UShooterBulletCounterUI;
class UMainMenuUI;
class UUserWidget;
enum class EShooterRunMode : uint8;

/**
 *  Simple PlayerController for a first person shooter game
 *  Manages input mappings
 *  Respawns the player pawn when it's destroyed
 */
UCLASS(abstract)
class FINAL_2026_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input mapping contexts for this player */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Character class to respawn when the possessed pawn is destroyed */
	UPROPERTY(EditAnywhere, Category="Shooter|Respawn")
	TSubclassOf<AShooterCharacter> CharacterClass;

	/** Type of bullet counter UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	TSubclassOf<UShooterBulletCounterUI> BulletCounterUIClass;

	/** Type of death menu widget to spawn on player death. */
	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	TSubclassOf<UUserWidget> DeathMenuUIClass;

	/** Type of win menu widget to spawn when the player escapes. */
	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	TSubclassOf<UUserWidget> WinMenuUIClass;

	/** Type of main menu widget to spawn when no run mode has been selected yet. */
	UPROPERTY(EditAnywhere, Category="Shooter|Main Menu")
	TSubclassOf<UMainMenuUI> MainMenuUIClass;

	/** Level name used by the death menu "Main Menu" action. */
	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	FName MainMenuLevelName = FName("Lvl_Shooter");

	/** Target level to load for key + escape mode. */
	UPROPERTY(EditAnywhere, Category="Shooter|Main Menu")
	FName KeyEscapeLevelName = FName("Lvl_Shooter");

	/** Target level to load for survival mode. */
	UPROPERTY(EditAnywhere, Category="Shooter|Main Menu")
	FName SurvivalLevelName = FName("Lvl_FirstPerson");

	/** Optional game mode override passed through OpenLevel options when launching a run. */
	UPROPERTY(EditAnywhere, Category="Shooter|Main Menu")
	FString GameplayGameModeOverride = TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterGameMode.BP_ShooterGameMode_C");

	/** Tag to grant the possessed pawn to flag it as the player */
	UPROPERTY(EditAnywhere, Category="Shooter|Player")
	FName PlayerPawnTag = FName("Player");

	/** Pointer to the bullet counter UI widget */
	TObjectPtr<UShooterBulletCounterUI> BulletCounterUI;
	TObjectPtr<UShooterUI> PlayerUI;
	TObjectPtr<UUserWidget> DeathMenuUI;
	TObjectPtr<UUserWidget> WinMenuUI;
	TObjectPtr<UMainMenuUI> MainMenuUI;

protected:

	/** Gameplay Initialization */
	virtual void BeginPlay() override;

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;

	/** Pawn initialization */
	virtual void OnPossess(APawn* InPawn) override;

	/** Called if the possessed pawn is destroyed */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	/** Called when the bullet count on the possessed pawn is updated */
	UFUNCTION()
	void OnBulletCountUpdated(int32 MagazineSize, int32 Bullets);

	/** Called when the possessed pawn is damaged */
	UFUNCTION()
	void OnPawnDamaged(float LifePercent);

	/** Called when interaction prompt visibility/content changes. */
	UFUNCTION()
	void OnInteractionPromptUpdated(bool bVisible, FText ObjectName, FText HintText);

	/** Returns true when this level should show the main menu overlay. */
	bool ShouldShowMainMenuOnCurrentLevel() const;

	/** Opens the main menu overlay and switches input to UI mode. */
	void ShowMainMenu();

	/** Hides the main menu overlay and restores gameplay input mode. */
	void HideMainMenu();

	/** Shared launcher for both main menu options. */
	void StartSelectedRun(EShooterRunMode SelectedMode, FName TargetLevelName);

	UFUNCTION()
	void HandleKeyEscapeSelected();

	UFUNCTION()
	void HandleSurvivalSelected();

public:

	/** Push objective text to HUD widgets. */
	void SetObjectiveText(const FText& ObjectiveText);

	/** Push quest timer to HUD widgets. */
	void SetObjectiveTimer(float RemainingTimeSeconds);

	/** Push kill count to HUD widgets. */
	void SetKillCount(int32 KillCount);

	/** Push ending result to HUD widgets. */
	void ShowEnding(const FName& EndingId, const FText& EndingText, bool bWon);

	/** Opens the death menu and switches input to UI mode. */
	UFUNCTION(BlueprintCallable, Category="Shooter|Death Menu")
	void ShowDeathMenu();

	/** Hides the death menu and restores gameplay input mode. */
	UFUNCTION(BlueprintCallable, Category="Shooter|Death Menu")
	void HideDeathMenu();

	/** Opens the win menu and switches input to UI mode. */
	UFUNCTION(BlueprintCallable, Category="Shooter|Win Menu")
	void ShowWinMenu();

	/** Hides the win menu and restores gameplay input mode. */
	UFUNCTION(BlueprintCallable, Category="Shooter|Win Menu")
	void HideWinMenu();

	/** Restarts the currently loaded map. */
	UFUNCTION(BlueprintCallable, Category="Shooter|Death Menu")
	void RestartCurrentLevel();

	/** Opens the configured main menu map. */
	UFUNCTION(BlueprintCallable, Category="Shooter|Death Menu")
	void ReturnToMainMenu();

	/** Exits the game application. */
	UFUNCTION(BlueprintCallable, Category="Shooter|Death Menu")
	void QuitToDesktop();
};
