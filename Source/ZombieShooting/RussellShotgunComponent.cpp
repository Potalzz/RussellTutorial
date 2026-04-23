// Fill out your copyright notice in the Description page of Project Settings.

#include "RussellShotgunComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

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
	bInfiniteAmmo = true;
	bShowPelletTracers = false;
	PelletTracerDuration = 0.18f;
	PelletTracerThickness = 2.5f;
	PelletTracerColor = FColor(255, 212, 64);
	MuzzleFlashScale = 1.0f;
	bDrawDebugTraces = false;
	DebugTraceDuration = 1.0f;

	CurrentAmmo = MaxAmmo;
	LastFireTime = -1000.0f;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> RocketMuzzleAsset(TEXT("/Game/sA_Megapack_v1/sA_ShootingVfxPack/FX/NiagaraSystems/NS_ROCKET_Muzzle.NS_ROCKET_Muzzle"));
	if (RocketMuzzleAsset.Succeeded())
	{
		MuzzleFlashSystem = RocketMuzzleAsset.Object;
	}
}

void URussellShotgunComponent::BeginPlay()
{
	Super::BeginPlay();

	Reload();
}

bool URussellShotgunComponent::Fire(AController* InstigatorController, const FVector& TraceStart, const FRotator& AimRotation)
{
	return FireInternal(InstigatorController, TraceStart, AimRotation, TraceStart);
}

bool URussellShotgunComponent::FireWithVisualStart(AController* InstigatorController, const FVector& TraceStart, const FRotator& AimRotation, const FVector& VisualStart)
{
	return FireInternal(InstigatorController, TraceStart, AimRotation, VisualStart);
}

bool URussellShotgunComponent::FireInternal(AController* InstigatorController, const FVector& TraceStart, const FRotator& AimRotation, const FVector& VisualStart)
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();

	if (!World || !Owner || !CanFire())
	{
		return false;
	}

	if (!bInfiniteAmmo)
	{
		CurrentAmmo = FMath::Max(0, CurrentAmmo - 1);
	}
	else
	{
		CurrentAmmo = MaxAmmo;
	}

	LastFireTime = World->GetTimeSeconds();
	SpawnMuzzleFX(World, VisualStart, AimRotation);

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

		if (bShowPelletTracers)
		{
			DrawDebugLine(World, VisualStart, DebugEnd, PelletTracerColor, false, PelletTracerDuration, 0, PelletTracerThickness);
			DrawDebugPoint(World, DebugEnd, bHit ? 9.0f : 4.0f, bHit ? FColor::Orange : PelletTracerColor, false, PelletTracerDuration, 0);
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

	const bool bHasAmmo = bInfiniteAmmo || CurrentAmmo > 0;
	return bHasAmmo && World->GetTimeSeconds() - LastFireTime >= FireInterval;
}

void URussellShotgunComponent::SpawnMuzzleFX(UWorld* World, const FVector& VisualStart, const FRotator& AimRotation) const
{
	if (!World)
	{
		return;
	}

	if (MuzzleFlashSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, MuzzleFlashSystem, VisualStart, AimRotation, FVector(MuzzleFlashScale), true, true);
	}
}
