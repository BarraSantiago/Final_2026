// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

class UShooterUI;
class AShooterObjectiveDoor;
class AShooterNPC;
class AController;
class AActor;
class APawn;

UENUM(BlueprintType)
enum class EShooterDoorUnlockMode : uint8
{
	KeyPickup UMETA(DisplayName = "Key Pickup"),
	SurvivalTimer UMETA(DisplayName = "Survival Timer")
};

/**
 *  Shooter game mode with quest door objective and ending logic.
 */
UCLASS(abstract)
class FINAL_2026_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	/** Type of UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Shooter")
	TSubclassOf<UShooterUI> ShooterUIClass;

	/** Pointer to the UI widget */
	TObjectPtr<UShooterUI> ShooterUI;

	/** Map of scores by team ID */
	TMap<uint8, int32> TeamScores;

	/** Door unlock condition type. */
	UPROPERTY(EditAnywhere, Category="Quest")
	EShooterDoorUnlockMode DoorUnlockMode = EShooterDoorUnlockMode::KeyPickup;

	/** Time to survive before door unlocks when using SurvivalTimer mode. */
	UPROPERTY(EditAnywhere, Category="Quest", meta = (ClampMin = 1.0, Units = "s"))
	float SurvivalUnlockTime = 120.0f;

	/** Number of keys required to unlock the door in KeyPickup mode. */
	UPROPERTY(EditAnywhere, Category="Quest", meta = (ClampMin = 1))
	int32 RequiredKeyCount = 1;

	/** Maximum kills for Final A. */
	UPROPERTY(EditAnywhere, Category="Quest|Endings", meta = (ClampMin = 0))
	int32 FinalAMaxKills = 19;

	/** Maximum kills for Final B. */
	UPROPERTY(EditAnywhere, Category="Quest|Endings", meta = (ClampMin = 0))
	int32 FinalBMaxKills = 50;

	/** Minimum kills required for Final C. */
	UPROPERTY(EditAnywhere, Category="Quest|Endings", meta = (ClampMin = 0))
	int32 FinalCMinKills = 100;

	/** If true, a death respawns the player. If false, death opens game over menu. */
	UPROPERTY(EditAnywhere, Category="Shooter|Player Death")
	bool bRespawnPlayerOnDeath = false;

	/** Enables wave spawning for AI enemies. */
	UPROPERTY(EditAnywhere, Category="Waves")
	bool bEnableWaveSystem = true;

	/** Enemy class used by the wave spawner. */
	UPROPERTY(EditAnywhere, Category="Waves", meta=(EditCondition="bEnableWaveSystem"))
	TSubclassOf<AShooterNPC> WaveEnemyClass;

	/** Actors with this gameplay tag are used as enemy spawn points. */
	UPROPERTY(EditAnywhere, Category="Waves", meta=(EditCondition="bEnableWaveSystem"))
	FName EnemySpawnPointTag = FName("EnemySpawn");

	/** Delay before first wave starts. */
	UPROPERTY(EditAnywhere, Category="Waves", meta=(EditCondition="bEnableWaveSystem", ClampMin = 0.0, Units = "s"))
	float FirstWaveDelay = 2.0f;

	/** Delay before spawning a new wave once threshold is met. */
	UPROPERTY(EditAnywhere, Category="Waves", meta=(EditCondition="bEnableWaveSystem", ClampMin = 0.0, Units = "s"))
	float TimeBetweenWaves = 6.0f;

	/** Delay between enemies spawned inside a wave. */
	UPROPERTY(EditAnywhere, Category="Waves", meta=(EditCondition="bEnableWaveSystem", ClampMin = 0.0, Units = "s"))
	float TimeBetweenEnemySpawns = 0.35f;

	/** Number of enemies in wave 1. */
	UPROPERTY(EditAnywhere, Category="Waves", meta=(EditCondition="bEnableWaveSystem", ClampMin = 1))
	int32 InitialEnemiesPerWave = 4;

	/** Additional enemies added each wave. */
	UPROPERTY(EditAnywhere, Category="Waves", meta=(EditCondition="bEnableWaveSystem", ClampMin = 0))
	int32 EnemiesAddedPerWave = 2;

	/** Next wave triggers when alive enemies are at or below this ratio of wave size. 0 means all dead. */
	UPROPERTY(EditAnywhere, Category="Waves", meta=(EditCondition="bEnableWaveSystem", ClampMin = 0.0, ClampMax = 1.0))
	float NextWaveAliveRatioThreshold = 0.0f;

	/** Current quest door actor, registered by the level door instance. */
	TObjectPtr<AShooterObjectiveDoor> ObjectiveDoor;

	/** Number of objective keys collected so far. */
	int32 CurrentKeyCount = 0;

	/** Player kill count used to compute endings. */
	int32 PlayerKillCount = 0;

	/** True once the door has been unlocked. */
	bool bDoorUnlocked = false;

	/** True once the quest has already ended. */
	bool bQuestEnded = false;

	/** World time when timer unlock should occur. */
	float SurvivalUnlockTimestamp = -1.0f;

	/** Timer used to update survival countdown on HUD. */
	FTimerHandle SurvivalTimerHandle;

	/** Enemy spawn points used by wave system. */
	TArray<TObjectPtr<AActor>> EnemySpawnPoints;

	/** Cached transforms used by wave spawning, so points remain valid even if source actors are destroyed. */
	TArray<FTransform> EnemySpawnPointTransforms;

	/** Runtime-resolved AI controller class used for wave-spawned enemies. */
	TSubclassOf<AController> WaveEnemyControllerClass;

	/** Current wave number. Starts at 0 before first wave. */
	int32 CurrentWave = 0;

	/** Number of enemies currently alive in the level. */
	int32 AliveEnemyCount = 0;

	/** Target number of enemies for the currently active wave. */
	int32 TargetEnemiesThisWave = 0;

	/** Number of enemies already spawned in the active wave. */
	int32 SpawnedEnemiesThisWave = 0;

	/** True while current wave is still being spawned. */
	bool bIsSpawningWave = false;

	/** Timer used to start the next wave. */
	FTimerHandle NextWaveTimerHandle;

	/** Timer used to spawn enemies one by one during a wave. */
	FTimerHandle SpawnEnemyTimerHandle;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	/** Increases the score for the given team */
	void IncrementTeamScore(uint8 TeamByte);

	/** Registers the level's objective door actor. */
	void RegisterObjectiveDoor(AShooterObjectiveDoor* InDoor);

	/** Called by key collectible actors. */
	void NotifyObjectiveKeyCollected();

	/** Called by enemy death handlers to credit player kills. */
	void NotifyEnemyKilled(AController* KillerController);

	/** Called when the player dies before escaping. */
	void NotifyPlayerDied();

	/** Called by the objective door when the player interacts with it. */
	void RequestEscape(APawn* EscapingPawn);

	/** Returns true if door can currently be used to escape. */
	bool IsDoorUnlocked() const { return bDoorUnlocked; }

	/** Returns true if the run has already ended. */
	bool IsQuestEnded() const { return bQuestEnded; }

	/** Returns true if player death should trigger respawn instead of game over menu. */
	bool ShouldRespawnPlayerOnDeath() const { return bRespawnPlayerOnDeath; }

protected:

	/** Recomputes and pushes objective text to the player's HUD. */
	void UpdateObjectiveText();

	/** Repeating timer callback for survival unlock mode. */
	void UpdateSurvivalCountdown();

	/** Opens the objective door and updates HUD. */
	void UnlockDoor();

	/** Ends the run with a specific ending id and description. */
	void EndRun(const FName& EndingId, const FText& EndingText, bool bWon);

	/** Resolves the ending id using current kill count. */
	FName ResolveEndingByKillCount() const;

	/** Returns HUD text for a resolved ending id. */
	FText BuildEndingDescription(const FName& EndingId) const;

	/** Initializes wave spawning state and spawn points. */
	void InitializeWaveSystem();

	/** Starts the next enemy wave. */
	void StartNextWave();

	/** Spawns one enemy for the current wave. */
	void SpawnOneWaveEnemy();

	/** Schedules the next wave if current conditions are met. */
	void TryScheduleNextWave();

	/** Stops all wave-related timers and spawning state. */
	void StopWaveSystem();
};
