// Copyright Epic Games, Inc. All Rights Reserved.

#include "SeekAndDestroyCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Engine/World.h"

#include "SADAIController.h"
#include "Weapon.h"
#include "SADBlueprintFunctionLibrary.h"
#include "SeekAndDestroyGameMode.h"


ASeekAndDestroyCharacter::ASeekAndDestroyCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 1600.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 200.0f;
	CameraBoom->CameraLagMaxDistance = 100.0f;

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ASADAIController::StaticClass();

	MaxHealth = 100.0f;
	Health = 100.0f;
}

void ASeekAndDestroyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

float ASeekAndDestroyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!ShouldTakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser))
	{
		return 0.f;
	}

	Health = Health - DamageAmount;

	// @NOTE We're not using any internal structure to modify damage, so we can use DamageAmount for cohesion.
	const float ActualDamage = AActor::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// respond to the damage
	if (DamageAmount != 0.f)
	{
		if (EventInstigator && EventInstigator != Controller)
		{
			LastHitBy = EventInstigator;
		}
	}

	if (Health <= 0.0f)
	{
		Die();
	}

	return DamageAmount;
}

void ASeekAndDestroyCharacter::Die()
{
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesSimulatePhysics(true);

	if (ASeekAndDestroyGameMode* SADGM = USADBlueprintFunctionLibrary::GetSeekAndDestroyGameMode(this))
	{
		SADGM->CharacterDied(this);
	}
}

void ASeekAndDestroyCharacter::SetMaxHealth(float NewMaxHealth)
{
	MaxHealth = NewMaxHealth;
	MaxHealthChanged.Broadcast(MaxHealth);
}

void ASeekAndDestroyCharacter::SetHealth(float NewHealth)
{
	Health = NewHealth;
	HealthChanged.Broadcast(Health);
}

void ASeekAndDestroyCharacter::EquipWeapon(AWeapon* NewHeldWeapon)
{
	if (!NewHeldWeapon)
	{
		SAD_WARNING("No NewHeldWeapon");
		return;
	}
	UnequipWeapon();
	HeldWeapon = NewHeldWeapon;
	NewHeldWeapon->OnEquippedBy(this);
	OnEquippedWeapon(NewHeldWeapon);
}

void ASeekAndDestroyCharacter::UnequipWeapon()
{
	if (AWeapon* Weapon = GetHeldWeapon())
	{
		Weapon->OnUnequipped();
		OnUnequippedWeapon(Weapon);
		HeldWeapon = nullptr;
	}
}
