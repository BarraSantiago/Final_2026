#include "Variant_Shooter/ShooterGameMode.h"
#include "Final_2026.h"
#include "AI/ShooterNPC.h"
#include "ShooterUI.h"
#include "ShooterPlayerController.h"
#include "Final_2026GameInstance.h"
#include "Interaction/ShooterObjectiveDoor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (const UFinal_2026GameInstance* GameInstance = Cast<UFinal_2026GameInstance>(GetGameInstance()))
	{
		switch (GameInstance->GetSelectedShooterRunMode())
		{
		case EShooterRunMode::KeyEscape:
			DoorUnlockMode = EShooterDoorUnlockMode::KeyPickup;
			break;
		case EShooterRunMode::Survival:
			DoorUnlockMode = EShooterDoorUnlockMode::SurvivalTimer;
			break;
		default:
			break;
		}
	}

	UE_LOG(LogFinal_2026, Log, TEXT("ShooterGameMode BeginPlay. UnlockMode=%s RequiredKeyCount=%d"),
	       DoorUnlockMode == EShooterDoorUnlockMode::KeyPickup ? TEXT("KeyPickup") : TEXT("SurvivalTimer"),
	       RequiredKeyCount);

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
	UpdateObjectiveScoreUI();

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
	UpdateObjectiveScoreUI();
}

void AShooterGameMode::RegisterObjectiveDoor(AShooterObjectiveDoor* InDoor)
{
	ObjectiveDoor = InDoor;

	UE_LOG(LogFinal_2026, Log, TEXT("Objective door registered: %s (AlreadyUnlocked=%s)"),
	       IsValid(ObjectiveDoor) ? *ObjectiveDoor->GetName() : TEXT("None"),
	       bDoorUnlocked ? TEXT("true") : TEXT("false"));

	if (IsValid(ObjectiveDoor))
	{
		ObjectiveDoor->SetDoorUnlocked(bDoorUnlocked);
	}
}

