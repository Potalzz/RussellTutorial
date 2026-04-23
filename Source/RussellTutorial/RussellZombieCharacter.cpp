// Fill out your copyright notice in the Description page of Project Settings.

#include "RussellZombieCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "RussellHealthComponent.h"
#include "RussellTutorialGameMode.h"
#include "UObject/ConstructorHelpers.h"

ARussellZombieCharacter::ARussellZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<URussellHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->MaxHealth = 80.0f;

	WalkSpeed = 180.0f;
	AttackRange = 135.0f;
	AttackDamage = 12.0f;
	AttackInterval = 1.15f;
	LastAttackTime = -1000.0f;
	bIsDead = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 92.0f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 420.0f, 0.0f);
	bUseControllerRotationYaw = false;

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> ZombieMeshAsset(TEXT("/Game/UndeadPack/Zombie/Mesh/SK_Zombie.SK_Zombie"));
	if (ZombieMeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(ZombieMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkAnimAsset(TEXT("/Game/UndeadPack/Zombie/Animations/Anim_Walk.Anim_Walk"));
	if (WalkAnimAsset.Succeeded())
	{
		WalkAnimation = WalkAnimAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> AttackAnimAsset(TEXT("/Game/UndeadPack/Zombie/Animations/Anim_Attack1.Anim_Attack1"));
	if (AttackAnimAsset.Succeeded())
	{
		AttackAnimation = AttackAnimAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> HitAnimAsset(TEXT("/Game/UndeadPack/Zombie/Animations/Anim_Hit.Anim_Hit"));
	if (HitAnimAsset.Succeeded())
	{
		HitAnimation = HitAnimAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> DeathAnimAsset(TEXT("/Game/UndeadPack/Zombie/Animations/Anim_Death.Anim_Death"));
	if (DeathAnimAsset.Succeeded())
	{
		DeathAnimation = DeathAnimAsset.Object;
	}
}

void ARussellZombieCharacter::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHealthDepleted.AddDynamic(this, &ARussellZombieCharacter::HandleHealthDepleted);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	AcquireTarget();
	ResumeWalkAnimation();
}

void ARussellZombieCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsDead)
	{
		return;
	}

	if (!TargetPlayer)
	{
		AcquireTarget();
	}

	if (TargetPlayer)
	{
		ChaseTarget(DeltaSeconds);
	}
}

float ARussellZombieCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	const float AppliedDamage = HealthComponent->ApplyDamage(DamageAmount);

	if (AppliedDamage > 0.0f && !bIsDead && HitAnimation)
	{
		PlayAnimation(HitAnimation, false);
		GetWorldTimerManager().SetTimer(ResumeWalkTimerHandle, this, &ARussellZombieCharacter::ResumeWalkAnimation, 0.35f, false);
	}

	return AppliedDamage > 0.0f ? AppliedDamage : ActualDamage;
}

void ARussellZombieCharacter::HandleHealthDepleted()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	GetWorldTimerManager().ClearTimer(ResumeWalkTimerHandle);
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PlayAnimation(DeathAnimation, false);
	SetLifeSpan(6.0f);

	if (UWorld* World = GetWorld())
	{
		if (ARussellTutorialGameMode* GameMode = World->GetAuthGameMode<ARussellTutorialGameMode>())
		{
			GameMode->NotifyZombieKilled(this);
		}
	}
}

void ARussellZombieCharacter::AcquireTarget()
{
	TargetPlayer = UGameplayStatics::GetPlayerCharacter(this, 0);
}

void ARussellZombieCharacter::ChaseTarget(float DeltaSeconds)
{
	if (!TargetPlayer)
	{
		return;
	}

	const FVector ToTarget = TargetPlayer->GetActorLocation() - GetActorLocation();
	const FVector FlatDirection(ToTarget.X, ToTarget.Y, 0.0f);
	const float Distance = FlatDirection.Size();

	if (Distance <= AttackRange)
	{
		AttackTarget();
		return;
	}

	if (!FlatDirection.IsNearlyZero())
	{
		AddMovementInput(FlatDirection.GetSafeNormal(), 1.0f);
	}
}

void ARussellZombieCharacter::AttackTarget()
{
	UWorld* World = GetWorld();
	if (!World || !TargetPlayer)
	{
		return;
	}

	if (World->GetTimeSeconds() - LastAttackTime < AttackInterval)
	{
		return;
	}

	LastAttackTime = World->GetTimeSeconds();
	PlayAnimation(AttackAnimation, false);
	TargetPlayer->TakeDamage(AttackDamage, FDamageEvent(), GetController(), this);
	GetWorldTimerManager().SetTimer(ResumeWalkTimerHandle, this, &ARussellZombieCharacter::ResumeWalkAnimation, 0.7f, false);
}

void ARussellZombieCharacter::PlayAnimation(UAnimSequence* Animation, bool bLooping)
{
	if (!Animation || !GetMesh())
	{
		return;
	}

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	GetMesh()->PlayAnimation(Animation, bLooping);
}

void ARussellZombieCharacter::ResumeWalkAnimation()
{
	if (!bIsDead)
	{
		PlayAnimation(WalkAnimation, true);
	}
}
