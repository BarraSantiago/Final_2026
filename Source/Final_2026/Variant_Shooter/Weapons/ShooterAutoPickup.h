#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeaponTypes.h"
#include "ShooterAutoPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;

UENUM(BlueprintType)
enum class EShooterAutoPickupType : uint8
{
	Health UMETA(DisplayName = "Health"),
	Ammo UMETA(DisplayName = "Ammo")
};

/**
 *  Proximity pickup for health or ammo resources.
 */
UCLASS()
class FINAL_2026_API AShooterAutoPickup : public AActor
{
	GENERATED_BODY()

	/** Overlap sphere that auto-consumes this pickup. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* SphereCollision;

	/** Pickup visual. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

protected:

	/** Pickup name for design clarity and debug. */
	UPROPERTY(EditAnywhere, Category = "Pickup")
	FName PickupName = FName("Pickup");

	/** Resource category consumed on overlap. */
	UPROPERTY(EditAnywhere, Category = "Pickup")
	EShooterAutoPickupType PickupType = EShooterAutoPickupType::Health;

	/** For ammo pickups, selects which slot can receive ammo. */
	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (EditCondition = "PickupType == EShooterAutoPickupType::Ammo"))
	EShooterWeaponSlot AmmoType = EShooterWeaponSlot::MainArm;

	/** Resource amount granted on consume. */
	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (ClampMin = 1.0))
	int32 Value = 25;

	/** Probability that this pickup is enabled on BeginPlay. */
	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float DropProbability = 1.0f;

	/** Respawn delay after being consumed. */
	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (ClampMin = 0.0, Units = "s"))
	float RespawnTime = 8.0f;

	/** If true, this pickup can respawn after being consumed. */
	UPROPERTY(EditAnywhere, Category = "Pickup")
	bool bCanRespawn = true;

	FTimerHandle RespawnTimer;

	/** True when currently disabled and waiting for respawn. */
	bool bConsumed = false;

public:

	AShooterAutoPickup();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Enables this pickup after a consume-respawn cycle. */
	void RespawnPickup();

	/** Applies a random drop roll on BeginPlay. */
	void ApplyDropRoll();

	/** Consumes this pickup and schedules respawn if enabled. */
	void ConsumePickup();

	/** Called in Blueprint when pickup is consumed. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup", meta = (DisplayName = "OnConsumed"))
	void BP_OnConsumed(AActor* Consumer);

	/** Called in Blueprint when pickup respawns. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup", meta = (DisplayName = "OnRespawned"))
	void BP_OnRespawned();
};

