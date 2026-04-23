// Fill out your copyright notice in the Description page of Project Settings.

#include "ZombieShootingGameMode.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "RussellFirstPersonCharacter.h"
#include "RussellSurvivalHUD.h"
#include "RussellZombieCharacter.h"

AZombieShootingGameMode::AZombieShootingGameMode()
{
	DefaultPawnClass = ARussellFirstPersonCharacter::StaticClass();
	HUDClass = ARussellSurvivalHUD::StaticClass();

	ZombieClass = ARussellZombieCharacter::StaticClass();
	InitialZombiesPerWave = 4;
	AdditionalZombiesPerWave = 2;
	SpawnInterval = 1.25f;
	MinSpawnRadius = 1200.0f;
	MaxSpawnRadius = 2200.0f;
	NextWaveDelay = 4.0f;

	WaveNumber = 0;
	KillCount = 0;
	AliveZombies = 0;
	SpawnedThisWave = 0;
	TargetZombiesThisWave = 0;
}

void AZombieShootingGameMode::BeginPlay()
{
	Super::BeginPlay();

	StartNextWave();
}

void AZombieShootingGameMode::NotifyZombieKilled(ARussellZombieCharacter* Zombie)
{
	KillCount++;
	AliveZombies = FMath::Max(0, AliveZombies - 1);

	if (SpawnedThisWave >= TargetZombiesThisWave && AliveZombies <= 0)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		GetWorldTimerManager().SetTimer(NextWaveTimerHandle, this, &AZombieShootingGameMode::StartNextWave, NextWaveDelay, false);
	}
}

void AZombieShootingGameMode::RestartCurrentLevel()
{
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (!CurrentLevelName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
	}
}

void AZombieShootingGameMode::StartNextWave()
{
	WaveNumber++;
	SpawnedThisWave = 0;
	TargetZombiesThisWave = InitialZombiesPerWave + (WaveNumber - 1) * AdditionalZombiesPerWave;

	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AZombieShootingGameMode::SpawnZombie, SpawnInterval, true, 0.2f);
}

void AZombieShootingGameMode::SpawnZombie()
{
	UWorld* World = GetWorld();
	if (!World || !ZombieClass || SpawnedThisWave >= TargetZombiesThisWave)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	const FVector SpawnLocation = FindSpawnLocation();
	const FRotator SpawnRotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (World->SpawnActor<ARussellZombieCharacter>(ZombieClass, SpawnLocation, SpawnRotation, SpawnParams))
	{
		SpawnedThisWave++;
		AliveZombies++;
	}

	if (SpawnedThisWave >= TargetZombiesThisWave)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

FVector AZombieShootingGameMode::FindSpawnLocation() const
{
	const ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	const FVector Origin = PlayerCharacter ? PlayerCharacter->GetActorLocation() : FVector::ZeroVector;

	if (UWorld* World = GetWorld())
	{
		if (const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World))
		{
			for (int32 AttemptIndex = 0; AttemptIndex < 16; ++AttemptIndex)
			{
				FNavLocation NavLocation;
				if (NavSystem->GetRandomReachablePointInRadius(Origin, MaxSpawnRadius, NavLocation))
				{
					const FVector FlatOffset(NavLocation.Location.X - Origin.X, NavLocation.Location.Y - Origin.Y, 0.0f);
					if (FlatOffset.SizeSquared() >= FMath::Square(MinSpawnRadius))
					{
						return NavLocation.Location + FVector(0.0f, 0.0f, 96.0f);
					}
				}
			}
		}
	}

	const float AngleRadians = FMath::FRandRange(0.0f, TWO_PI);
	const float SpawnDistance = FMath::FRandRange(MinSpawnRadius, MaxSpawnRadius);
	FVector Candidate = Origin + FVector(FMath::Cos(AngleRadians) * SpawnDistance, FMath::Sin(AngleRadians) * SpawnDistance, 350.0f);

	if (const UWorld* World = GetWorld())
	{
		FHitResult HitResult;
		const FVector TraceStart = Candidate + FVector(0.0f, 0.0f, 1200.0f);
		const FVector TraceEnd = Candidate - FVector(0.0f, 0.0f, 2500.0f);

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RussellSpawnTrace), false);
		if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
		{
			Candidate = HitResult.Location + FVector(0.0f, 0.0f, 96.0f);
		}
	}

	return Candidate;
}
