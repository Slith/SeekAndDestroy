// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SeekAndDestroy.h"

#include "SADAIController.generated.h"

class UBehaviorTree;

/**
 * 
 */
UCLASS()
class SEEKANDDESTROY_API ASADAIController : public AAIController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = SeekAndDestroy)
	UBehaviorTree* DefaultBehaviorTree;


	virtual void BeginPlay() override;

public:
	virtual bool RunBehaviorTree(UBehaviorTree* BTAsset) override;

	UFUNCTION(BlueprintPure, Category = SeekAndDestroy)
	UBehaviorTree* GetDefaultBehaviorTree() const { return DefaultBehaviorTree; }

	UFUNCTION()
	void OnGamePhaseChanged(EGamePhase NewGamePhase);
};
