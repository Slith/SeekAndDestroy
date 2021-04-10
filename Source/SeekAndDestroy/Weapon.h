// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Weapon.generated.h"

UCLASS()
class SEEKANDDESTROY_API AWeapon : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	float Range;
	UPROPERTY()
	float DamagePerHit;
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = SeekAndDestroy)
	FPointDamageEvent PrepareDamageEvent() const;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = SeekAndDestroy)
	USkeletalMeshComponent* GetMesh() const { return Mesh; }

	UFUNCTION(BlueprintPure, Category = SeekAndDestroy)
	float GetRange() const { return Range; }

	UFUNCTION(BlueprintPure, Category = SeekAndDestroy)
	float GetDamagePerHit() const { return DamagePerHit; }


	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	bool IsActorInLOS(AActor* InActor) const;

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void ApplyDamageToActor(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void OnEquippedBy(APawn* InInstigator);

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void OnUnequipped();
};
