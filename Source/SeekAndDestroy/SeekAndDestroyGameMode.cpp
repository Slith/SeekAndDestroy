// Copyright Epic Games, Inc. All Rights Reserved.

#include "SeekAndDestroyGameMode.h"
#include "SeekAndDestroyPlayerController.h"
#include "SeekAndDestroyCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/SpectatorPawn.h"

#include "NavigationSystem.h"

ASeekAndDestroyGameMode::ASeekAndDestroyGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ASeekAndDestroyPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SeekAndDestroyContent/SAD_PlayerCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPlayerPawnClass = PlayerPawnBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<APawn> HostilePawnBPClass(TEXT("/Game/SeekAndDestroyContent/SAD_HostileCharacter"));
	if (HostilePawnBPClass.Class != nullptr)
	{
		DefaultHostilePawnClass = HostilePawnBPClass.Class;
	}

	DefaultPawnClass = SpectatorClass;
}

void ASeekAndDestroyGameMode::SwitchToGamePhase(EGamePhase InGamePhase)
{
	GamePhaseChangingCode.Broadcast(InGamePhase);
	GamePhaseChanging.Broadcast(InGamePhase);
	GamePhase = InGamePhase;
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

bool ASeekAndDestroyGameMode::FindRandomNavLocation(APawn* ForPawn, float InRadius, FVector& OutLocation) const
{
	const FNavLocation& FoundNavLocation = FindRandomNavLocation(ForPawn, InRadius);
	OutLocation = FoundNavLocation.Location;
	return FoundNavLocation.HasNodeRef();
}

FNavLocation ASeekAndDestroyGameMode::FindRandomNavLocation(APawn* ForPawn, float InRadius) const
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

void ASeekAndDestroyGameMode::OnGamePhaseChanged()
{
	FVector InitialLocation = FVector::ZeroVector;
	UNavigationSystemV1* NavSys = nullptr;
	float RadiusForSpawn = 0.0f;

	switch (GetGamePhase())
	{
	case EGamePhase::Configuration:
		break;
	case EGamePhase::Play:
		if (HostilePawnCount <= 0)
		{
			// @TODO log warning;
			SwitchToGamePhase(EGamePhase::End);
			return;
		}
		NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (!NavSys)
		{
			// @TODO log warning;
			SwitchToGamePhase(EGamePhase::End);
			return;
		}

		// Conditional player pawn spawning
		if (PlayerPawns.Num() <= 0 || PlayerPawns[0] == nullptr)
		{
			PlayerPawns.Reset();
			// @TODO Spawning in loop PlayerPawnCount.
			InitialLocation = NavSys->ProjectPointToNavigation(this, InitialLocation); // FindRandomNavLocation(nullptr, 0.0f).Location;
			PlayerPawns.Add(Cast<ASeekAndDestroyCharacter>(GetWorld()->SpawnActor(DefaultPlayerPawnClass, &InitialLocation)));
		}

		// Hostile spawning
		if (PlayerPawns.Num() > 0 && PlayerPawns[0] != nullptr)
		{
			// @TODO Auto spectate on first player pawn. Add selecting next and previous target to spectate?
			GetWorld()->GetFirstPlayerController()->SetViewTarget(PlayerPawns[0]);

			const TSet<FNavigationBounds>& NavBounds = NavSys->GetNavigationBounds();
			for (const FNavigationBounds& NavBound : NavBounds)
			{
				RadiusForSpawn = NavBound.AreaBox.GetSize().GetMax();
				break;
			}

			for (int i = 0; i < HostilePawnCount; ++i)
			{
				InitialLocation = FindRandomNavLocation(PlayerPawns[0], RadiusForSpawn).Location;
				HostilePawns.Add(Cast<ASeekAndDestroyCharacter>(GetWorld()->SpawnActor(DefaultHostilePawnClass, &InitialLocation)));
			}
		}

		break;
	case EGamePhase::End:
		break;
	}
}
