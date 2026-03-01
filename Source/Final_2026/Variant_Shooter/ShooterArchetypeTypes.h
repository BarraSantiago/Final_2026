// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShooterArchetypeTypes.generated.h"

/**
 *  Configurable gameplay stats for a character archetype.
 *  Applies to both player and enemies.
 */
USTRUCT(BlueprintType)
struct FShooterArchetypeStats
{
	GENERATED_BODY()

	/** Maximum health points. Character dies when reaching 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype", meta = (ClampMin = 1.0))
	float MaxHealth = 100.0f;

	/** Max movement speed in cm/s. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype", meta = (ClampMin = 1.0))
	float MoveSpeed = 600.0f;

	/** Scales outgoing damage for this archetype. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype", meta = (ClampMin = 0.1))
	float DamageMultiplier = 1.0f;
};

