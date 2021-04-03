// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SeekAndDestroyGameMode.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Configuration,
	Play,
	End
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGamePhaseChangingDelegate, EGamePhase, NewGamePhase);
DECLARE_MULTICAST_DELEGATE_OneParam(FGamePhaseChangingDelegateCode, EGamePhase /*NewGamePhase*/);

UCLASS(minimalapi)
class ASeekAndDestroyGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	EGamePhase GamePhase;

public:
	UPROPERTY(BlueprintAssignable, Category = SeekAndDestroy)
	FGamePhaseChangingDelegate GamePhaseChanging;
	FGamePhaseChangingDelegateCode GamePhaseChangingCode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SeekAndDestroy)
	int32 PlayerPawnSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SeekAndDestroy)
	int32 PlayerWeaponRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SeekAndDestroy)
	int32 PlayerWeaponDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SeekAndDestroy)
	int32 HostilePawnSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SeekAndDestroy)
	int32 HostilePawnCount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SeekAndDestroy)
	int32 HostilePawnHealth;

	// @TODO TArray of HostilePawns?


	ASeekAndDestroyGameMode();

	UFUNCTION(BlueprintPure, Category = SeekAndDestroy)
	EGamePhase GetGamePhase() const { return GamePhase; }

	void SwitchToGamePhase(EGamePhase InGamePhase);

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void RestartGame();

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void StartGame();

protected:
	virtual void BeginPlay() override;

	void OnGamePhaseChanged();
};



