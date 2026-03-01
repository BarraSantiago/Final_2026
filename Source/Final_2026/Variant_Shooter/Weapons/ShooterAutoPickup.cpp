// Copyright Epic Games, Inc. All Rights Reserved.


#include "Weapons/ShooterAutoPickup.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ShooterCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"

AShooterAutoPickup::AShooterAutoPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetupAttachment(RootComponent);
	SphereCollision->SetSphereRadius(90.0f);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ECC_WorldDynamic);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCollision->bFillCollisionUnderneathForNavmesh = true;
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AShooterAutoPickup::OnOverlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);
	Mesh->SetCollisionProfileName(FName("NoCollision"));
}

void AShooterAutoPickup::BeginPlay()
{
	Super::BeginPlay();
	ApplyDropRoll();
}

void AShooterAutoPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterAutoPickup::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bConsumed || !IsValid(OtherActor))
	{
		return;
	}

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (!IsValid(ShooterCharacter))
	{
		return;
	}

	bool bConsumedByCharacter = false;

	if (PickupType == EShooterAutoPickupType::Health)
	{
		bConsumedByCharacter = ShooterCharacter->TryRestoreHealth(Value);
	}
	else
	{
		bConsumedByCharacter = ShooterCharacter->TryAddAmmo(AmmoType, Value);
	}

	if (!bConsumedByCharacter)
	{
		return;
	}

	BP_OnConsumed(ShooterCharacter);
	ConsumePickup();
}

void AShooterAutoPickup::RespawnPickup()
{
	bConsumed = false;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	BP_OnRespawned();
}

void AShooterAutoPickup::ApplyDropRoll()
{
	if (DropProbability >= 1.0f)
	{
		return;
	}

	if (FMath::FRand() > DropProbability)
	{
		bConsumed = true;
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}

void AShooterAutoPickup::ConsumePickup()
{
	bConsumed = true;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	if (bCanRespawn && RespawnTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterAutoPickup::RespawnPickup, RespawnTime, false);
	}
}

