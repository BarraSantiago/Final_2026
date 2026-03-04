// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterGameMode.h"
#include "AI/ShooterNPC.h"
#include "ShooterUI.h"
#include "ShooterPlayerController.h"
#include "Interaction/ShooterObjectiveDoor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"

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
		GetWorld()->GetTimerManager().SetTimer(SurvivalTimerHandle, this, &AShooterGameMode::UpdateSurvivalCountdown,
		                                       0.1f, true);
	}

	UpdateObjectiveText();

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(PlayerController))
	{
		ShooterPC->SetKillCount(PlayerKillCount);
	}

	InitializeWaveSystem();
}

void AShooterGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(SurvivalTimerHandle);
	StopWaveSystem();
	Super::EndPlay(EndPlayReason);
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
	if (!bQuestEnded && bEnableWaveSystem)
	{
		AliveEnemyCount = FMath::Max(0, AliveEnemyCount - 1);
		TryScheduleNextWave();
	}

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

	// Respawn mode keeps run active. Game-over mode opens death menu.
	if (bRespawnPlayerOnDeath)
	{
		return;
	}

	bQuestEnded = true;
	GetWorld()->GetTimerManager().ClearTimer(SurvivalTimerHandle);
	StopWaveSystem();

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		ShooterPC->SetObjectiveText(FText::FromString(TEXT("You died.")));
		ShooterPC->ShowDeathMenu();
	}
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
		ShooterPC->SetObjectiveText(
			FText::FromString(TEXT("Door unlocked. Escape now or keep fighting for a better ending.")));
		return;
	}

	if (DoorUnlockMode == EShooterDoorUnlockMode::KeyPickup)
	{
		const FText ObjectiveText = FText::Format(
			FText::FromString(
				TEXT("Find Key_asset_0001 to unlock the Unassuming Wooden Door of Ultimate Destiny. ({0}/{1})")),
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

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)))
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

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		ShooterPC->SetObjectiveTimer(0.0f);
	}

	UpdateObjectiveText();
}

void AShooterGameMode::EndRun(const FName& EndingId, const FText& EndingText, bool bWon)
{
	if (bQuestEnded)
	{
		UE_LOG(LogTemp, Warning, TEXT("B quest unfinished."));
		return;
	}

	bQuestEnded = true;
	GetWorld()->GetTimerManager().ClearTimer(SurvivalTimerHandle);
	StopWaveSystem();

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)))
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

void AShooterGameMode::InitializeWaveSystem()
{
	if (!bEnableWaveSystem)
	{
		return;
	}

	EnemySpawnPoints.Reset();
	EnemySpawnPointTransforms.Reset();
	WaveEnemyControllerClass = nullptr;

	TArray<AActor*> FoundSpawnPoints;
	if (!EnemySpawnPointTag.IsNone())
	{
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), EnemySpawnPointTag, FoundSpawnPoints);
	}

	// Fallback to PlayerStart actors so wave system can run even without tagged points.
	if (FoundSpawnPoints.Num() == 0)
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundSpawnPoints);
	}

	for (AActor* SpawnPoint : FoundSpawnPoints)
	{
		if (IsValid(SpawnPoint))
		{
			EnemySpawnPoints.Add(SpawnPoint);
			EnemySpawnPointTransforms.Add(SpawnPoint->GetActorTransform());
		}
	}

	TArray<AActor*> ExistingEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShooterNPC::StaticClass(), ExistingEnemies);
	AliveEnemyCount = ExistingEnemies.Num();

	// If no explicit spawn points were tagged, reuse current enemy locations.
	// This keeps wave spawns in AI-valid areas in levels that start with hand-placed enemies.
	if (FoundSpawnPoints.Num() == 0 && ExistingEnemies.Num() > 0)
	{
		EnemySpawnPoints.Reset();
		EnemySpawnPointTransforms.Reset();

		for (AActor* ExistingEnemyActor : ExistingEnemies)
		{
			if (IsValid(ExistingEnemyActor))
			{
				EnemySpawnPoints.Add(ExistingEnemyActor);
				EnemySpawnPointTransforms.Add(ExistingEnemyActor->GetActorTransform());
			}
		}
	}

	if (AliveEnemyCount > 0)
	{
		// Reuse class/controller from already-working placed enemies.
		if (AShooterNPC* ExistingEnemy = Cast<AShooterNPC>(ExistingEnemies[0]))
		{
			WaveEnemyClass = ExistingEnemy->GetClass();

			if (AController* ExistingController = ExistingEnemy->GetController())
			{
				WaveEnemyControllerClass = ExistingController->GetClass();
			}
		}

		CurrentWave = 1;
		TargetEnemiesThisWave = AliveEnemyCount;
		SpawnedEnemiesThisWave = TargetEnemiesThisWave;
		return;
	}

	if (!WaveEnemyClass || EnemySpawnPointTransforms.Num() == 0)
	{
		return;
	}

	if (FirstWaveDelay <= 0.0f)
	{
		StartNextWave();
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(NextWaveTimerHandle, this, &AShooterGameMode::StartNextWave, FirstWaveDelay,
	                                       false);
}

