// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RussellShotgunComponent.generated.h"

UCLASS(ClassGroup=(Russell), meta=(BlueprintSpawnableComponent))
class ZOMBIESHOOTING_API URussellShotgunComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URussellShotgunComponent();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun", meta = (ClampMin = "1.0"))
	float DamagePerPellet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun", meta = (ClampMin = "1"))
	int32 PelletCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun", meta = (ClampMin = "0.0"))
	float SpreadDegrees;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun", meta = (ClampMin = "100.0"))
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun", meta = (ClampMin = "0.01"))
	float FireInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun", meta = (ClampMin = "1"))
	int32 MaxAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun")
	bool bInfiniteReserve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebugTraces;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (ClampMin = "0.0"))
	float DebugTraceDuration;

	UFUNCTION(BlueprintCallable, Category = "Shotgun")
	bool Fire(AController* InstigatorController, const FVector& TraceStart, const FRotator& AimRotation);

	UFUNCTION(BlueprintCallable, Category = "Shotgun")
	void Reload();

	UFUNCTION(BlueprintPure, Category = "Shotgun")
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = "Shotgun")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintPure, Category = "Shotgun")
	int32 GetMaxAmmo() const { return MaxAmmo; }

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Shotgun")
	int32 CurrentAmmo;

	float LastFireTime;
};
