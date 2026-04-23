// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RussellZombieCharacter.h"
#include "ZombieShootingGameMode.generated.h"

class ARussellWeaponPickup;

UCLASS()
class ZOMBIESHOOTING_API AZombieShootingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AZombieShootingGameMode();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves|Variants")
	TArray<FRussellZombieVariantDefinition> ZombieVariants;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup")
	TSubclassOf<ARussellWeaponPickup> WeaponPickupClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup", meta = (ClampMin = "0.0"))
	float WeaponPickupSpawnDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup", meta = (ClampMin = "100.0"))
	float WeaponPickupSpawnRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bApplyPerformanceProfile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0", ClampMax = "4"))
	int32 PerformanceOverallQualityLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "50.0", ClampMax = "100.0"))
	float PerformanceResolutionQuality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0", ClampMax = "4"))
	int32 PerformanceShadowQuality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0", ClampMax = "4"))
	int32 PerformanceGlobalIlluminationQuality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0", ClampMax = "4"))
	int32 PerformanceReflectionQuality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0", ClampMax = "4"))
	int32 PerformancePostProcessQuality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0", ClampMax = "4"))
	int32 PerformanceFoliageQuality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.25", ClampMax = "1.0"))
	float PerformanceShadowDistanceScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bDisableMotionBlur;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bDisableContactShadows;

	void StartNextWave();
	void SpawnZombie();
	void SpawnWeaponPickup();
	FVector FindSpawnLocation() const;
	FVector FindWeaponPickupLocation() const;
	const FRussellZombieVariantDefinition* ChooseZombieVariant() const;
	void BuildDefaultZombieVariants();
	void ApplyPerformanceProfile();

private:
	FTimerHandle SpawnTimerHandle;
	FTimerHandle NextWaveTimerHandle;
	FTimerHandle WeaponPickupTimerHandle;

	int32 WaveNumber;
	int32 KillCount;
	int32 AliveZombies;
	int32 SpawnedThisWave;
	int32 TargetZombiesThisWave;
};
