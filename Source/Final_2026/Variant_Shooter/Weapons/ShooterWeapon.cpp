#include "ShooterWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Final_2026.h"
#include "ShooterProjectile.h"
#include "ShooterWeaponHolder.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/DamageType.h"

AShooterWeapon::AShooterWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	// create the root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// create the first person mesh
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(RootComponent);

	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
	FirstPersonMesh->bOnlyOwnerSee = true;

	// create the third person mesh
	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third Person Mesh"));
	ThirdPersonMesh->SetupAttachment(RootComponent);

	ThirdPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	ThirdPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);
	ThirdPersonMesh->bOwnerNoSee = true;
}

void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (!WeaponDamageType)
	{
		WeaponDamageType = UDamageType::StaticClass();
	}

	// subscribe to the owner's destroyed delegate
	if (IsValid(GetOwner()))
	{
		GetOwner()->OnDestroyed.AddDynamic(this, &AShooterWeapon::OnOwnerDestroyed);
	}

	// cast the weapon owner
	WeaponOwner = Cast<IShooterWeaponHolder>(GetOwner());
	PawnOwner = Cast<APawn>(GetOwner());

	// fill ammo and reserve using capacity limits
	CurrentBullets = FMath::Clamp(MagazineSize, 0, AmmoCapacity);
	ReserveAmmo = FMath::Max(0, AmmoCapacity - CurrentBullets);

	// attach the meshes to the owner
	if (WeaponOwner)
	{
		WeaponOwner->AttachWeaponMeshes(this);
		WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
	}
}

void AShooterWeapon::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the refire timer
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
}

void AShooterWeapon::OnOwnerDestroyed(AActor* DestroyedActor)
{
	// ensure this weapon is destroyed when the owner is destroyed
	Destroy();
}

void AShooterWeapon::ActivateWeapon()
{
	// unhide this weapon
	SetActorHiddenInGame(false);

	// notify the owner
	if (WeaponOwner)
	{
		WeaponOwner->OnWeaponActivated(this);
		WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
	}
}

void AShooterWeapon::DeactivateWeapon()
{
	// ensure we're no longer firing this weapon while deactivated
	StopFiring();

	// hide the weapon
	SetActorHiddenInGame(true);

	// notify the owner
	if (WeaponOwner)
	{
		WeaponOwner->OnWeaponDeactivated(this);
	}
}

void AShooterWeapon::StartFiring()
{
	UE_LOG(LogFinal_2026, Log, TEXT("ShooterWeapon %s StartFiring"), *GetName());
	if (!WeaponOwner)
	{
		UE_LOG(LogFinal_2026, Warning, TEXT("ShooterWeapon::StartFiring failed: WeaponOwner is null for %s"), *GetName());
		return;
	}

	// raise the firing flag
	bIsFiring = true;

	if (!CanFireNow())
	{
		StartReload();
		return;
	}

	// check how much time has passed since we last shot
	// this may be under the refire rate if the weapon shoots slow enough and the trigger is held/spammed
	const float TimeSinceLastShot = GetWorld()->GetTimeSeconds() - TimeOfLastShot;
	const bool bCanShootImmediately = (TimeOfLastShot <= 0.0f) || (TimeSinceLastShot >= RefireRate);

	if (bCanShootImmediately)
	{
		// fire the weapon right away
		Fire();
		return;
	}

	const float TimeToNextShot = FMath::Max(0.0f, RefireRate - TimeSinceLastShot);

	// full-auto retries by calling Fire directly; semi-auto notifies owner so AI holders can request another shot.
	if (bFullAuto)
	{
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, TimeToNextShot, false);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::FireCooldownExpired, TimeToNextShot,
		                                       false);
	}
}

void AShooterWeapon::StopFiring()
{
	// lower the firing flag
	bIsFiring = false;

	// clear the refire timer
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
}

void AShooterWeapon::Fire()
{
	// ensure the weapon can currently shoot
	if (!bIsFiring || !CanFireNow() || !WeaponOwner)
	{
		if (bIsFiring)
		{
			StartReload();
		}

		return;
	}

	if (!ProjectileClass)
	{
		UE_LOG(LogFinal_2026, Warning, TEXT("ShooterWeapon::Fire aborted: ProjectileClass is null for %s"), *GetName());
		return;
	}
	
	// fire a projectile at the target
	FireProjectile(WeaponOwner->GetWeaponTargetLocation());

	// update the time of our last shot
	TimeOfLastShot = GetWorld()->GetTimeSeconds();

	// make noise so the AI perception system can hear us
	if (PawnOwner)
	{
		MakeNoise(ShotLoudness, PawnOwner, PawnOwner->GetActorLocation(), ShotNoiseRange, ShotNoiseTag);
	}

	// are we full auto?
	if (bFullAuto)
	{
		// schedule the next shot
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, RefireRate, false);
	} else {

		// for semi-auto weapons, schedule the cooldown notification
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::FireCooldownExpired, RefireRate, false);

	}
}

