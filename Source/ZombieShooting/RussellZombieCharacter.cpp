// Fill out your copyright notice in the Description page of Project Settings.

#include "RussellZombieCharacter.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "RussellHealthComponent.h"
#include "ZombieShootingGameMode.h"
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
	PathRefreshInterval = 0.35f;
	LastAttackTime = -1000.0f;
	LastPathRequestTime = -1000.0f;
	bIsDead = false;
	bShowBloodHitFX = true;
	BloodSprayCount = 18;
	BloodFXDuration = 0.2f;
	BloodFXThickness = 1.15f;
	BloodMistPointCount = 14;
	BloodMistRadius = 5.5f;
	BloodSprayDistance = 44.0f;
	BloodSpraySpreadDegrees = 56.0f;
	BloodFXColor = FColor(135, 16, 16, 96);
	BloodImpactScale = 0.24f;
	BloodImpactOpacity = 0.38f;

	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 92.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 420.0f, 0.0f);
	bUseControllerRotationYaw = false;

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

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

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> BloodImpactAsset(TEXT("/Game/sA_Megapack_v1/sA_Projectilevfx/Vfx/Fx/Niagara_Systems/NS_Hit5.NS_Hit5"));
	if (BloodImpactAsset.Succeeded())
	{
		BloodImpactSystem = BloodImpactAsset.Object;
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

	if (AppliedDamage > 0.0f)
	{
		SpawnBloodHitFX(DamageEvent);
	}

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
		if (AZombieShootingGameMode* GameMode = World->GetAuthGameMode<AZombieShootingGameMode>())
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
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			AIController->StopMovement();
		}

		AttackTarget();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			if (const UPathFollowingComponent* PathFollowingComponent = AIController->GetPathFollowingComponent())
			{
				if (PathFollowingComponent->GetStatus() == EPathFollowingStatus::Moving)
				{
					return;
				}
			}

			if (World->GetTimeSeconds() - LastPathRequestTime >= PathRefreshInterval)
			{
				LastPathRequestTime = World->GetTimeSeconds();
				const EPathFollowingRequestResult::Type MoveResult = AIController->MoveToActor(TargetPlayer, AttackRange * 0.8f, true, true, false, nullptr, true);
				if (MoveResult != EPathFollowingRequestResult::Failed)
				{
					return;
				}
			}
		}
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

void ARussellZombieCharacter::SpawnBloodHitFX(const FDamageEvent& DamageEvent)
{
	UWorld* World = GetWorld();
	if (!World || !bShowBloodHitFX)
	{
		return;
	}

	FVector ImpactPoint = GetActorLocation() + FVector(0.0f, 0.0f, 76.0f);
	FVector ShotDirection = -GetActorForwardVector();

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
		if (PointDamageEvent)
		{
			if (!PointDamageEvent->HitInfo.ImpactPoint.IsNearlyZero())
			{
				ImpactPoint = PointDamageEvent->HitInfo.ImpactPoint;
			}
			if (!PointDamageEvent->ShotDirection.IsNearlyZero())
			{
				ShotDirection = PointDamageEvent->ShotDirection.GetSafeNormal();
			}
		}
	}

	const FVector SprayBaseDirection = ShotDirection.IsNearlyZero() ? GetActorForwardVector() : -ShotDirection.GetSafeNormal();
	const float HalfAngleRadians = FMath::DegreesToRadians(BloodSpraySpreadDegrees);
	const FLinearColor BloodTint = FLinearColor(
		static_cast<float>(BloodFXColor.R) / 255.0f,
		static_cast<float>(BloodFXColor.G) / 255.0f,
		static_cast<float>(BloodFXColor.B) / 255.0f,
		BloodImpactOpacity);

	if (BloodImpactSystem)
	{
		if (UNiagaraComponent* BloodImpactComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			BloodImpactSystem,
			ImpactPoint,
			SprayBaseDirection.Rotation(),
			FVector(BloodImpactScale),
			true,
			true))
		{
			BloodImpactComponent->SetVariableLinearColor(TEXT("User.Color"), BloodTint);
			BloodImpactComponent->SetVariableLinearColor(TEXT("User.Tint"), BloodTint);
			BloodImpactComponent->SetVariableLinearColor(TEXT("User.TintColor"), BloodTint);
			BloodImpactComponent->SetVariableFloat(TEXT("User.Alpha"), BloodImpactOpacity);
		}
	}

	const FVector RightVector = FVector::CrossProduct(SprayBaseDirection, FVector::UpVector).GetSafeNormal();
	const FVector UpJitterBase = FVector::UpVector * 0.35f;

	for (int32 MistIndex = 0; MistIndex < BloodMistPointCount; ++MistIndex)
	{
		const FVector RandomOffset =
			RightVector * FMath::FRandRange(-BloodMistRadius, BloodMistRadius) +
			FVector::UpVector * FMath::FRandRange(-BloodMistRadius * 0.25f, BloodMistRadius) +
			SprayBaseDirection * FMath::FRandRange(0.0f, BloodMistRadius * 0.7f);
		const float MistSize = FMath::FRandRange(1.2f, 2.8f);
		DrawDebugPoint(World, ImpactPoint + RandomOffset, MistSize, BloodFXColor, false, BloodFXDuration, 0);
	}

	for (int32 SprayIndex = 0; SprayIndex < BloodSprayCount; ++SprayIndex)
	{
		const FVector SprayDirection = FMath::VRandCone((SprayBaseDirection + UpJitterBase).GetSafeNormal(), HalfAngleRadians);
		const float SprayLength = FMath::FRandRange(BloodSprayDistance * 0.2f, BloodSprayDistance);
		const FVector SegmentStart = ImpactPoint + SprayDirection * FMath::FRandRange(1.5f, 4.0f);
		const FVector MidPoint = SegmentStart + SprayDirection * (SprayLength * FMath::FRandRange(0.2f, 0.45f));
		const FVector SprayEnd = SegmentStart + SprayDirection * SprayLength;
		const float TrailThickness = BloodFXThickness * FMath::FRandRange(0.45f, 0.95f);

		DrawDebugLine(World, SegmentStart, MidPoint, BloodFXColor, false, BloodFXDuration, 0, TrailThickness);
		DrawDebugPoint(World, SprayEnd, FMath::FRandRange(1.5f, 3.6f), BloodFXColor, false, BloodFXDuration, 0);

		if (FMath::FRand() < 0.45f)
		{
			const FVector DropletEnd = SprayEnd + SprayDirection * FMath::FRandRange(1.5f, 6.0f);
			DrawDebugPoint(World, DropletEnd, FMath::FRandRange(1.0f, 2.4f), BloodFXColor, false, BloodFXDuration, 0);
		}
	}
}
