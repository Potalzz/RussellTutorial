// Fill out your copyright notice in the Description page of Project Settings.

#include "RussellMissileProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

ARussellMissileProjectile::ARussellMissileProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 5.0f;

	Damage = 120.0f;
	ExplosionRadius = 350.0f;
	ExplosionEffectScale = 0.8f;
	bExploded = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->InitSphereRadius(14.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->CanCharacterStepUpOn = ECB_No;
	SetRootComponent(CollisionComponent);

	MissileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MissileMesh"));
	MissileMeshComponent->SetupAttachment(CollisionComponent);
	MissileMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MissileMeshComponent->CastShadow = false;

	TrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailFX"));
	TrailComponent->SetupAttachment(CollisionComponent);
	TrailComponent->bAutoActivate = true;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
	ProjectileMovementComponent->InitialSpeed = 2600.0f;
	ProjectileMovementComponent->MaxSpeed = 2600.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> RocketMeshAsset(TEXT("/Game/sA_Megapack_v1/sA_ShootingVfxPack/Meshes/SM_Rocket.SM_Rocket"));
	if (RocketMeshAsset.Succeeded())
	{
		MissileMeshComponent->SetStaticMesh(RocketMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> TrailAsset(TEXT("/Game/sA_Megapack_v1/sA_ShootingVfxPack/FX/NiagaraSystems/NS_ROCKET_Trail.NS_ROCKET_Trail"));
	if (TrailAsset.Succeeded())
	{
		TrailComponent->SetAsset(TrailAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> ExplosionAsset(TEXT("/Game/sA_Megapack_v1/sA_ShootingVfxPack/FX/NiagaraSystems/NS_Explossion.NS_Explossion"));
	if (ExplosionAsset.Succeeded())
	{
		ExplosionEffect = ExplosionAsset.Object;
	}

	CollisionComponent->OnComponentHit.AddDynamic(this, &ARussellMissileProjectile::HandleHit);
}

void ARussellMissileProjectile::InitializeProjectile(AActor* NewOwner, AController* NewDamageInstigator, float NewDamage, float NewExplosionRadius)
{
	if (NewOwner)
	{
		SetOwner(NewOwner);

		if (APawn* OwnerPawn = Cast<APawn>(NewOwner))
		{
			SetInstigator(OwnerPawn);
		}

		CollisionComponent->IgnoreActorWhenMoving(NewOwner, true);
		MissileMeshComponent->IgnoreActorWhenMoving(NewOwner, true);
	}

	DamageInstigator = NewDamageInstigator;
	Damage = NewDamage;
	ExplosionRadius = NewExplosionRadius;
}

void ARussellMissileProjectile::HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bExploded || OtherActor == GetOwner())
	{
		return;
	}

	const FVector ExplosionLocation = Hit.ImpactPoint.IsNearlyZero() ? GetActorLocation() : FVector(Hit.ImpactPoint);
	Explode(ExplosionLocation);
}

void ARussellMissileProjectile::Explode(const FVector& ExplosionLocation)
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	UWorld* World = GetWorld();
	if (World)
	{
		TArray<AActor*> IgnoredActors;
		if (AActor* OwnerActor = GetOwner())
		{
			IgnoredActors.Add(OwnerActor);
		}

		UGameplayStatics::ApplyRadialDamage(this, Damage, ExplosionLocation, ExplosionRadius, UDamageType::StaticClass(), IgnoredActors, this, DamageInstigator.Get(), true);

		if (ExplosionEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, ExplosionEffect, ExplosionLocation, FRotator::ZeroRotator, FVector(ExplosionEffectScale), true, true);
		}
	}

	Destroy();
}
