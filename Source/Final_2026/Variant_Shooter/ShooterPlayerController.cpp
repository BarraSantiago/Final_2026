// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterBulletCounterUI.h"
#include "MainMenu/MainMenuUI.h"
#include "ShooterGameMode.h"
#include "Final_2026GameInstance.h"
#include "Final_2026.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController())
	{
		if (SVirtualJoystick::ShouldDisplayTouchInterface())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);
			}
			else
			{
				UE_LOG(LogFinal_2026, Error, TEXT("Could not spawn mobile controls widget."));
			}
		}

		// create the bullet counter widget and add it to the screen
		BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);
		
		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UShooterUI::StaticClass(), false);
		if (FoundWidgets.Num() > 0)
		{
			PlayerUI = Cast<UShooterUI>(FoundWidgets[0]);
		}
		else
		{
			UE_LOG(LogFinal_2026, Error, TEXT("Could not find ShooterUI widget in the level."));
		}

		if (BulletCounterUI)
		{
			BulletCounterUI->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(LogFinal_2026, Error, TEXT("Could not spawn bullet counter widget."));
		}

		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
		SetIgnoreMoveInput(false);
		SetIgnoreLookInput(false);

		if (const UFinal_2026GameInstance* GameInstance = Cast<UFinal_2026GameInstance>(GetGameInstance()))
		{
			if (GameInstance->GetSelectedShooterRunMode() == EShooterRunMode::None && ShouldShowMainMenuOnCurrentLevel())
			{
				ShowMainMenu();
			}
		}
	}
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Ensure gameplay input is restored when re-possessing after a respawn.
	HideDeathMenu();
	HideWinMenu();

	if (const UFinal_2026GameInstance* GameInstance = Cast<UFinal_2026GameInstance>(GetGameInstance()))
	{
		if (GameInstance->GetSelectedShooterRunMode() == EShooterRunMode::None && ShouldShowMainMenuOnCurrentLevel())
		{
			ShowMainMenu();
		}
	}

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddDynamic(this, &AShooterPlayerController::OnPawnDestroyed);

	// is this a shooter character?
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		// add the player tag
		ShooterCharacter->Tags.Add(PlayerPawnTag);

		// subscribe to the pawn's delegates
		ShooterCharacter->OnBulletCountUpdated.AddDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
		ShooterCharacter->OnDamaged.AddDynamic(this, &AShooterPlayerController::OnPawnDamaged);
		ShooterCharacter->OnInteractionPromptUpdated.AddDynamic(
			this, &AShooterPlayerController::OnInteractionPromptUpdated);

		// force update the life bar
		ShooterCharacter->OnDamaged.Broadcast(1.0f);
	}
}

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// reset the bullet counter HUD
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(0, 0);
	}

	if (const AShooterGameMode* ShooterGameMode = GetWorld()->GetAuthGameMode<AShooterGameMode>())
	{
		if (ShooterGameMode->IsQuestEnded() || !ShooterGameMode->ShouldRespawnPlayerOnDeath())
		{
			return;
		}
	}

	// find the player start
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() > 0)
	{
		if (!CharacterClass)
		{
			return;
		}

		// select a random player start
		AActor* RandomPlayerStart = ActorList[FMath::RandRange(0, ActorList.Num() - 1)];

		// spawn a character at the player start
		const FTransform SpawnTransform = RandomPlayerStart->GetActorTransform();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (AShooterCharacter* RespawnedCharacter = GetWorld()->SpawnActor<AShooterCharacter>(
			CharacterClass, SpawnTransform, SpawnParams))
		{
			// possess the character
			Possess(RespawnedCharacter);
		}
	}
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// update the UI
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

void AShooterPlayerController::OnInteractionPromptUpdated(bool bVisible, FText ObjectName, FText HintText)
{
	if (IsValid(BulletCounterUI))
	{
		PlayerUI->BP_SetInteractionPrompt(bVisible, ObjectName, HintText);
	}
}

bool AShooterPlayerController::ShouldShowMainMenuOnCurrentLevel() const
{
	// Backward-compatible fallback: old projects may still have "MainMenu" configured but no map with that name.
	if (MainMenuLevelName == FName("MainMenu"))
	{
		return true;
	}

	if (MainMenuLevelName.IsNone())
	{
		return false;
	}

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	return CurrentLevelName.Equals(MainMenuLevelName.ToString(), ESearchCase::IgnoreCase);
}

void AShooterPlayerController::ShowMainMenu()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	HideDeathMenu();
	HideWinMenu();

	if (!IsValid(MainMenuUI))
	{
		const TSubclassOf<UMainMenuUI> MenuClass =  MainMenuUIClass;// ? MainMenuUIClass : UMainMenuUI::StaticClass();
		MainMenuUI = CreateWidget<UMainMenuUI>(this, MenuClass);
	}

	if (!IsValid(MainMenuUI))
	{
		return;
	}

	MainMenuUI->OnKeyEscapeSelected.RemoveDynamic(this, &AShooterPlayerController::HandleKeyEscapeSelected);
	MainMenuUI->OnKeyEscapeSelected.AddDynamic(this, &AShooterPlayerController::HandleKeyEscapeSelected);

	MainMenuUI->OnSurvivalSelected.RemoveDynamic(this, &AShooterPlayerController::HandleSurvivalSelected);
	MainMenuUI->OnSurvivalSelected.AddDynamic(this, &AShooterPlayerController::HandleSurvivalSelected);

	if (!MainMenuUI->IsInViewport())
	{
		MainMenuUI->AddToViewport(200);
	}

	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->SetVisibility(ESlateVisibility::Collapsed);
	}

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(MainMenuUI->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	SetInputMode(InputMode);
	bShowMouseCursor = true;
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	SetPause(true);
}

