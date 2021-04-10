// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SeekAndDestroy.h"

#include "SeekAndDestroyGameMode.generated.h"

class ASeekAndDestroyCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGamePhaseChangingDelegate, EGamePhase, NewGamePhase);
DECLARE_MULTICAST_DELEGATE_OneParam(FGamePhaseChangingDelegateCode, EGamePhase /*NewGamePhase*/);

UCLASS(minimalapi)
class ASeekAndDestroyGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	EGamePhase GamePhase;

	/** The default pawn class used by player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Classes)
	TSubclassOf<APawn> DefaultPlayerPawnClass;
	/** The default pawn class used by hostiles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Classes)
	TSubclassOf<APawn> DefaultHostilePawnClass;

	// @TODO Spawning in loop PlayerPawnCount.
	UPROPERTY(VisibleAnywhere, Category=SeekAndDestroy)
	TArray<ASeekAndDestroyCharacter*> PlayerPawns;
	UPROPERTY(VisibleAnywhere, Category=SeekAndDestroy)
	TArray<ASeekAndDestroyCharacter*> HostilePawns;
	UPROPERTY(VisibleAnywhere, Category=SeekAndDestroy)
	TArray<ASeekAndDestroyCharacter*> PawnsToCleanUp;

public:
	UPROPERTY(BlueprintAssignable, Category = SeekAndDestroy)
	FGamePhaseChangingDelegate GamePhaseChanging;
	FGamePhaseChangingDelegateCode GamePhaseChangingCode;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SeekAndDestroy)
	//int32 PlayerPawnCount;
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


	ASeekAndDestroyGameMode();

	// @FIXME See if const reference can be modified in BP. If it can, make a copy for safety.
	UFUNCTION(BlueprintPure, Category = SeekAndDestroy)
	const TArray<ASeekAndDestroyCharacter*>& GetPlayerPawns() const { return PlayerPawns; }

	// @FIXME See if const reference can be modified in BP. If it can, make a copy for safety.
	UFUNCTION(BlueprintPure, Category = SeekAndDestroy)
	const TArray<ASeekAndDestroyCharacter*>& GetHostilePawns() const { return HostilePawns; }

	virtual void CharacterDied(ASeekAndDestroyCharacter* DeadCharacter);

	UFUNCTION(BlueprintPure, Category = SeekAndDestroy)
	EGamePhase GetGamePhase() const { return GamePhase; }

	void SwitchToGamePhase(EGamePhase NewGamePhase);

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void RestartGame();

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	void FinishGame();

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	bool FindRandomNavLocation(APawn* ForPawn, FVector& OutLocation) const;

	UFUNCTION(BlueprintCallable, Category = SeekAndDestroy)
	bool FindRandomNavLocationInRadius(APawn* ForPawn, float InRadius, FVector& OutLocation) const;
	FNavLocation FindRandomNavLocationInRadius(APawn* ForPawn, float InRadius) const;

protected:
	virtual void BeginPlay() override;

	void OnPreGamePhaseChanged(EGamePhase NewGamePhase);
	void OnGamePhaseChanged();
};



