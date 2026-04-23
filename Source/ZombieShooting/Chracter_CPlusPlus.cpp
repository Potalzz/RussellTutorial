// Fill out your copyright notice in the Description page of Project Settings.


#include "Chracter_CPlusPlus.h"

// Sets default values
AChracter_CPlusPlus::AChracter_CPlusPlus()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AChracter_CPlusPlus::SpawnActor()
{
	if (!ActorBPToSpawn)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	World->SpawnActor<AActor>(ActorBPToSpawn, GetActorTransform(), spawnParams);
}
