// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterGameMode.h"
#include "ShooterUI.h"
#include "ShooterPlayerController.h"
#include "Interaction/ShooterObjectiveDoor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	// create the UI
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController && ShooterUIClass)
	{
		ShooterUI = CreateWidget<UShooterUI>(PlayerController, ShooterUIClass);
		if (ShooterUI)
		{
			ShooterUI->AddToViewport(0);
		}
	}

	if (DoorUnlockMode == EShooterDoorUnlockMode::SurvivalTimer)
	{
		SurvivalUnlockTimestamp = GetWorld()->GetTimeSeconds() + SurvivalUnlockTime;
		GetWorld()->GetTimerManager().SetTimer(SurvivalTimerHandle, this, &AShooterGameMode::UpdateSurvivalCountdown, 0.1f, true);
	}

	UpdateObjectiveText();

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(PlayerController))
	{
		ShooterPC->SetKillCount(PlayerKillCount);
	}
}

void AShooterGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(SurvivalTimerHandle);
}

void AShooterGameMode::IncrementTeamScore(uint8 TeamByte)
{
	// retrieve the team score if any
	int32 Score = 0;
	if (int32* FoundScore = TeamScores.Find(TeamByte))
	{
		Score = *FoundScore;
	}

	// increment the score for the given team
	++Score;
	TeamScores.Add(TeamByte, Score);

	// update the UI
	if (ShooterUI)
	{
		ShooterUI->BP_UpdateScore(TeamByte, Score);
	}
}

void AShooterGameMode::RegisterObjectiveDoor(AShooterObjectiveDoor* InDoor)
{
	ObjectiveDoor = InDoor;

	if (IsValid(ObjectiveDoor))
	{
		ObjectiveDoor->SetDoorUnlocked(bDoorUnlocked);
	}
}

void AShooterGameMode::NotifyObjectiveKeyCollected()
{
	if (bQuestEnded || DoorUnlockMode != EShooterDoorUnlockMode::KeyPickup)
	{
		return;
	}

	++CurrentKeyCount;

	if (CurrentKeyCount >= RequiredKeyCount)
	{
		UnlockDoor();
	}
	else
	{
		UpdateObjectiveText();
	}
}

void AShooterGameMode::NotifyEnemyKilled(AController* KillerController)
{
	if (bQuestEnded || !IsValid(KillerController) || !KillerController->IsPlayerController())
	{
		return;
	}

	++PlayerKillCount;

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(KillerController))
	{
		ShooterPC->SetKillCount(PlayerKillCount);
	}
}

void AShooterGameMode::NotifyPlayerDied()
{
	if (bQuestEnded)
	{
		return;
	}

	EndRun(FName("FinalZ"), FText::FromString(TEXT("Final Z - Garbage Collected: Bob was eliminated before escaping.")), false);
}

void AShooterGameMode::RequestEscape(AShooterCharacter* EscapingCharacter)
{
	if (bQuestEnded || !bDoorUnlocked) //|| !IsValid(EscapingCharacter))
	{
		return;
	}

	const FName EndingId = ResolveEndingByKillCount();
	EndRun(EndingId, BuildEndingDescription(EndingId), true);
}

void AShooterGameMode::UpdateObjectiveText()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(PlayerController);
	if (!ShooterPC)
	{
		return;
	}

	if (bQuestEnded)
	{
		return;
	}

	if (bDoorUnlocked)
	{
		ShooterPC->SetObjectiveText(FText::FromString(TEXT("Door unlocked. Escape now or keep fighting for a better ending.")));
		return;
	}

	if (DoorUnlockMode == EShooterDoorUnlockMode::KeyPickup)
	{
		const FText ObjectiveText = FText::Format(
			FText::FromString(TEXT("Find Key_asset_0001 to unlock the Unassuming Wooden Door of Ultimate Destiny. ({0}/{1})")),
			FText::AsNumber(CurrentKeyCount),
			FText::AsNumber(RequiredKeyCount));
		ShooterPC->SetObjectiveText(ObjectiveText);
		return;
	}

	ShooterPC->SetObjectiveText(FText::FromString(TEXT("Survive until the door unlocks.")));
}

void AShooterGameMode::UpdateSurvivalCountdown()
{
	if (bQuestEnded || DoorUnlockMode != EShooterDoorUnlockMode::SurvivalTimer || bDoorUnlocked)
	{
		GetWorld()->GetTimerManager().ClearTimer(SurvivalTimerHandle);
		return;
	}

	const float RemainingTime = FMath::Max(0.0f, SurvivalUnlockTimestamp - GetWorld()->GetTimeSeconds());

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		ShooterPC->SetObjectiveTimer(RemainingTime);
	}

	if (RemainingTime <= 0.0f)
	{
		UnlockDoor();
		GetWorld()->GetTimerManager().ClearTimer(SurvivalTimerHandle);
	}
}

void AShooterGameMode::UnlockDoor()
{
	if (bDoorUnlocked || bQuestEnded)
	{
		return;
	}

	bDoorUnlocked = true;
	if (IsValid(ObjectiveDoor))
	{
		ObjectiveDoor->SetDoorUnlocked(true);
	}

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		ShooterPC->SetObjectiveTimer(0.0f);
	}

	UpdateObjectiveText();
}

void AShooterGameMode::EndRun(const FName& EndingId, const FText& EndingText, bool bWon)
{
	if (bQuestEnded)
	{
		return;
	}

	bQuestEnded = true;
	GetWorld()->GetTimerManager().ClearTimer(SurvivalTimerHandle);

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		ShooterPC->SetObjectiveText(FText::FromString(TEXT("Run finished.")));
		ShooterPC->ShowEnding(EndingId, EndingText, bWon);
	}
}

FName AShooterGameMode::ResolveEndingByKillCount() const
{
	if (PlayerKillCount <= FinalAMaxKills)
	{
		return FName("FinalA");
	}

	// Keep Final B as the default middle range, including the 51-99 gap.
	if (PlayerKillCount <= FinalBMaxKills || PlayerKillCount < FinalCMinKills)
	{
		return FName("FinalB");
	}

	return FName("FinalC");
}

FText AShooterGameMode::BuildEndingDescription(const FName& EndingId) const
{
	if (EndingId == FName("FinalA"))
	{
		return FText::Format(
			FText::FromString(TEXT("Final A - Fish Particle: escaped with {0} kills.")),
			FText::AsNumber(PlayerKillCount));
	}

	if (EndingId == FName("FinalB"))
	{
		return FText::Format(
			FText::FromString(TEXT("Final B - Random NPC: escaped with {0} kills.")),
			FText::AsNumber(PlayerKillCount));
	}

	return FText::Format(
		FText::FromString(TEXT("Final C - Main Character: escaped with {0} kills.")),
		FText::AsNumber(PlayerKillCount));
}
