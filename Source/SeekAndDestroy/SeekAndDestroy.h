// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSeekAndDestroy, Log, All);

#define SAD_WARNING(Message) UE_LOG(LogSeekAndDestroy, Warning, TEXT("%s"), *FString::Printf(TEXT("%s : %s."), TEXT(__FUNCTION__), TEXT(Message)))


UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Configuration,
	Play,
	End
};