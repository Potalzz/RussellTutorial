// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpawnTestCharacter.generated.h"

UCLASS()
class ZOMBIESHOOTING_API ASpawnTestCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASpawnTestCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AActor> ActorBPToSpawn;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SpawnActor();

};