void AShooterGameMode::StartNextWave()
{
	if (bQuestEnded || !bEnableWaveSystem || !WaveEnemyClass || EnemySpawnPointTransforms.Num() == 0)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(NextWaveTimerHandle);

	++CurrentWave;
	TargetEnemiesThisWave = FMath::Max(1, InitialEnemiesPerWave + ((CurrentWave - 1) * EnemiesAddedPerWave));
	SpawnedEnemiesThisWave = 0;
	bIsSpawningWave = true;

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		ShooterPC->SetObjectiveText(FText::Format(
			FText::FromString(TEXT("Wave {0} started.")),
			FText::AsNumber(CurrentWave)));
	}

	if (TimeBetweenEnemySpawns <= 0.0f)
	{
		while (SpawnedEnemiesThisWave < TargetEnemiesThisWave)
		{
			SpawnOneWaveEnemy();
		}

		bIsSpawningWave = false;
		return;
	}

	SpawnOneWaveEnemy();

	if (SpawnedEnemiesThisWave < TargetEnemiesThisWave)
	{
		GetWorld()->GetTimerManager().SetTimer(SpawnEnemyTimerHandle, this, &AShooterGameMode::SpawnOneWaveEnemy,
		                                       TimeBetweenEnemySpawns, true);
	}
	else
	{
		bIsSpawningWave = false;
	}
}

void AShooterGameMode::SpawnOneWaveEnemy()
{
	if (bQuestEnded || !bEnableWaveSystem || !WaveEnemyClass || EnemySpawnPointTransforms.Num() == 0)
	{
		StopWaveSystem();
		return;
	}

	if (SpawnedEnemiesThisWave >= TargetEnemiesThisWave)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnEnemyTimerHandle);
		bIsSpawningWave = false;
		return;
	}

	const int32 SpawnPointIdx = FMath::RandRange(0, EnemySpawnPointTransforms.Num() - 1);
	const FTransform SpawnTransform = EnemySpawnPointTransforms[SpawnPointIdx];


	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (AShooterNPC* SpawnedEnemy = GetWorld()->SpawnActor<AShooterNPC>(WaveEnemyClass, SpawnTransform, SpawnParams))
	{
		if (WaveEnemyControllerClass)
		{
			FActorSpawnParameters ControllerSpawnParams;
			ControllerSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (AController* NewController = GetWorld()->SpawnActor<AController>(
				WaveEnemyControllerClass,
				SpawnedEnemy->GetActorLocation(),
				SpawnedEnemy->GetActorRotation(),
				ControllerSpawnParams))
			{
				NewController->Possess(SpawnedEnemy);
			}
		}
		else
		{
			// Only fallback if no explicit class was captured
			SpawnedEnemy->SpawnDefaultController();
		}

		++AliveEnemyCount;
	}


	++SpawnedEnemiesThisWave;

	if (SpawnedEnemiesThisWave >= TargetEnemiesThisWave)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnEnemyTimerHandle);
		bIsSpawningWave = false;
	}
}

void AShooterGameMode::TryScheduleNextWave()
{
	if (bQuestEnded || !bEnableWaveSystem || bIsSpawningWave || CurrentWave <= 0)
	{
		return;
	}

	if (!WaveEnemyClass || EnemySpawnPointTransforms.Num() == 0)
	{
		return;
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(NextWaveTimerHandle))
	{
		return;
	}

	const int32 AliveThreshold = FMath::FloorToInt(
		static_cast<float>(TargetEnemiesThisWave) * NextWaveAliveRatioThreshold);
	if (AliveEnemyCount > AliveThreshold)
	{
		return;
	}

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		ShooterPC->SetObjectiveText(FText::Format(
			FText::FromString(TEXT("Wave {0} almost cleared. Next wave in {1}s.")),
			FText::AsNumber(CurrentWave),
			FText::AsNumber(FMath::RoundToInt(TimeBetweenWaves))));
	}

	if (TimeBetweenWaves <= 0.0f)
	{
		StartNextWave();
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(NextWaveTimerHandle, this, &AShooterGameMode::StartNextWave,
	                                       TimeBetweenWaves, false);
}

void AShooterGameMode::StopWaveSystem()
{
	GetWorld()->GetTimerManager().ClearTimer(NextWaveTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(SpawnEnemyTimerHandle);
	bIsSpawningWave = false;
}
