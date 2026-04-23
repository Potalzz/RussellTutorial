// Fill out your copyright notice in the Description page of Project Settings.


#include "Chracter_CPlusPlus.h"

// Sets default values
AChracter_CPlusPlus::AChracter_CPlusPlus()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AChracter_CPlusPlus::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AChracter_CPlusPlus::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AChracter_CPlusPlus::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

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
