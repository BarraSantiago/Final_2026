// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterInteractable.h"
#include "ShooterToggleInteractable.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

/**
 *  Generic toggle interactable used for environment interactions (drawer/lever/switch).
 */
UCLASS()
class FINAL_2026_API AShooterToggleInteractable : public AActor, public IShooterInteractable
{
	GENERATED_BODY()

	/** Collision used for trace interaction. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* InteractionCollision;

	/** Mesh representation. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

protected:

	/** Name shown in prompt. */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	FText ObjectName = FText::FromString(TEXT("Environment Object"));

	/** Hint shown when object is inactive. */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	FText ActivateHint = FText::FromString(TEXT("Press Interact to activate"));

	/** Hint shown when object is active. */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	FText DeactivateHint = FText::FromString(TEXT("Press Interact to deactivate"));

	/** Current toggle state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bIsActive = false;

public:

	AShooterToggleInteractable();

	//~ Begin IShooterInteractable interface
	virtual FText GetInteractionName() const override;
	virtual FText GetInteractionHint() const override;
	virtual bool CanInteract(APawn* InteractingPawn) const override;
	virtual void Interact(APawn* InteractingPawn) override;
	//~ End IShooterInteractable interface

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction", meta = (DisplayName = "OnToggled"))
	void BP_OnToggled(bool bNewIsActive, APawn* InteractingPawn);
};

