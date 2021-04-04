// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SeekAndDestroyPlayerController.generated.h"

UCLASS()
class ASeekAndDestroyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASeekAndDestroyPlayerController();

protected:
	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

	/** Resets HMD orientation in VR. */
	void OnResetVR();
	
	/** Navigate player to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);
};


