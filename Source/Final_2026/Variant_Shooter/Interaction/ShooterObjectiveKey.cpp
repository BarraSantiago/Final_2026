#include "Interaction/ShooterObjectiveKey.h"
#include "Final_2026.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ShooterGameMode.h"

AShooterObjectiveKey::AShooterObjectiveKey()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	InteractionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
	InteractionCollision->SetupAttachment(RootComponent);
	InteractionCollision->SetSphereRadius(90.0f);
	InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCollision->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(InteractionCollision);
	Mesh->SetCollisionProfileName(FName("NoCollision"));
}

void AShooterObjectiveKey::BeginPlay()
{
	Super::BeginPlay();
}

FText AShooterObjectiveKey::GetInteractionName() const
{
	return KeyName;
}

FText AShooterObjectiveKey::GetInteractionHint() const
{
	return InteractionHint;
}

bool AShooterObjectiveKey::CanInteract(APawn* InteractingPawn) const
{
	return !bCollected && IsValid(InteractingPawn);
}

void AShooterObjectiveKey::Interact(APawn* InteractingPawn)
{
	if (!CanInteract(InteractingPawn))
	{
		UE_LOG(LogFinal_2026, Warning, TEXT("ObjectiveKey %s interaction rejected"), *GetName());
		return;
	}

	bCollected = true;
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	UE_LOG(LogFinal_2026, Log, TEXT("ObjectiveKey %s collected by %s"), *GetName(), *GetNameSafe(InteractingPawn));

	if (AShooterGameMode* GameMode = GetWorld()->GetAuthGameMode<AShooterGameMode>())
	{
		GameMode->NotifyObjectiveKeyCollected();
	}
	else
	{
		UE_LOG(LogFinal_2026, Warning, TEXT("ObjectiveKey %s could not notify collection: AShooterGameMode missing"),
		       *GetName());
	}

	BP_OnCollected(InteractingPawn);
}
