// Copyright Epic Games, Inc. All Rights Reserved.

#include "SeekAndDestroyGameMode.h"
#include "SeekAndDestroyPlayerController.h"
#include "SeekAndDestroyCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/SpectatorPawn.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "NavigationSystem.h"

void ASeekAndDestroyGameMode::CleanUpCharacterArray(TArray<ASeekAndDestroyCharacter*>& CharacterArray)
{
	for (auto&& Character : CharacterArray)
	{
		Character->Destroy();
	}
	CharacterArray.Reset();
}

ASeekAndDestroyGameMode::ASeekAndDestroyGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ASeekAndDestroyPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SeekAndDestroyContent/Characters/BP_SAD_PlayerCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPlayerPawnClass = PlayerPawnBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<APawn> HostilePawnBPClass(TEXT("/Game/SeekAndDestroyContent/Characters/BP_SAD_HostileCharacter"));
	if (HostilePawnBPClass.Class != nullptr)
	{
		DefaultHostilePawnClass = HostilePawnBPClass.Class;
	}

	DefaultPawnClass = SpectatorClass;
}

void ASeekAndDestroyGameMode::CharacterDied(ASeekAndDestroyCharacter* DeadCharacter)
{
	if (!DeadCharacter)
	{
		return;
	}

	if (HostilePawns.Remove(DeadCharacter) <= 0)
	{
		PlayerPawns.Remove(DeadCharacter);
	}

	PawnsToCleanUp.Add(DeadCharacter);
	OnCharacterDied.Broadcast(DeadCharacter);

	if (HostilePawns.Num() <= 0 || PlayerPawns.Num() <= 0)
	{
		FinishGame();
	}
}

void ASeekAndDestroyGameMode::SwitchToGamePhase(EGamePhase NewGamePhase)
{
	if (OnPreGamePhaseChanged(NewGamePhase))
	{
		GamePhaseChangingCode.Broadcast(NewGamePhase);
		GamePhaseChanging.Broadcast(NewGamePhase);
		GamePhase = NewGamePhase;
		OnGamePhaseChanged();
	}
}

void ASeekAndDestroyGameMode::RestartGame()
{
	SwitchToGamePhase(EGamePhase::Configuration);
}

void ASeekAndDestroyGameMode::StartGame()
{
	SwitchToGamePhase(EGamePhase::Play);
}

void ASeekAndDestroyGameMode::FinishGame()
{
	SwitchToGamePhase(EGamePhase::End);
}

bool ASeekAndDestroyGameMode::FindRandomNavLocation(APawn* ForPawn, FVector& OutLocation) const
{
	float Radius = 1000000.0f;
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		const TSet<FNavigationBounds>& NavBounds = NavSys->GetNavigationBounds();
		for (const FNavigationBounds& NavBound : NavBounds)
		{
			Radius = NavBound.AreaBox.GetSize().GetMax();
			break;
		}
	}
	return FindRandomNavLocationInRadius(ForPawn, Radius, OutLocation);
}

bool ASeekAndDestroyGameMode::FindRandomNavLocationInRadius(APawn* ForPawn, float InRadius, FVector& OutLocation) const
{
	const FNavLocation& FoundNavLocation = FindRandomNavLocationInRadius(ForPawn, InRadius);
	OutLocation = FoundNavLocation.Location;
	return FoundNavLocation.HasNodeRef();
}

FNavLocation ASeekAndDestroyGameMode::FindRandomNavLocationInRadius(APawn* ForPawn, float InRadius) const
{
	FNavLocation FoundNavLocation;

	if (!ForPawn)
	{
		// @TODO log warning;
		return FoundNavLocation;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		// @TODO log warning;
		return FoundNavLocation;
	}

	const FNavAgentProperties& AgentProps = ForPawn->GetNavAgentPropertiesRef();
	const ANavigationData* NavData = NavSys->GetNavDataForProps(AgentProps);
	if (!NavData || !NavData->GetRandomReachablePointInRadius(ForPawn->GetNavAgentLocation(), InRadius, FoundNavLocation))
	{
		if (!NavSys->MainNavData || !NavSys->MainNavData->GetRandomReachablePointInRadius(FVector::ZeroVector, InRadius, FoundNavLocation))
		{
			// @TODO log warning;
			return FoundNavLocation;
		}
	}

	return FoundNavLocation;
}

