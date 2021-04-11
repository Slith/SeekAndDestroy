// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "SADBlueprintFunctionLibrary.h"
#include "SeekAndDestroyGameMode.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));

	Range = 100.0f;
	DamagePerHit = 10.0f;
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

FPointDamageEvent AWeapon::PrepareDamageEvent() const
{
	FPointDamageEvent DamageEvent;
	DamageEvent.DamageTypeClass = UDamageType::StaticClass();
	// @TODO Extend function parameters to get these.
	//DamageEvent.ShotDirection = ;
	//DamageEvent.HitInfo = ;
	DamageEvent.Damage = DamagePerHit;

	return DamageEvent;
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AWeapon::TryAttack(AActor* TargetActor)
{
	if (!TargetActor)
	{
		// log warning
		return false;
	}
	
	float CollisionRadius = 0.0f;
	float CollisionHalfHeight = 0.0f;
	TargetActor->GetComponentsBoundingCylinder(CollisionRadius, CollisionHalfHeight);

	const FVector DistanceVector = TargetActor->GetActorLocation() - GetMuzzleLocation();
	const bool bInRange = DistanceVector.Size() - CollisionRadius <= GetRange();

	if (bInRange)
	{
		ShowAttackFX(TargetActor);
		ApplyDamageToActor(TargetActor);
	}

	return bInRange;
}

void AWeapon::ApplyDamageToActor(AActor* TargetActor)
{
	if (!TargetActor || !TargetActor->CanBeDamaged())
	{
		// log warning
		return;
	}

	TargetActor->TakeDamage(DamagePerHit, PrepareDamageEvent(), GetInstigator()? GetInstigator()->GetController() : nullptr, this);
}

void AWeapon::OnEquippedBy(APawn* InInstigator)
{
	SetInstigator(InInstigator);

	// @TODO This should be set based on InInstigator's team ID, but that's way beyond our scope here.
	if (ASeekAndDestroyGameMode* SADGM = USADBlueprintFunctionLibrary::GetSeekAndDestroyGameMode(this))
	{
		Range = SADGM->PlayerWeaponRange;
		DamagePerHit = SADGM->PlayerWeaponDamage;
	}
}

void AWeapon::OnUnequipped()
{
	SetInstigator(nullptr);

	// @TODO Reset default values in dedicated method?
	Range = Cast<AWeapon>(GetClass()->GetDefaultObject())->Range;
	DamagePerHit = Cast<AWeapon>(GetClass()->GetDefaultObject())->DamagePerHit;
}
