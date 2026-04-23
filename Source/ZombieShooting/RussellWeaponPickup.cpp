// Fill out your copyright notice in the Description page of Project Settings.

#include "RussellWeaponPickup.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "RussellFirstPersonCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARussellWeaponPickup::ARussellWeaponPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	SetCanBeDamaged(false);

	CollisionRadius = 120.0f;
	bConsumed = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootSceneComponent);

	PickupCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	PickupCollisionComponent->SetupAttachment(RootSceneComponent);
	PickupCollisionComponent->InitSphereRadius(CollisionRadius);
	PickupCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupCollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	PickupCollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupCollisionComponent->SetGenerateOverlapEvents(true);

	PickupFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupFX"));
	PickupFXComponent->SetupAttachment(RootSceneComponent);
	PickupFXComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 18.0f));
	PickupFXComponent->bAutoActivate = true;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> PickupFXAsset(TEXT("/Game/sA_Megapack_v1/sA_PickupSet_1/Fx/NiagaraSystems/NS_Pickup_1.NS_Pickup_1"));
	if (PickupFXAsset.Succeeded())
	{
		PickupFXComponent->SetAsset(PickupFXAsset.Object);
	}

	PickupCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ARussellWeaponPickup::HandlePickupOverlap);
}

void ARussellWeaponPickup::HandlePickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bConsumed)
	{
		return;
	}

	ARussellFirstPersonCharacter* PlayerCharacter = Cast<ARussellFirstPersonCharacter>(OtherActor);
	if (!PlayerCharacter || PlayerCharacter->IsDead())
	{
		return;
	}

	bConsumed = true;
	PlayerCharacter->EquipRPG7();
	Destroy();
}
