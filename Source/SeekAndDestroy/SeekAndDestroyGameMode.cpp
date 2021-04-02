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