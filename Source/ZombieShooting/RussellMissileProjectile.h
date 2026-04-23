// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RussellMissileProjectile.generated.h"

class AController;
class UNiagaraComponent;
class UNiagaraSystem;
class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class ZOMBIESHOOTING_API ARussellMissileProjectile : public AActor
{
	GENERATED_BODY()

public:
	ARussellMissileProjectile();

	void InitializeProjectile(AActor* NewOwner, AController* NewDamageInstigator, float NewDamage, float NewExplosionRadius);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Missile")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Missile")
	TObjectPtr<UStaticMeshComponent> MissileMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Missile")
	TObjectPtr<UNiagaraComponent> TrailComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Missile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missile", meta = (ClampMin = "1.0"))
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missile", meta = (ClampMin = "1.0"))
	float ExplosionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missile|FX")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missile|FX", meta = (ClampMin = "0.01"))
	float ExplosionEffectScale;

	UFUNCTION()
	void HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY()
	TObjectPtr<AController> DamageInstigator;

	bool bExploded;

	void Explode(const FVector& ExplosionLocation);
};
