// Fill out your copyright notice in the Description page of Project Settings.

#include "RussellShotgunComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"

URussellShotgunComponent::URussellShotgunComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	DamagePerPellet = 12.0f;
	PelletCount = 8;
	SpreadDegrees = 4.0f;
	Range = 6500.0f;
	FireInterval = 0.75f;
	MaxAmmo = 8;
	bInfiniteReserve = true;
	bDrawDebugTraces = false;
	DebugTraceDuration = 1.0f;

	CurrentAmmo = MaxAmmo;
	LastFireTime = -1000.0f;
}

void URussellShotgunComponent::BeginPlay()
{
	Super::BeginPlay();

	Reload();
}

bool URussellShotgunComponent::Fire(AController* InstigatorController, const FVector& TraceStart, const FRotator& AimRotation)
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();

	if (!World || !Owner || !CanFire())
	{
		return false;
	}

	CurrentAmmo = FMath::Max(0, CurrentAmmo - 1);
	LastFireTime = World->GetTimeSeconds();

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RussellShotgunTrace), true, Owner);
	QueryParams.AddIgnoredActor(Owner);

	const float HalfAngleRadians = FMath::DegreesToRadians(SpreadDegrees);

	for (int32 PelletIndex = 0; PelletIndex < PelletCount; ++PelletIndex)
	{
		const FVector ShotDirection = FMath::VRandCone(AimRotation.Vector(), HalfAngleRadians);
		const FVector TraceEnd = TraceStart + (ShotDirection * Range);

		FHitResult HitResult;
		const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
		const FVector DebugEnd = bHit ? HitResult.ImpactPoint : TraceEnd;

		if (bHit)
		{
			if (AActor* HitActor = HitResult.GetActor())
			{
				FPointDamageEvent DamageEvent(DamagePerPellet, HitResult, ShotDirection, UDamageType::StaticClass());
				HitActor->TakeDamage(DamagePerPellet, DamageEvent, InstigatorController, Owner);
			}
		}

		if (bDrawDebugTraces)
		{
			DrawDebugLine(World, TraceStart, DebugEnd, bHit ? FColor::Red : FColor::Green, false, DebugTraceDuration, 0, 1.0f);
		}
	}

	if (bInfiniteReserve && CurrentAmmo <= 0)
	{
		Reload();
	}

	return true;
}

void URussellShotgunComponent::Reload()
{
	CurrentAmmo = MaxAmmo;
}

bool URussellShotgunComponent::CanFire() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return CurrentAmmo > 0 && World->GetTimeSeconds() - LastFireTime >= FireInterval;
}
