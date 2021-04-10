// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SeekAndDestroyCharacter.generated.h"

class AWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFloatValueChanged, float, NewValue);


UCLASS(Blueprintable)
class ASeekAndDestroyCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

protected:
	UPROPERTY()
	TWeakObjectPtr<AWeapon> HeldWeapon;

	UPROPERTY()
	float MaxHealth;
	UPROPERTY()
	float Health;

	UPROPERTY(BlueprintAssignable, Category = SeekAndDestroy)
	FFloatValueChanged MaxHealthChanged;
	UPROPERTY(BlueprintAssignable, Category = SeekAndDestroy)
	FFloatValueChanged HealthChanged;

public:
	ASeekAndDestroyCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die();

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	UFUNCTION(BlueprintPure, Category = SeekAndDestroy)
	AWeapon* GetHeldWeapon() const { return HeldWeapon.Get(); }


	UFUNCTION(BlueprintPure, Category = SeekAndDestroy)
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = SeekAndDestroy)
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void SetMaxHealth(float NewMaxHealth);

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void SetHealth(float NewHealth);


	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void EquipWeapon(AWeapon* NewHeldWeapon);

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void UnequipWeapon();
};