void AShooterWeapon::FireCooldownExpired()
{
	// notify the owner
	if (WeaponOwner)
	{
		WeaponOwner->OnSemiWeaponRefire();
	}
}

void AShooterWeapon::StartReload()
{
	if (bIsReloading)
	{
		return;
	}

	if (CurrentBullets >= MagazineSize || ReserveAmmo <= 0)
	{
		return;
	}

	bIsReloading = true;
	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &AShooterWeapon::CompleteReload, ReloadTime, false);
}

void AShooterWeapon::CompleteReload()
{
	bIsReloading = false;

	if (CurrentBullets >= MagazineSize || ReserveAmmo <= 0)
	{
		return;
	}

	const int32 MissingBullets = MagazineSize - CurrentBullets;
	const int32 BulletsToLoad = FMath::Clamp(MissingBullets, 0, ReserveAmmo);

	CurrentBullets += BulletsToLoad;
	ReserveAmmo -= BulletsToLoad;

	if (WeaponOwner)
	{
		WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
	}

	// Resume fire automatically for full-auto weapons while trigger is held.
	if (bIsFiring && bFullAuto && CanFireNow())
	{
		Fire();
	}
}

void AShooterWeapon::FireProjectile(const FVector& TargetLocation)
{
	if (!ProjectileClass)
	{
		return;
	}

	// consume one bullet up-front, then spawn the projectile.
	--CurrentBullets;

	// get the projectile transform
	FTransform ProjectileTransform = CalculateProjectileSpawnTransform(TargetLocation);
	
	// spawn the projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = PawnOwner;

	AShooterProjectile* Projectile = GetWorld()->SpawnActor<AShooterProjectile>(ProjectileClass, ProjectileTransform, SpawnParams);

	if (Projectile)
	{
		const float EffectiveDamage = WeaponDamage * WeaponOwner->GetWeaponDamageMultiplier();
		Projectile->SetHitDamage(EffectiveDamage);
		Projectile->SetHitDamageType(WeaponDamageType);
	}

	// play the firing montage
	WeaponOwner->PlayFiringMontage(FiringMontage);

	// add recoil
	WeaponOwner->AddWeaponRecoil(FiringRecoil);

	// update the weapon HUD
	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);

	if (CurrentBullets <= 0)
	{
		StartReload();
	}
}

FTransform AShooterWeapon::CalculateProjectileSpawnTransform(const FVector& TargetLocation) const
{
	// find the muzzle location
	const FVector MuzzleLoc = FirstPersonMesh->GetSocketLocation(MuzzleSocketName);

	// calculate the spawn location ahead of the muzzle
	const FVector SpawnLoc = MuzzleLoc + ((TargetLocation - MuzzleLoc).GetSafeNormal() * MuzzleOffset);

	// find the aim rotation vector while applying some variance to the target 
	const FRotator AimRot = UKismetMathLibrary::FindLookAtRotation(SpawnLoc, TargetLocation + (UKismetMathLibrary::RandomUnitVector() * AimVariance));

	// return the built transform
	return FTransform(AimRot, SpawnLoc, FVector::OneVector);
}

const TSubclassOf<UAnimInstance>& AShooterWeapon::GetFirstPersonAnimInstanceClass() const
{
	return FirstPersonAnimInstanceClass;
}

const TSubclassOf<UAnimInstance>& AShooterWeapon::GetThirdPersonAnimInstanceClass() const
{
	return ThirdPersonAnimInstanceClass;
}

void AShooterWeapon::RequestReload()
{
	StartReload();
}

bool AShooterWeapon::CanFireNow() const
{
	return !bIsReloading && CurrentBullets > 0;
}

int32 AShooterWeapon::AddAmmo(int32 AmmoToAdd)
{
	if (AmmoToAdd <= 0)
	{
		return 0;
	}

	const int32 TotalAmmo = CurrentBullets + ReserveAmmo;
	const int32 MissingAmmo = FMath::Max(0, AmmoCapacity - TotalAmmo);
	const int32 AddedAmmo = FMath::Min(AmmoToAdd, MissingAmmo);

	ReserveAmmo += AddedAmmo;

	if (WeaponOwner)
	{
		WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
	}

	return AddedAmmo;
}
