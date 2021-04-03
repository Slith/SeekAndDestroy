// Fill out your copyright notice in the Description page of Project Settings.


#include "SADBlueprintFunctionLibrary.h"
#include "SeekAndDestroyGameMode.h"

ASeekAndDestroyGameMode* USADBlueprintFunctionLibrary::GetSeekAndDestroyGameMode(const UObject* WorldContextObject)
{
	ASeekAndDestroyGameMode* GameMode = nullptr;
	if (UWorld* World = WorldContextObject->GetWorld())
	{
		GameMode = Cast<ASeekAndDestroyGameMode>(World->GetAuthGameMode());
	}

	return GameMode;
}
