// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenu/MainMenuGameMode.h"
#include "MainMenu/MainMenuPlayerController.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	PlayerControllerClass = AMainMenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
	bStartPlayersAsSpectators = true;
}