ASeekAndDestroyCharacter* ASeekAndDestroyGameMode::PickRandomTarget(const TArray<ASeekAndDestroyCharacter*>& ArrayOfTargets)
{
	if (ArrayOfTargets.Num() > 0)
	{
		return ArrayOfTargets[FMath::RandRange(0, ArrayOfTargets.Num() - 1)];
	}
	return nullptr;
}

void ASeekAndDestroyGameMode::BeginPlay()
{
	Super::BeginPlay();

	RestartGame();
}

bool ASeekAndDestroyGameMode::OnPreGamePhaseChanged(EGamePhase NewGamePhase)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		// @TODO log warning;
		SwitchToGamePhase(EGamePhase::End);
		return false;
	}

	float RadiusForSpawn = 0.0f;
	FNavLocation InitialNavLocation;
	NavSys->ProjectPointToNavigation(InitialNavLocation.Location, InitialNavLocation);

	ASeekAndDestroyCharacter* SpawnedCharacter = nullptr;

	switch (NewGamePhase)
	{
	case EGamePhase::Configuration:
		CleanUpCharacterArray(PawnsToCleanUp);
		CleanUpCharacterArray(PlayerPawns);
		CleanUpCharacterArray(HostilePawns);

		// Player pawn spawning / restart.
		// @TODO Spawning in loop PlayerPawnCount.
		SpawnedCharacter = Cast<ASeekAndDestroyCharacter>(GetWorld()->SpawnActor(DefaultPlayerPawnClass, &InitialNavLocation.Location));
		if (SpawnedCharacter)
		{
			PlayerPawns.Add(SpawnedCharacter);
			SpawnedCharacter->SetActorLocation(InitialNavLocation.Location);
			// Auto spectate on first player pawn.
			GetWorld()->GetFirstPlayerController()->SetViewTarget(SpawnedCharacter);
			// @TODO Add selecting next and previous target to spectate?
		}

		break;
	case EGamePhase::Play:
		if (HostilePawnCount <= 0)
		{
			// @TODO log warning;
			SwitchToGamePhase(EGamePhase::End);
			return false;
		}

		if (PlayerPawns.Num() > 0 && PlayerPawns[0] != nullptr)
		{
			// Hostile spawning
			const TSet<FNavigationBounds>& NavBounds = NavSys->GetNavigationBounds();
			for (const FNavigationBounds& NavBound : NavBounds)
			{
				RadiusForSpawn = NavBound.AreaBox.GetSize().GetMax();
				break;
			}

			for (int i = 0; i < HostilePawnCount; ++i)
			{
				// Based on PlayerPawn which is already present.
				InitialNavLocation = FindRandomNavLocationInRadius(PlayerPawns[0], RadiusForSpawn);
				SpawnedCharacter = Cast<ASeekAndDestroyCharacter>(GetWorld()->SpawnActor(DefaultHostilePawnClass, &InitialNavLocation.Location));
				if (SpawnedCharacter)
				{
					HostilePawns.Add(SpawnedCharacter);
				}
			}

			// Account for impossibility of spawning too much.
			HostilePawnCount = HostilePawns.Num();
		}

		break;
	case EGamePhase::End:
		break;
	}

	return true;
}

void ASeekAndDestroyGameMode::OnGamePhaseChanged()
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	float RadiusForSpawn = 0.0f;
	FNavLocation InitialNavLocation;
	NavSys->ProjectPointToNavigation(InitialNavLocation.Location, InitialNavLocation);

	switch (GetGamePhase())
	{
	case EGamePhase::Configuration:
		break;
	case EGamePhase::Play:
		for (auto&& PlayerPawn : PlayerPawns)
		{
			// Reequip weapon to apply configuration mutators.
			PlayerPawn->EquipWeapon(PlayerPawn->GetHeldWeapon());
			if (PlayerPawn->GetCharacterMovement())
			{
				PlayerPawn->GetCharacterMovement()->MaxWalkSpeed = PlayerPawnSpeed;
			}
		}

		for (auto&& HostilePawn : HostilePawns)
		{
			// Reequip weapon to apply configuration mutators.
			HostilePawn->EquipWeapon(HostilePawn->GetHeldWeapon());
			if (HostilePawn->GetCharacterMovement())
			{
				HostilePawn->GetCharacterMovement()->MaxWalkSpeed = HostilePawnSpeed;
			}
			HostilePawn->SetMaxHealth(HostilePawnHealth);
			HostilePawn->SetHealth(HostilePawnHealth);
			HostilePawn->EquipWeapon(HostilePawns.Last()->GetHeldWeapon());
		}

		break;
	case EGamePhase::End:
		break;
	}
}
