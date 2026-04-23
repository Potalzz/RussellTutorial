// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RussellTutorialGameMode.generated.h"

class ARussellZombieCharacter;

UCLASS()
class RUSSELLTUTORIAL_API ARussellTutorialGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARussellTutorialGameMode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Waves")
	void NotifyZombieKilled(ARussellZombieCharacter* Zombie);

	UFUNCTION(BlueprintCallable, Category = "Waves")
	void RestartCurrentLevel();

	UFUNCTION(BlueprintPure, Category = "Waves")
	int32 GetWaveNumber() const { return WaveNumber; }

	UFUNCTION(BlueprintPure, Category = "Waves")
	int32 GetKillCount() const { return KillCount; }

	UFUNCTION(BlueprintPure, Category = "Waves")
	int32 GetAliveZombieCount() const { return AliveZombies; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	TSubclassOf<ARussellZombieCharacter> ZombieClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves", meta = (ClampMin = "1"))
	int32 InitialZombiesPerWave;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves", meta = (ClampMin = "0"))
	int32 AdditionalZombiesPerWave;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves", meta = (ClampMin = "0.1"))
	float SpawnInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves", meta = (ClampMin = "200.0"))
	float MinSpawnRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves", meta = (ClampMin = "300.0"))
	float MaxSpawnRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves", meta = (ClampMin = "0.0"))
	float NextWaveDelay;

	void StartNextWave();
	void SpawnZombie();
	FVector FindSpawnLocation() const;

private:
	FTimerHandle SpawnTimerHandle;
	FTimerHandle NextWaveTimerHandle;

	int32 WaveNumber;
	int32 KillCount;
	int32 AliveZombies;
	int32 SpawnedThisWave;
	int32 TargetZombiesThisWave;
};
