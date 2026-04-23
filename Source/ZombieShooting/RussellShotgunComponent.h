// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RussellShotgunComponent.generated.h"

class UNiagaraSystem;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun")
	bool bInfiniteAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun|FX")
	bool bShowPelletTracers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun|FX", meta = (ClampMin = "0.01"))
	float PelletTracerDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun|FX", meta = (ClampMin = "0.1"))
	float PelletTracerThickness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun|FX")
	FColor PelletTracerColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun|FX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shotgun|FX", meta = (ClampMin = "0.01"))
	float MuzzleFlashScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebugTraces;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (ClampMin = "0.0"))
	float DebugTraceDuration;

	UFUNCTION(BlueprintCallable, Category = "Shotgun")
	bool Fire(AController* InstigatorController, const FVector& TraceStart, const FRotator& AimRotation);

	UFUNCTION(BlueprintCallable, Category = "Shotgun")
	bool FireWithVisualStart(AController* InstigatorController, const FVector& TraceStart, const FRotator& AimRotation, const FVector& VisualStart);

	UFUNCTION(BlueprintCallable, Category = "Shotgun")
	void Reload();

	UFUNCTION(BlueprintPure, Category = "Shotgun")
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = "Shotgun")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintPure, Category = "Shotgun")
	int32 GetMaxAmmo() const { return MaxAmmo; }

	UFUNCTION(BlueprintPure, Category = "Shotgun")
	bool HasInfiniteAmmo() const { return bInfiniteAmmo; }

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Shotgun")
	int32 CurrentAmmo;

	float LastFireTime;

	bool FireInternal(AController* InstigatorController, const FVector& TraceStart, const FRotator& AimRotation, const FVector& VisualStart);
	void SpawnMuzzleFX(UWorld* World, const FVector& VisualStart, const FRotator& AimRotation) const;
};
