// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

class UShooterUI;
class AShooterObjectiveDoor;
class AShooterCharacter;
class AController;

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
	void RequestEscape(AShooterCharacter* EscapingCharacter);

	/** Returns true if door can currently be used to escape. */
	bool IsDoorUnlocked() const { return bDoorUnlocked; }

	/** Returns true if the run has already ended. */
	bool IsQuestEnded() const { return bQuestEnded; }

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
};
