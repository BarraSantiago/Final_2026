// Copyright Epic Games, Inc. All Rights Reserved.


#include "Interaction/ShooterToggleInteractable.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

AShooterToggleInteractable::AShooterToggleInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	InteractionCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollision"));
	InteractionCollision->SetupAttachment(RootComponent);
	InteractionCollision->SetBoxExtent(FVector(45.0f, 20.0f, 30.0f));
	InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCollision->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(InteractionCollision);
	Mesh->SetCollisionProfileName(FName("BlockAll"));
}

FText AShooterToggleInteractable::GetInteractionName() const
{
	return ObjectName;
}

FText AShooterToggleInteractable::GetInteractionHint() const
{
	return bIsActive ? DeactivateHint : ActivateHint;
}

bool AShooterToggleInteractable::CanInteract(APawn* InteractingPawn) const
{
	return IsValid(InteractingPawn);
}

void AShooterToggleInteractable::Interact(APawn* InteractingPawn)
{
	if (!CanInteract(InteractingPawn))
	{
		return;
	}

	bIsActive = !bIsActive;
	BP_OnToggled(bIsActive, InteractingPawn);
}

