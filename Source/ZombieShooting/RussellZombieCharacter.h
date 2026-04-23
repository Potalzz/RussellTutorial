// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RussellZombieCharacter.generated.h"

class UAnimSequence;
class UNiagaraSystem;
class URussellHealthComponent;

UCLASS()
class ZOMBIESHOOTING_API ARussellZombieCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARussellZombieCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "Zombie")
	bool IsDead() const { return bIsDead; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URussellHealthComponent> HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float AttackInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie", meta = (ClampMin = "0.05"))
	float PathRefreshInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX")
	bool bShowBloodHitFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX", meta = (ClampMin = "1"))
	int32 BloodSprayCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX", meta = (ClampMin = "0.01"))
	float BloodFXDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX", meta = (ClampMin = "0.1"))
	float BloodFXThickness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX", meta = (ClampMin = "1"))
	int32 BloodMistPointCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX", meta = (ClampMin = "0.5"))
	float BloodMistRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX", meta = (ClampMin = "1.0"))
	float BloodSprayDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float BloodSpraySpreadDegrees;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX")
	FColor BloodFXColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX")
	TObjectPtr<UNiagaraSystem> BloodImpactSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX", meta = (ClampMin = "0.01"))
	float BloodImpactScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BloodImpactOpacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimSequence> WalkAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimSequence> AttackAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimSequence> HitAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimSequence> DeathAnimation;

	UFUNCTION()
	void HandleHealthDepleted();

	void AcquireTarget();
	void ChaseTarget(float DeltaSeconds);
	void AttackTarget();
	void PlayAnimation(UAnimSequence* Animation, bool bLooping);
	void ResumeWalkAnimation();
	void SpawnBloodHitFX(const FDamageEvent& DamageEvent);

private:
	UPROPERTY()
	TObjectPtr<ACharacter> TargetPlayer;

	FTimerHandle ResumeWalkTimerHandle;
	float LastAttackTime;
	float LastPathRequestTime;
	bool bIsDead;
};
