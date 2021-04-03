// Copyright Epic Games, Inc. All Rights Reserved.

#include "SeekAndDestroyGameMode.h"
#include "SeekAndDestroyPlayerController.h"
#include "SeekAndDestroyCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASeekAndDestroyGameMode::ASeekAndDestroyGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ASeekAndDestroyPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
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

void ASeekAndDestroyGameMode::BeginPlay()
{
	Super::BeginPlay();

	RestartGame();
}

void ASeekAndDestroyGameMode::OnGamePhaseChanged()
{
	switch (GetGamePhase())
	{
	case EGamePhase::Configuration:
		break;
	case EGamePhase::Play:
		if (HostilePawnCount <= 0)
		{
			SwitchToGamePhase(EGamePhase::End);
			return;
		}
		// @TODO spawning
		break;
	case EGamePhase::End:
		break;
	}
}
