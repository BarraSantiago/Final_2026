#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Final_2026GameInstance.generated.h"

UENUM(BlueprintType)
enum class EShooterRunMode : uint8
{
	None UMETA(DisplayName = "None"),
	KeyEscape UMETA(DisplayName = "Grab Key And Escape"),
	Survival UMETA(DisplayName = "Survival")
};

/**
 * Stores the selected shooter run mode between map loads.
 */
UCLASS()
class FINAL_2026_API UFinal_2026GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Shooter|Main Menu")
	void SetSelectedShooterRunMode(EShooterRunMode InMode);

	UFUNCTION(BlueprintPure, Category = "Shooter|Main Menu")
	EShooterRunMode GetSelectedShooterRunMode() const { return SelectedShooterRunMode; }

protected:

	UPROPERTY(VisibleAnywhere, Category = "Shooter|Main Menu")
	EShooterRunMode SelectedShooterRunMode = EShooterRunMode::None;
};
