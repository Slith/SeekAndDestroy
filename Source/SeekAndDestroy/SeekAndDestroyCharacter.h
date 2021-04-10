// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SeekAndDestroyCharacter.generated.h"

class AWeapon;

UCLASS(Blueprintable)
class ASeekAndDestroyCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	TWeakObjectPtr<AWeapon> HeldWeapon;

public:
	ASeekAndDestroyCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	///** Returns CursorToWorld subobject **/
	//FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

	UFUNCTION(BlueprintPure, Category = SeekAndDestroy)
	AWeapon* GetHeldWeapon() const { return HeldWeapon.Get(); }

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void EquipWeapon(AWeapon* NewHeldWeapon);

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void UnequipWeapon();

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
};

