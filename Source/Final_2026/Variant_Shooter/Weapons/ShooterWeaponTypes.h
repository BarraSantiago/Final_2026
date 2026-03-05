#pragma once

#include "CoreMinimal.h"
#include "ShooterWeaponTypes.generated.h"

/**
 *  Weapon slot categories used for loadout and ammo pickups.
 */
UENUM(BlueprintType)
enum class EShooterWeaponSlot : uint8
{
	MainArm UMETA(DisplayName = "Main Arm"),
	SideArm UMETA(DisplayName = "Side Arm")
};

