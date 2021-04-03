// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SADBlueprintFunctionLibrary.generated.h"

class ASeekAndDestroyGameMode;

/**
 * 
 */
UCLASS()
class SEEKANDDESTROY_API USADBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintPure, Category = SeekAndDestroy, meta = (WorldContext = "WorldContextObject"))
	static ASeekAndDestroyGameMode* GetSeekAndDestroyGameMode(const UObject* WorldContextObject);
};
