// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenu/MainMenuUI.h"

void UMainMenuUI::NotifyKeyEscapeSelected()
{
	OnKeyEscapeSelected.Broadcast();
}

void UMainMenuUI::NotifySurvivalSelected()
{
	OnSurvivalSelected.Broadcast();
}

void UMainMenuUI::NotifyOptionsSelected()
{
	OnOptionsSelected.Broadcast();
}

void UMainMenuUI::NotifyQuitSelected()
{
	OnQuitSelected.Broadcast();
}