void AShooterGameMode::NotifyObjectiveKeyCollected()
{
	if (bQuestEnded || DoorUnlockMode != EShooterDoorUnlockMode::KeyPickup)
	{
		UE_LOG(LogFinal_2026, Warning,
		       TEXT("NotifyObjectiveKeyCollected ignored. QuestEnded=%s UnlockMode=%s"),
		       bQuestEnded ? TEXT("true") : TEXT("false"),
		       DoorUnlockMode == EShooterDoorUnlockMode::KeyPickup ? TEXT("KeyPickup") : TEXT("SurvivalTimer"));
		return;
	}

	++CurrentKeyCount;
	UE_LOG(LogFinal_2026, Log, TEXT("Objective key collected: %d/%d"), CurrentKeyCount, RequiredKeyCount);

	if (CurrentKeyCount >= RequiredKeyCount)
	{
		UnlockDoor();
	}
	else
	{
		UpdateObjectiveScoreUI();
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

	UpdateObjectiveScoreUI();
	UpdateObjectiveText();
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

void AShooterGameMode::RequestEscape(APawn* EscapingPawn)
{
	if (bQuestEnded || !bDoorUnlocked)
	{
		UE_LOG(LogFinal_2026, Warning, TEXT("RequestEscape denied. QuestEnded=%s DoorUnlocked=%s Pawn=%s"),
		       bQuestEnded ? TEXT("true") : TEXT("false"),
		       bDoorUnlocked ? TEXT("true") : TEXT("false"),
		       *GetNameSafe(EscapingPawn));
		return;
	}

	UE_LOG(LogFinal_2026, Log, TEXT("RequestEscape accepted. Pawn=%s Kills=%d"),
	       *GetNameSafe(EscapingPawn), PlayerKillCount);

	const FName EndingId = ResolveEndingByKillCount();
	EndRun(EndingId, BuildEndingDescription(EndingId), true);
}

int32 AShooterGameMode::ResolveNextKillObjective() const
{
	TArray<int32> KillTargets;
	KillTargets.Add(FMath::Max(1, FinalAMaxKills + 1));
	KillTargets.Add(FMath::Max(1, FinalBMaxKills + 1));
	KillTargets.Add(FMath::Max(1, FinalCMinKills));
	KillTargets.Sort();

	for (const int32 KillTarget : KillTargets)
	{
		if (PlayerKillCount < KillTarget)
		{
			return KillTarget;
		}
	}

	return PlayerKillCount;
}

void AShooterGameMode::UpdateObjectiveScoreUI()
{
	if (!IsValid(ShooterUI))
	{
		return;
	}

	if (DoorUnlockMode == EShooterDoorUnlockMode::KeyPickup)
	{
		// Reuse score slots to show key objective progress instead of combat score.
		ShooterUI->BP_UpdateScore(0, CurrentKeyCount);
		ShooterUI->BP_UpdateScore(1, RequiredKeyCount);
		return;
	}

	ShooterUI->BP_UpdateScore(0, PlayerKillCount);
	ShooterUI->BP_UpdateScore(1, ResolveNextKillObjective());
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
		ShooterPC->SetObjectiveText(FText::FromString(TEXT("Find the door.")));
		return;
	}

	if (DoorUnlockMode == EShooterDoorUnlockMode::KeyPickup)
	{
		FText ObjectiveText = FText::FromString(TEXT("Grab the key."));
		if (RequiredKeyCount > 1)
		{
			ObjectiveText = FText::Format(
				FText::FromString(TEXT("Grab the key. ({0}/{1})")),
				FText::AsNumber(CurrentKeyCount),
				FText::AsNumber(RequiredKeyCount));
		}

		ShooterPC->SetObjectiveText(ObjectiveText);
		return;
	}

	const int32 NextKillObjective = ResolveNextKillObjective();
	FText ObjectiveText;
	if (NextKillObjective > PlayerKillCount)
	{
		ObjectiveText = FText::Format(
			FText::FromString(TEXT("Survive. Kills: {0}. Next objective: {1} kills.")),
			FText::AsNumber(PlayerKillCount),
			FText::AsNumber(NextKillObjective));
	}
	else
	{
		ObjectiveText = FText::Format(
			FText::FromString(TEXT("Survive. Kills: {0}. Final kill objective reached.")),
			FText::AsNumber(PlayerKillCount));
	}

	ShooterPC->SetObjectiveText(ObjectiveText);
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
		UE_LOG(LogFinal_2026, Warning, TEXT("UnlockDoor ignored. DoorUnlocked=%s QuestEnded=%s"),
		       bDoorUnlocked ? TEXT("true") : TEXT("false"),
		       bQuestEnded ? TEXT("true") : TEXT("false"));
		return;
	}

	bDoorUnlocked = true;
	UE_LOG(LogFinal_2026, Log, TEXT("Objective door unlocked. RegisteredDoor=%s"),
	       IsValid(ObjectiveDoor) ? *ObjectiveDoor->GetName() : TEXT("None"));
	if (IsValid(ObjectiveDoor))
	{
		ObjectiveDoor->SetDoorUnlocked(true);
	}

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		ShooterPC->SetObjectiveTimer(0.0f);
	}

	UpdateObjectiveScoreUI();
	UpdateObjectiveText();
}

void AShooterGameMode::EndRun(const FName& EndingId, const FText& EndingText, bool bWon)
{
	if (bQuestEnded)
	{
		UE_LOG(LogFinal_2026, Warning, TEXT("EndRun ignored because quest already ended."));
		return;
	}

	UE_LOG(LogFinal_2026, Log, TEXT("EndRun called. Ending=%s Won=%s Kills=%d"), *EndingId.ToString(),
	       bWon ? TEXT("true") : TEXT("false"), PlayerKillCount);

	bQuestEnded = true;
	GetWorld()->GetTimerManager().ClearTimer(SurvivalTimerHandle);
	StopWaveSystem();

	if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		ShooterPC->SetObjectiveText(FText::FromString(TEXT("Run finished.")));
		ShooterPC->ShowEnding(EndingId, EndingText, bWon);

		if (bWon)
		{
			ShooterPC->ShowWinMenu();
		}
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

	UpdateObjectiveText();

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
		UE_LOG(LogFinal_2026, Log, TEXT("Wave enemy spawned: %s Controller=%s AIControllerClass=%s"),
		       *SpawnedEnemy->GetName(),
		       *GetNameSafe(SpawnedEnemy->GetController()),
		       *GetNameSafe(SpawnedEnemy->AIControllerClass));
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

	UpdateObjectiveText();

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
