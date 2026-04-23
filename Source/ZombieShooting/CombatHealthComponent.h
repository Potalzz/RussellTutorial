// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHealthChangedSignature, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHealthDepletedSignature);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class ZOMBIESHOOTING_API UCombatHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatHealthComponent();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FHealthDepletedSignature OnHealthDepleted;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

	UFUNCTION(BlueprintCallable, Category = "Health")
	float ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bIsDead; }

private:
	bool bIsDead;
};

