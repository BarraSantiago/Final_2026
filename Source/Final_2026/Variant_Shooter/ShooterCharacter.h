#pragma once

#include "CoreMinimal.h"
#include "Final_2026Character.h"
#include "ShooterWeaponHolder.h"
#include "ShooterArchetypeTypes.h"
#include "Weapons/ShooterWeaponTypes.h"
#include "ShooterCharacter.generated.h"

class AShooterWeapon;
class UInputAction;
class UInputComponent;
class UPawnNoiseEmitterComponent;
class IShooterInteractable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBulletCountUpdatedDelegate, int32, MagazineSize, int32, Bullets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDamagedDelegate, float, LifePercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInteractionPromptUpdatedDelegate, bool, bVisible, FText, ObjectName, FText, HintText);

/**
 *  A player controllable first person shooter character
 *  Manages a weapon inventory through the IShooterWeaponHolder interface
 *  Manages health and death
 */
UCLASS(abstract)
class FINAL_2026_API AShooterCharacter : public AFinal_2026Character, public IShooterWeaponHolder
{
	GENERATED_BODY()
	
	/** AI Noise emitter component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UPawnNoiseEmitterComponent* PawnNoiseEmitter;

protected:

	/** Fire weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* FireAction;

	/** Switch weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SwitchWeaponAction;

	/** Reload weapon input action. */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* ReloadAction;

	/** Manual interaction input action. */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* InteractAction;

	/** Name of the first person mesh weapon socket */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName FirstPersonWeaponSocket = FName("HandGrip_R");

	/** Name of the third person mesh weapon socket */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName ThirdPersonWeaponSocket = FName("HandGrip_R");

	/** Max distance to use for aim traces */
	UPROPERTY(EditAnywhere, Category ="Aim", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxAimDistance = 10000.0f;

	/** Max HP this character can have */
	UPROPERTY(EditAnywhere, Category="Stats")
	FShooterArchetypeStats ArchetypeStats;

	/** Max HP from current archetype. */
	float MaxHP = 100.0f;

	/** Current HP remaining to this character */
	float CurrentHP = 0.0f;

	/** Team ID for this character*/
	UPROPERTY(EditAnywhere, Category="Team")
	uint8 TeamByte = 0;

	/** Max distance for manual interaction traces from the camera. */
	UPROPERTY(EditAnywhere, Category="Interaction", meta = (ClampMin = 0, ClampMax = 2000, Units = "cm"))
	float InteractionTraceDistance = 500.0f;

	/** List of weapons picked up by the character */
	TArray<AShooterWeapon*> OwnedWeapons;

	/** Weapon currently equipped and ready to shoot with */
	TObjectPtr<AShooterWeapon> CurrentWeapon;

	UPROPERTY(EditAnywhere, Category ="Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RespawnTime = 5.0f;

	FTimerHandle RespawnTimer;

	/** Actor currently under interaction focus. */
	TObjectPtr<AActor> FocusedInteractableActor;

	/** Whether interaction prompt is currently shown. */
	bool bShowingInteractionPrompt = false;

public:

	/** Bullet count updated delegate */
	FBulletCountUpdatedDelegate OnBulletCountUpdated;

	/** Damaged delegate */
	FDamagedDelegate OnDamaged;

	/** Interaction prompt state changed delegate. */
	FInteractionPromptUpdatedDelegate OnInteractionPromptUpdated;

public:

	/** Constructor */
	AShooterCharacter();

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Gameplay tick for interaction tracing. */
	virtual void Tick(float DeltaSeconds) override;

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:

	/** Handle incoming damage */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:

	/** Handles start firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartFiring();

	/** Handles stop firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopFiring();

	/** Handles switch weapon input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSwitchWeapon();

	/** Handles manual reload input. */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoReload();

	/** Handles manual interaction input. */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoInteract();

	/** Restores health and returns true if any health was restored. */
	UFUNCTION(BlueprintCallable, Category="Health")
	bool TryRestoreHealth(float HealthToRestore);

	/** Adds ammo to a weapon slot and returns true if any ammo was added. */
	UFUNCTION(BlueprintCallable, Category="Weapons")
	bool TryAddAmmo(EShooterWeaponSlot Slot, int32 AmmoToAdd);

	/** Returns current health percentage from 0 to 1. */
	UFUNCTION(BlueprintPure, Category="Health")
	float GetHealthPercent() const;

public:

	//~Begin IShooterWeaponHolder interface

	/** Attaches a weapon's meshes to the owner */
	virtual void AttachWeaponMeshes(AShooterWeapon* Weapon) override;

	/** Plays the firing montage for the weapon */
	virtual void PlayFiringMontage(UAnimMontage* Montage) override;

	/** Applies weapon recoil to the owner */
	virtual void AddWeaponRecoil(float Recoil) override;

	/** Updates the weapon's HUD with the current ammo count */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;

	/** Returns archetype damage multiplier for outgoing weapon damage. */
	virtual float GetWeaponDamageMultiplier() const override;

	/** Calculates and returns the aim location for the weapon */
	virtual FVector GetWeaponTargetLocation() override;

	/** Gives a weapon of this class to the owner */
	virtual void AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass) override;

	/** Activates the passed weapon */
	virtual void OnWeaponActivated(AShooterWeapon* Weapon) override;

	/** Deactivates the passed weapon */
	virtual void OnWeaponDeactivated(AShooterWeapon* Weapon) override;

	/** Notifies the owner that the weapon cooldown has expired and it's ready to shoot again */
	virtual void OnSemiWeaponRefire() override;

	//~End IShooterWeaponHolder interface

protected:

	/** Returns true if the character already owns a weapon of the given class */
	AShooterWeapon* FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const;

	/** Returns the first owned weapon matching a slot. */
	AShooterWeapon* FindWeaponBySlot(EShooterWeaponSlot Slot) const;

	/** Refreshes interaction focus by tracing from the first person camera. */
	void RefreshInteractionFocus();

	/** Updates HUD interaction prompt. */
	void UpdateInteractionPrompt(bool bVisible, const FText& ObjectName = FText::GetEmpty(), const FText& HintText = FText::GetEmpty());

	/** Called when this character's HP is depleted */
	void Die();

	/** Called to allow Blueprint code to react to this character's death */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "On Death"))
	void BP_OnDeath();

	/** Called from the respawn timer to destroy this character and force the PC to respawn */
	void OnRespawn();
};
