// Copyright Epic Games, Inc. All Rights Reserved.

#include "SeekAndDestroyGameMode.h"
#include "SeekAndDestroyPlayerController.h"
#include "SeekAndDestroyCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/SpectatorPawn.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "NavigationSystem.h"

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
	OnPreGamePhaseChanged(NewGamePhase);
	GamePhaseChangingCode.Broadcast(NewGamePhase);
	GamePhaseChanging.Broadcast(NewGamePhase);
	GamePhase = NewGamePhase;
	OnGamePhaseChanged();
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

void ASeekAndDestroyGameMode::BeginPlay()
{
	Super::BeginPlay();

	RestartGame();
}

void ASeekAndDestroyGameMode::OnPreGamePhaseChanged(EGamePhase NewGamePhase)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		// @TODO log warning;
		SwitchToGamePhase(EGamePhase::End);
		return;
	}

	float RadiusForSpawn = 0.0f;
	FNavLocation InitialNavLocation;
	NavSys->ProjectPointToNavigation(InitialNavLocation.Location, InitialNavLocation);

	switch (NewGamePhase)
	{
	case EGamePhase::Configuration:
		// @TODO Contain in method.
		for (auto&& Pawn : PawnsToCleanUp)
		{
			Pawn->Destroy();
		}
		PawnsToCleanUp.Reset();

		for (auto&& PlayerPawn : PlayerPawns)
		{
			PlayerPawn->Destroy();
		}
		PlayerPawns.Reset();

		// Player pawn spawning / restart.
		// @TODO Spawning in loop PlayerPawnCount.
		PlayerPawns.Add(Cast<ASeekAndDestroyCharacter>(GetWorld()->SpawnActor(DefaultPlayerPawnClass, &InitialNavLocation.Location)));
		if (PlayerPawns.Num() > 0 && PlayerPawns[0] != nullptr)
		{
			PlayerPawns[0]->SetActorLocation(InitialNavLocation.Location);
			// Auto spectate on first player pawn.
			GetWorld()->GetFirstPlayerController()->SetViewTarget(PlayerPawns[0]);
			// @TODO Add selecting next and previous target to spectate?
		}

		for (auto&& HostilePawn : HostilePawns)
		{
			HostilePawn->Destroy();
		}
		HostilePawns.Reset();

		break;
	case EGamePhase::Play:
		if (HostilePawnCount <= 0)
		{
			// @TODO log warning;
			SwitchToGamePhase(EGamePhase::End);
			return;
		}

		for (auto&& PlayerPawn : PlayerPawns)
		{
			// Reequip weapon to apply configuration mutators.
			PlayerPawn->EquipWeapon(PlayerPawn->GetHeldWeapon());
			if (PlayerPawn->GetCharacterMovement())
			{
				PlayerPawn->GetCharacterMovement()->MaxWalkSpeed = PlayerPawnSpeed;
			}
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
				HostilePawns.Add(Cast<ASeekAndDestroyCharacter>(GetWorld()->SpawnActor(DefaultHostilePawnClass, &InitialNavLocation.Location)));
				HostilePawns.Last()->GetCharacterMovement()->MaxWalkSpeed = HostilePawnSpeed;
				HostilePawns.Last()->SetMaxHealth(HostilePawnHealth);
				HostilePawns.Last()->SetHealth(HostilePawnHealth);
			}
		}

		break;
	case EGamePhase::End:
		break;
	}
}

void ASeekAndDestroyGameMode::OnGamePhaseChanged()
{
	switch (GetGamePhase())
	{
	case EGamePhase::Configuration:
		break;
	case EGamePhase::Play:
		break;
	case EGamePhase::End:
		break;
	}
}
