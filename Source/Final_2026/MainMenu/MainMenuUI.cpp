// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenu/MainMenuUI.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UMainMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	BuildRuntimeMenu();

	if (IsValid(KeyEscapeButton))
	{
		KeyEscapeButton->OnClicked.RemoveDynamic(this, &UMainMenuUI::HandleKeyEscapeClicked);
		KeyEscapeButton->OnClicked.AddDynamic(this, &UMainMenuUI::HandleKeyEscapeClicked);
	}

	if (IsValid(SurvivalButton))
	{
		SurvivalButton->OnClicked.RemoveDynamic(this, &UMainMenuUI::HandleSurvivalClicked);
		SurvivalButton->OnClicked.AddDynamic(this, &UMainMenuUI::HandleSurvivalClicked);
	}
}

void UMainMenuUI::HandleKeyEscapeClicked()
{
	OnKeyEscapeSelected.Broadcast();
}

void UMainMenuUI::HandleSurvivalClicked()
{
	OnSurvivalSelected.Broadcast();
}

void UMainMenuUI::BuildRuntimeMenu()
{
	if (!WidgetTree || (IsValid(KeyEscapeButton) && IsValid(SurvivalButton)))
	{
		return;
	}

	UCanvasPanel* RootPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootPanel"));
	WidgetTree->RootWidget = RootPanel;

	UVerticalBox* MenuBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MenuBox"));
	if (UCanvasPanelSlot* MenuCanvasSlot = RootPanel->AddChildToCanvas(MenuBox))
	{
		MenuCanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		MenuCanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		MenuCanvasSlot->SetAutoSize(true);
	}

	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetText(FText::FromString(TEXT("FINAL 2026")));
	TitleText->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* TitleSlot = MenuBox->AddChildToVerticalBox(TitleText))
	{
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 24.0f));
		TitleSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	KeyEscapeButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("KeyEscapeButton"));
	UTextBlock* KeyEscapeLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("KeyEscapeLabel"));
	KeyEscapeLabel->SetText(FText::FromString(TEXT("Grab Key + Escape")));
	KeyEscapeLabel->SetJustification(ETextJustify::Center);
	if (UButtonSlot* KeySlot = Cast<UButtonSlot>(KeyEscapeButton->AddChild(KeyEscapeLabel)))
	{
		KeySlot->SetHorizontalAlignment(HAlign_Center);
		KeySlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UVerticalBoxSlot* KeyRow = MenuBox->AddChildToVerticalBox(KeyEscapeButton))
	{
		KeyRow->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
		KeyRow->SetHorizontalAlignment(HAlign_Fill);
	}

	SurvivalButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SurvivalButton"));
	UTextBlock* SurvivalLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SurvivalLabel"));
	SurvivalLabel->SetText(FText::FromString(TEXT("Survival")));
	SurvivalLabel->SetJustification(ETextJustify::Center);
	if (UButtonSlot* SurvivalSlot = Cast<UButtonSlot>(SurvivalButton->AddChild(SurvivalLabel)))
	{
		SurvivalSlot->SetHorizontalAlignment(HAlign_Center);
		SurvivalSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UVerticalBoxSlot* SurvivalRow = MenuBox->AddChildToVerticalBox(SurvivalButton))
	{
		SurvivalRow->SetHorizontalAlignment(HAlign_Fill);
	}
}
