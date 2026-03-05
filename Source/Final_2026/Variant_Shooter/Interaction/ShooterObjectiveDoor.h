

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterInteractable.h"
#include "ShooterObjectiveDoor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

/**
 *  Final objective door used to escape and finish the run.
 */
UCLASS()
class FINAL_2026_API AShooterObjectiveDoor : public AActor, public IShooterInteractable
{
	GENERATED_BODY()

	/** Collision used for trace interaction. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* InteractionCollision;

	/** Door mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* DoorMesh;

protected:

	/** Prompt display name. */
	UPROPERTY(EditAnywhere, Category = "Door")
	FText DoorName = FText::FromString(TEXT("Unassuming Wooden Door of Ultimate Destiny"));

	/** Hint when the door can be used to escape. */
	UPROPERTY(EditAnywhere, Category = "Door")
	FText EscapeHint = FText::FromString(TEXT("Press Interact to escape"));

	/** Hint when the door is still locked. */
	UPROPERTY(EditAnywhere, Category = "Door")
	FText LockedHint = FText::FromString(TEXT("Door is locked"));

	/** True once the objective unlock condition is met. */
	UPROPERTY(BlueprintReadOnly, Category = "Door")
	bool bUnlocked = false;

public:

	AShooterObjectiveDoor();

protected:

	virtual void BeginPlay() override;

public:

	/** Sets whether this door is unlocked and can be used to escape. */
	void SetDoorUnlocked(bool bInUnlocked);

	/** Returns true if this door is currently unlocked. */
	bool IsDoorUnlocked() const { return bUnlocked; }

	//~ Begin IShooterInteractable interface
	virtual FText GetInteractionName() const override;
	virtual FText GetInteractionHint() const override;
	virtual bool CanInteract(APawn* InteractingPawn) const override;
	virtual void Interact(APawn* InteractingPawn) override;
	//~ End IShooterInteractable interface

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Door", meta = (DisplayName = "OnDoorUnlocked"))
	void BP_OnDoorUnlocked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Door", meta = (DisplayName = "OnLockedInteract"))
	void BP_OnLockedInteract(APawn* InteractingPawn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Door", meta = (DisplayName = "OnEscapeInteract"))
	void BP_OnEscapeInteract(APawn* InteractingPawn);
};
