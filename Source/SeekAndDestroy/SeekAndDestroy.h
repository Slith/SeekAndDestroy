// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSeekAndDestroy, Log, All);

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Configuration,
	Play,
	End
};