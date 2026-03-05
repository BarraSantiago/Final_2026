// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenu/MainMenuPlayerController.h"

#include "Final_2026.h"
#include "Final_2026GameInstance.h"
#include "MainMenu/MainMenuUI.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

namespace
{
FString BuildMainMenuGameModeTravelOption(const FString& RawGameModePath)
{
	FString Path = RawGameModePath.TrimStartAndEnd();
	if (Path.IsEmpty())
	{
		return FString();
	}

	if (Path.StartsWith(TEXT("Game="), ESearchCase::IgnoreCase))
	{
		Path.RightChopInline(5, EAllowShrinking::No);
	}

	if (Path.StartsWith(TEXT("Class'")) && Path.EndsWith(TEXT("'")))
	{
		Path = Path.Mid(6, Path.Len() - 7);
	}

	// Recover from accidental duplicated content paths, e.g. "/Game/.../Game/...".
	if (Path.StartsWith(TEXT("/Game/")))
	{
		const int32 DuplicateIdx = Path.Find(TEXT("/Game/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, 1);
		if (DuplicateIdx != INDEX_NONE)
		{
			Path.RightChopInline(DuplicateIdx, EAllowShrinking::No);
		}

		if (Path.Contains(TEXT(".")) && !Path.EndsWith(TEXT("_C")))
		{
			Path.Append(TEXT("_C"));
		}
	}

	return FString::Printf(TEXT("Game=%s"), *Path);
}
}

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController() || !MainMenuUIClass)
	{
		return;
	}

	MainMenuUI = CreateWidget<UMainMenuUI>(this, MainMenuUIClass);
	UE_LOG(LogFinal_2026, Log, TEXT("MainMenuPlayerController created MainMenuUI: %s"), IsValid(MainMenuUI) ? *MainMenuUI->GetName() : TEXT("None"));
	if (!IsValid(MainMenuUI))
	{
		return;
	}

	MainMenuUI->OnKeyEscapeSelected.RemoveDynamic(this, &AMainMenuPlayerController::HandleKeyEscapeSelected);
	MainMenuUI->OnKeyEscapeSelected.AddDynamic(this, &AMainMenuPlayerController::HandleKeyEscapeSelected);

	MainMenuUI->OnSurvivalSelected.RemoveDynamic(this, &AMainMenuPlayerController::HandleSurvivalSelected);
	MainMenuUI->OnSurvivalSelected.AddDynamic(this, &AMainMenuPlayerController::HandleSurvivalSelected);

	MainMenuUI->OnOptionsSelected.RemoveDynamic(this, &AMainMenuPlayerController::HandleOptionsSelected);
	MainMenuUI->OnOptionsSelected.AddDynamic(this, &AMainMenuPlayerController::HandleOptionsSelected);

	MainMenuUI->OnQuitSelected.RemoveDynamic(this, &AMainMenuPlayerController::HandleQuitSelected);
	MainMenuUI->OnQuitSelected.AddDynamic(this, &AMainMenuPlayerController::HandleQuitSelected);

	MainMenuUI->AddToViewport(100);

	MainMenuUI->SetIsFocusable(true);

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	SetInputMode(InputMode);
	bShowMouseCursor = true;
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	SetPause(true);
}

void AMainMenuPlayerController::LaunchSelectedRun(const EShooterRunMode SelectedMode, const FName TargetLevelName)
{
	if (TargetLevelName.IsNone())
	{
		return;
	}

	if (UFinal_2026GameInstance* GameInstance = Cast<UFinal_2026GameInstance>(GetGameInstance()))
	{
		GameInstance->SetSelectedShooterRunMode(SelectedMode);
	}

	if (IsValid(MainMenuUI))
	{
		MainMenuUI->RemoveFromParent();
		MainMenuUI = nullptr;
	}
	//UWidgetBlueprintLibrary::RemoveAllWidgets(this);

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
	SetPause(false);

	const FString OpenLevelOptions = BuildMainMenuGameModeTravelOption(GameplayGameModeOverride);

	UGameplayStatics::OpenLevel(this, TargetLevelName, true, OpenLevelOptions);
}

void AMainMenuPlayerController::HandleKeyEscapeSelected()
{
	LaunchSelectedRun(EShooterRunMode::KeyEscape, KeyEscapeLevelName);
}

void AMainMenuPlayerController::HandleSurvivalSelected()
{
	LaunchSelectedRun(EShooterRunMode::Survival, SurvivalLevelName);
}

void AMainMenuPlayerController::HandleOptionsSelected()
{
	// Options menu behavior is fully controlled in Blueprint UI.
}

void AMainMenuPlayerController::HandleQuitSelected()
{
	SetPause(false);
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}
