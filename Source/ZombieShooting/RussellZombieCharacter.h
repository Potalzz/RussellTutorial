// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RussellZombieCharacter.generated.h"

class UAnimSequence;
class UMaterialInterface;
class UNiagaraSystem;
class URussellHealthComponent;
class USkeletalMesh;

USTRUCT(BlueprintType)
struct FRussellZombieAnimationSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<TSoftObjectPtr<UAnimSequence>> LocomotionAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<TSoftObjectPtr<UAnimSequence>> AttackAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<TSoftObjectPtr<UAnimSequence>> HitAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<TSoftObjectPtr<UAnimSequence>> DeathAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<TSoftObjectPtr<UAnimSequence>> SpawnAnimations;
};

USTRUCT(BlueprintType)
struct FRussellZombieVariantDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant")
	FName VariantId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant", meta = (ClampMin = "1"))
	int32 SpawnWeight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant")
	TArray<TSoftObjectPtr<UMaterialInterface>> MaterialOverrides;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant")
	FRussellZombieAnimationSet Animations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant", meta = (ClampMin = "1.0"))
	float MaxHealth = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant", meta = (ClampMin = "10.0"))
	float WalkSpeed = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant", meta = (ClampMin = "10.0"))
	float AttackRange = 135.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant", meta = (ClampMin = "1.0"))
	float AttackDamage = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant", meta = (ClampMin = "0.1"))
	float AttackInterval = 1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant", meta = (ClampMin = "10.0"))
	float CapsuleRadius = 42.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant", meta = (ClampMin = "10.0"))
	float CapsuleHalfHeight = 92.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant")
	FVector MeshRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant")
	FRotator MeshRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant")
	FVector MeshRelativeScale = FVector(1.0f, 1.0f, 1.0f);
};

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

	UFUNCTION(BlueprintCallable, Category = "Zombie")
	void ApplyVariantDefinition(const FRussellZombieVariantDefinition& VariantDefinition);

	UFUNCTION(BlueprintPure, Category = "Zombie")
	FName GetVariantId() const { return CurrentVariantId; }

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Performance", meta = (ClampMin = "0.01"))
	float ZombieTickInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie", meta = (ClampMin = "0.05"))
	float PathRefreshInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX")
	bool bShowBloodHitFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|FX")
	bool bShowBloodDebugTrails;

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

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Zombie")
	FName CurrentVariantId;

	UFUNCTION()
	void HandleHealthDepleted();

	void AcquireTarget();
	void ChaseTarget(float DeltaSeconds);
	void AttackTarget();
	void PlayAnimation(UAnimSequence* Animation, bool bLooping);
	void ResumeLocomotionAnimation();
	void SpawnBloodHitFX(const FDamageEvent& DamageEvent);
	UAnimSequence* ChooseRandomAnimation(const TArray<TObjectPtr<UAnimSequence>>& AnimationOptions) const;
	void LoadAnimationOptions(const TArray<TSoftObjectPtr<UAnimSequence>>& SoftAnimations, TArray<TObjectPtr<UAnimSequence>>& LoadedAnimations);
	bool PlaySpawnAnimationIfNeeded();

private:
	UPROPERTY()
	TObjectPtr<ACharacter> TargetPlayer;

	UPROPERTY()
	TObjectPtr<UAnimSequence> LocomotionAnimation;

	UPROPERTY()
	TArray<TObjectPtr<UAnimSequence>> AttackAnimationOptions;

	UPROPERTY()
	TArray<TObjectPtr<UAnimSequence>> HitAnimationOptions;

	UPROPERTY()
	TArray<TObjectPtr<UAnimSequence>> DeathAnimationOptions;

	UPROPERTY()
	TArray<TObjectPtr<UAnimSequence>> SpawnAnimationOptions;

	FTimerHandle ResumeWalkTimerHandle;
	float LastAttackTime;
	float LastPathRequestTime;
	bool bIsDead;
	bool bIsSpawnAnimationActive;
};
