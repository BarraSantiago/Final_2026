#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ShooterInteractable.generated.h"

UINTERFACE(MinimalAPI)
class UShooterInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 *  Interface for manually interactable world objects.
 *  Interaction is driven by aiming at the object and pressing the interact input.
 */
class FINAL_2026_API IShooterInteractable
{
	GENERATED_BODY()

public:

	/** Name shown near the crosshair when this object is targeted. */
	virtual FText GetInteractionName() const = 0;

	/** Hint shown near the crosshair when this object is targeted. */
	virtual FText GetInteractionHint() const = 0;

	/** Returns true if this object can currently be interacted with. */
	virtual bool CanInteract(APawn* InteractingPawn) const = 0;

	/** Executes the interaction. */
	virtual void Interact(APawn* InteractingPawn) = 0;
};

