// Copyright Druid Mechanics


#include "Player/AuraPlayerController.h"


AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;

}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(AuraContext);

}
