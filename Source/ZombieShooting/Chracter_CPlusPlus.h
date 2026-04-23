// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Chracter_CPlusPlus.generated.h"

UCLASS()
class ZOMBIESHOOTING_API AChracter_CPlusPlus : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AChracter_CPlusPlus();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AActor> ActorBPToSpawn;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SpawnActor();

};
