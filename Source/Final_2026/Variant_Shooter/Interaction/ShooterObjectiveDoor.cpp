// Copyright Epic Games, Inc. All Rights Reserved.


#include "Interaction/ShooterObjectiveDoor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "ShooterGameMode.h"
#include "ShooterCharacter.h"

AShooterObjectiveDoor::AShooterObjectiveDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	InteractionCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollision"));
	InteractionCollision->SetupAttachment(RootComponent);
	InteractionCollision->SetBoxExtent(FVector(50.0f, 20.0f, 100.0f));
	InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCollision->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(InteractionCollision);
	DoorMesh->SetCollisionProfileName(FName("BlockAll"));
}

void AShooterObjectiveDoor::BeginPlay()
{
	Super::BeginPlay();

	if (AShooterGameMode* GameMode = GetWorld()->GetAuthGameMode<AShooterGameMode>())
	{
		GameMode->RegisterObjectiveDoor(this);
	}
}

void AShooterObjectiveDoor::SetDoorUnlocked(bool bInUnlocked)
{
	const bool bWasLocked = !bUnlocked && bInUnlocked;
	bUnlocked = bInUnlocked;

	if (bWasLocked)
	{
		BP_OnDoorUnlocked();
	}
}

FText AShooterObjectiveDoor::GetInteractionName() const
{
	return DoorName;
}

FText AShooterObjectiveDoor::GetInteractionHint() const
{
	return bUnlocked ? EscapeHint : LockedHint;
}

bool AShooterObjectiveDoor::CanInteract(APawn* InteractingPawn) const
{
	return IsValid(InteractingPawn);
}

void AShooterObjectiveDoor::Interact(APawn* InteractingPawn)
{
	if (!CanInteract(InteractingPawn))
	{
		return;
	}

	if (bUnlocked)
	{
		if (AShooterGameMode* GameMode = GetWorld()->GetAuthGameMode<AShooterGameMode>())
		{
			if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InteractingPawn))
			{
				GameMode->RequestEscape(ShooterCharacter);
			}
		}

		BP_OnEscapeInteract(InteractingPawn);
		return;
	}

	BP_OnLockedInteract(InteractingPawn);
}

