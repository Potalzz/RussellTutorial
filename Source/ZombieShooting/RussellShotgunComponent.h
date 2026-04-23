// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RussellShotgunComponent.generated.h"

class ARussellMissileProjectile;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class ERussellWeaponMode : uint8
{
	Shotgun UMETA(DisplayName = "Shotgun"),
	RPG7 UMETA(DisplayName = "RPG7")
};

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	ERussellWeaponMode WeaponMode;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG7", meta = (ClampMin = "0.01"))
	float RPG7FireInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG7|Projectile")
	TSubclassOf<ARussellMissileProjectile> MissileProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG7|Projectile", meta = (ClampMin = "1.0"))
	float MissileDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG7|Projectile", meta = (ClampMin = "1.0"))
	float MissileExplosionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG7|Projectile", meta = (ClampMin = "0.0"))
	float MissileMuzzleOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG7|FX")
	TObjectPtr<UNiagaraSystem> RPG7MuzzleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG7|FX", meta = (ClampMin = "0.01"))
	float RPG7MuzzleEffectScale;

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

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWeaponMode(ERussellWeaponMode NewWeaponMode);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	ERussellWeaponMode GetWeaponMode() const { return WeaponMode; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsUsingRPG7() const { return WeaponMode == ERussellWeaponMode::RPG7; }

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Shotgun")
	int32 CurrentAmmo;

	float LastFireTime;

	bool FireInternal(AController* InstigatorController, const FVector& TraceStart, const FRotator& AimRotation, const FVector& VisualStart);
	bool FireMissile(AController* InstigatorController, const FVector& TraceStart, const FRotator& AimRotation, const FVector& VisualStart);
	void SpawnMuzzleFX(UWorld* World, const FVector& VisualStart, const FRotator& AimRotation) const;
	void SpawnNiagaraFX(UWorld* World, UNiagaraSystem* NiagaraSystem, const FVector& Location, const FRotator& Rotation, float Scale) const;
	float GetActiveFireInterval() const;
};