void AShooterPlayerController::HideMainMenu()
{
	if (IsValid(MainMenuUI))
	{
		MainMenuUI->RemoveFromParent();
	}

	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->SetVisibility(ESlateVisibility::Visible);
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
	SetPause(false);
}

void AShooterPlayerController::StartSelectedRun(const EShooterRunMode SelectedMode, const FName TargetLevelName)
{
	if (TargetLevelName.IsNone())
	{
		return;
	}

	if (UFinal_2026GameInstance* GameInstance = Cast<UFinal_2026GameInstance>(GetGameInstance()))
	{
		GameInstance->SetSelectedShooterRunMode(SelectedMode);
	}

	HideMainMenu();
	HideDeathMenu();
	HideWinMenu();

	FString OpenLevelOptions;
	if (!GameplayGameModeOverride.IsEmpty())
	{
		OpenLevelOptions = FString::Printf(TEXT("game=%s"), *GameplayGameModeOverride);
	}

	UGameplayStatics::OpenLevel(this, TargetLevelName, true, OpenLevelOptions);
}

void AShooterPlayerController::HandleKeyEscapeSelected()
{
	StartSelectedRun(EShooterRunMode::KeyEscape, KeyEscapeLevelName);
}

void AShooterPlayerController::HandleSurvivalSelected()
{
	StartSelectedRun(EShooterRunMode::Survival, SurvivalLevelName);
}

void AShooterPlayerController::SetObjectiveText(const FText& ObjectiveText)
{
	if (IsValid(BulletCounterUI))
	{
		PlayerUI->BP_SetObjectiveText(ObjectiveText);
	}
}

void AShooterPlayerController::SetObjectiveTimer(float RemainingTimeSeconds)
{
	if (IsValid(BulletCounterUI))
	{
		PlayerUI->BP_SetObjectiveTimer(RemainingTimeSeconds);
	}
}

void AShooterPlayerController::SetKillCount(int32 KillCount)
{
	if (IsValid(BulletCounterUI))
	{
		PlayerUI->BP_SetKillCount(KillCount);
	}
}

void AShooterPlayerController::ShowEnding(const FName& EndingId, const FText& EndingText, bool bWon)
{
	if (IsValid(PlayerUI))
	{
		PlayerUI->BP_ShowEnding(EndingId, EndingText, bWon);
		UE_LOG(LogFinal_2026, Log, TEXT("Showing ending: %s - %s"), *EndingId.ToString(), *EndingText.ToString());
	}
	else
	{
		UE_LOG(LogFinal_2026, Error, TEXT("PlayerUI is not valid. Cannot show ending."));
	}
}

void AShooterPlayerController::ShowDeathMenu()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	HideWinMenu();

	if (!IsValid(DeathMenuUI) && DeathMenuUIClass)
	{
		DeathMenuUI = CreateWidget<UUserWidget>(this, DeathMenuUIClass);
	}

	if (IsValid(DeathMenuUI) && !DeathMenuUI->IsInViewport())
	{
		DeathMenuUI->AddToViewport(100);
	}

	FInputModeUIOnly InputMode;
	if (IsValid(DeathMenuUI))
	{
		InputMode.SetWidgetToFocus(DeathMenuUI->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	SetInputMode(InputMode);
	bShowMouseCursor = true;
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	SetPause(true);
}

void AShooterPlayerController::HideDeathMenu()
{
	if (IsValid(DeathMenuUI))
	{
		DeathMenuUI->RemoveFromParent();
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
	SetPause(false);
}

void AShooterPlayerController::ShowWinMenu()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	HideDeathMenu();

	if (!IsValid(WinMenuUI) && WinMenuUIClass)
	{
		WinMenuUI = CreateWidget<UUserWidget>(this, WinMenuUIClass);
	}

	if (IsValid(WinMenuUI) && !WinMenuUI->IsInViewport())
	{
		WinMenuUI->AddToViewport(110);
	}

	FInputModeUIOnly InputMode;
	if (IsValid(WinMenuUI))
	{
		InputMode.SetWidgetToFocus(WinMenuUI->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	SetInputMode(InputMode);
	bShowMouseCursor = true;
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	SetPause(true);
}

void AShooterPlayerController::HideWinMenu()
{
	if (IsValid(WinMenuUI))
	{
		WinMenuUI->RemoveFromParent();
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
	SetPause(false);
}

void AShooterPlayerController::RestartCurrentLevel()
{
	HideDeathMenu();
	HideWinMenu();

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (!CurrentLevelName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName), true);
	}
}

void AShooterPlayerController::ReturnToMainMenu()
{
	if (UFinal_2026GameInstance* GameInstance = Cast<UFinal_2026GameInstance>(GetGameInstance()))
	{
		GameInstance->SetSelectedShooterRunMode(EShooterRunMode::None);
	}

	HideMainMenu();
	HideDeathMenu();
	HideWinMenu();

	FName TargetMenuLevel = MainMenuLevelName;
	if (TargetMenuLevel.IsNone() || TargetMenuLevel == FName("MainMenu"))
	{
		TargetMenuLevel = KeyEscapeLevelName;
	}

	if (TargetMenuLevel.IsNone())
	{
		UE_LOG(LogFinal_2026, Warning, TEXT("No valid level configured for ReturnToMainMenu."));
		return;
	}

	UGameplayStatics::OpenLevel(this, TargetMenuLevel, true);
}

void AShooterPlayerController::QuitToDesktop()
{
	HideDeathMenu();
	HideWinMenu();
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}
