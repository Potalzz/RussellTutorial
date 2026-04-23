// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnTestCharacter.h"

// Sets default values
ASpawnTestCharacter::ASpawnTestCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASpawnTestCharacter::SpawnActor()
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

