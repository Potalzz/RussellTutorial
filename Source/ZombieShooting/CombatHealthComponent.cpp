// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatHealthComponent.h"

UCombatHealthComponent::UCombatHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	bIsDead = false;
}

void UCombatHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	ResetHealth();
}

void UCombatHealthComponent::ResetHealth()
{
	bIsDead = false;
	CurrentHealth = MaxHealth;
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

float UCombatHealthComponent::ApplyDamage(float DamageAmount)
{
	if (bIsDead || DamageAmount <= 0.0f)
	{
		return 0.0f;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	const float ActualDamage = PreviousHealth - CurrentHealth;

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		OnHealthDepleted.Broadcast();
	}

	return ActualDamage;
}

float UCombatHealthComponent::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

