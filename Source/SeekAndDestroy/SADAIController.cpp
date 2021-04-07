// Fill out your copyright notice in the Description page of Project Settings.


#include "SADAIController.h"
#include "SADBlueprintFunctionLibrary.h"
#include "SeekAndDestroyGameMode.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

void ASADAIController::BeginPlay()
{
	Super::BeginPlay();

	if (ASeekAndDestroyGameMode* SADGM = USADBlueprintFunctionLibrary::GetSeekAndDestroyGameMode(this))
	{
		SADGM->GamePhaseChangingCode.AddUObject(this, &ASADAIController::OnGamePhaseChanged);
	}
}

bool ASADAIController::RunBehaviorTree(UBehaviorTree* BTAsset)
{
	const bool bResult = Super::RunBehaviorTree(BTAsset);
	//CurrentBehaviorTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	return bResult;
}

void ASADAIController::OnGamePhaseChanged(EGamePhase NewGamePhase)
{
	switch (NewGamePhase)
	{
	case EGamePhase::Configuration:
		break;
	case EGamePhase::Play:
		RunBehaviorTree(DefaultBehaviorTree);
		break;
	case EGamePhase::End:
		if (BrainComponent)
		{
			static const FString FunctionName = GET_FUNCTION_NAME_STRING_CHECKED(ASADAIController, OnGamePhaseChanged);
			BrainComponent->StopLogic(FunctionName);
		}
		break;
	}
}
