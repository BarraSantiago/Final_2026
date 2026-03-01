// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterInteractable.h"
#include "ShooterObjectiveKey.generated.h"

class UStaticMeshComponent;
class USphereComponent;

/**
 *  Key collectible that unlocks the quest door.
 */
UCLASS()
class FINAL_2026_API AShooterObjectiveKey : public AActor, public IShooterInteractable
{
	GENERATED_BODY()

	/** Root collision used to support trace targeting and optional overlap behavior. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* InteractionCollision;

	/** Visible representation of the key. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

protected:

	/** Name shown in the interaction prompt. */
	UPROPERTY(EditAnywhere, Category = "Key")
	FText KeyName = FText::FromString(TEXT("Key_asset_0001"));

	/** Hint shown in the interaction prompt. */
	UPROPERTY(EditAnywhere, Category = "Key")
	FText InteractionHint = FText::FromString(TEXT("Press Interact to pick up"));

	/** If true, this key was already collected and can no longer be interacted with. */
	UPROPERTY(BlueprintReadOnly, Category = "Key")
	bool bCollected = false;

public:

	AShooterObjectiveKey();

protected:

	virtual void BeginPlay() override;

public:

	//~ Begin IShooterInteractable interface
	virtual FText GetInteractionName() const override;
	virtual FText GetInteractionHint() const override;
	virtual bool CanInteract(APawn* InteractingPawn) const override;
	virtual void Interact(APawn* InteractingPawn) override;
	//~ End IShooterInteractable interface

protected:

	/** Called in Blueprints after this key is collected. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Key", meta = (DisplayName = "OnCollected"))
	void BP_OnCollected(APawn* InteractingPawn);
};

