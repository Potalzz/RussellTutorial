// Fill out your copyright notice in the Description page of Project Settings.

#include "ZombieCharacter.h"

#include "Animation/AnimSequence.h"
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
#include "CombatHealthComponent.h"
#include "ZombieShootingGameMode.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UCombatHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->MaxHealth = 80.0f;

	WalkSpeed = 180.0f;
	AttackRange = 135.0f;
	AttackDamage = 12.0f;
	AttackInterval = 1.15f;
	ZombieTickInterval = 0.1f;
	PathRefreshInterval = 0.35f;
	LastAttackTime = -1000.0f;
	LastPathRequestTime = -1000.0f;
	bIsDead = false;
	bIsSpawnAnimationActive = false;
	bShowBloodHitFX = true;
	bShowBloodDebugTrails = false;
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
	CurrentVariantId = TEXT("default_zombie");

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
		LocomotionAnimation = WalkAnimAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> AttackAnimAsset(TEXT("/Game/UndeadPack/Zombie/Animations/Anim_Attack1.Anim_Attack1"));
	if (AttackAnimAsset.Succeeded())
	{
		AttackAnimationOptions.Add(AttackAnimAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> AttackAnimAltAsset(TEXT("/Game/UndeadPack/Zombie/Animations/Anim_Attack2.Anim_Attack2"));
	if (AttackAnimAltAsset.Succeeded())
	{
		AttackAnimationOptions.Add(AttackAnimAltAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> HitAnimAsset(TEXT("/Game/UndeadPack/Zombie/Animations/Anim_Hit.Anim_Hit"));
	if (HitAnimAsset.Succeeded())
	{
		HitAnimationOptions.Add(HitAnimAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> DeathAnimAsset(TEXT("/Game/UndeadPack/Zombie/Animations/Anim_Death.Anim_Death"));
	if (DeathAnimAsset.Succeeded())
	{
		DeathAnimationOptions.Add(DeathAnimAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> BloodImpactAsset(TEXT("/Game/sA_Megapack_v1/sA_Projectilevfx/Vfx/Fx/Niagara_Systems/NS_Hit5.NS_Hit5"));
	if (BloodImpactAsset.Succeeded())
	{
		BloodImpactSystem = BloodImpactAsset.Object;
	}

	PrimaryActorTick.TickInterval = ZombieTickInterval;
}

void AZombieCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnHealthDepleted.AddDynamic(this, &AZombieCharacter::HandleHealthDepleted);
	}

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	SetActorTickInterval(ZombieTickInterval);

	AcquireTarget();
	if (!PlaySpawnAnimationIfNeeded())
	{
		ResumeLocomotionAnimation();
	}
}

void AZombieCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsDead || bIsSpawnAnimationActive)
	{
		return;
	}

	if (!TargetPlayer)
	{
		AcquireTarget();
	}

	if (TargetPlayer)
	{
		ChaseTarget();
	}
}

float AZombieCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (!HealthComponent)
	{
		return ActualDamage;
	}

	const float DamageToApply = ActualDamage > 0.0f ? ActualDamage : DamageAmount;
	const float AppliedDamage = HealthComponent->ApplyDamage(DamageToApply);

	if (AppliedDamage > 0.0f)
	{
		SpawnBloodHitFX(DamageEvent);
	}

	if (AppliedDamage > 0.0f && !bIsDead)
	{
		bIsSpawnAnimationActive = false;
		if (UAnimSequence* HitAnimation = ChooseRandomAnimation(HitAnimationOptions))
		{
			PlayAnimation(HitAnimation, false);
			const float ResumeDelay = FMath::Min(HitAnimation->GetPlayLength(), 0.35f);
			GetWorldTimerManager().SetTimer(ResumeWalkTimerHandle, this, &AZombieCharacter::ResumeLocomotionAnimation, ResumeDelay, false);
		}
	}

	return AppliedDamage > 0.0f ? AppliedDamage : ActualDamage;
}

void AZombieCharacter::HandleHealthDepleted()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	GetWorldTimerManager().ClearTimer(ResumeWalkTimerHandle);
	SetActorTickEnabled(false);
	GetCharacterMovement()->DisableMovement();

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PlayAnimation(ChooseRandomAnimation(DeathAnimationOptions), false);
	SetLifeSpan(6.0f);

	if (UWorld* World = GetWorld())
	{
		if (AZombieShootingGameMode* GameMode = World->GetAuthGameMode<AZombieShootingGameMode>())
		{
			GameMode->NotifyZombieKilled();
		}
	}
}

void AZombieCharacter::AcquireTarget()
{
	TargetPlayer = UGameplayStatics::GetPlayerCharacter(this, 0);
}

void AZombieCharacter::ChaseTarget()
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

void AZombieCharacter::AttackTarget()
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
	if (UAnimSequence* AttackAnimation = ChooseRandomAnimation(AttackAnimationOptions))
	{
		PlayAnimation(AttackAnimation, false);
		const float ResumeDelay = FMath::Min(AttackAnimation->GetPlayLength(), AttackInterval);
		GetWorldTimerManager().SetTimer(ResumeWalkTimerHandle, this, &AZombieCharacter::ResumeLocomotionAnimation, ResumeDelay, false);
	}
	TargetPlayer->TakeDamage(AttackDamage, FDamageEvent(), GetController(), this);
}

void AZombieCharacter::PlayAnimation(UAnimSequence* Animation, bool bLooping)
{
	if (!Animation || !GetMesh())
	{
		return;
	}

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	GetMesh()->PlayAnimation(Animation, bLooping);
}

void AZombieCharacter::ResumeLocomotionAnimation()
{
	bIsSpawnAnimationActive = false;

	if (!bIsDead)
	{
		PlayAnimation(LocomotionAnimation, true);
	}
}

void AZombieCharacter::SpawnBloodHitFX(const FDamageEvent& DamageEvent)
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

	if (!bShowBloodDebugTrails)
	{
		return;
	}

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

void AZombieCharacter::ApplyVariantDefinition(const FZombieVariantDefinition& VariantDefinition)
{
	CurrentVariantId = VariantDefinition.VariantId.IsNone() ? TEXT("unnamed_variant") : VariantDefinition.VariantId;

	if (USkeletalMesh* VariantMesh = VariantDefinition.SkeletalMesh.LoadSynchronous())
	{
		GetMesh()->SetSkeletalMesh(VariantMesh);
	}

	GetCapsuleComponent()->SetCapsuleSize(VariantDefinition.CapsuleRadius, VariantDefinition.CapsuleHalfHeight);
	GetMesh()->SetRelativeLocation(VariantDefinition.MeshRelativeLocation);
	GetMesh()->SetRelativeRotation(VariantDefinition.MeshRelativeRotation);
	GetMesh()->SetRelativeScale3D(VariantDefinition.MeshRelativeScale);

	if (!VariantDefinition.MaterialOverrides.IsEmpty())
	{
		for (int32 MaterialIndex = 0; MaterialIndex < VariantDefinition.MaterialOverrides.Num(); ++MaterialIndex)
		{
			if (UMaterialInterface* Material = VariantDefinition.MaterialOverrides[MaterialIndex].LoadSynchronous())
			{
				GetMesh()->SetMaterial(MaterialIndex, Material);
			}
		}
	}

	WalkSpeed = VariantDefinition.WalkSpeed;
	AttackRange = VariantDefinition.AttackRange;
	AttackDamage = VariantDefinition.AttackDamage;
	AttackInterval = VariantDefinition.AttackInterval;

	if (HealthComponent)
	{
		HealthComponent->MaxHealth = VariantDefinition.MaxHealth;
		HealthComponent->ResetHealth();
	}

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	if (!VariantDefinition.Animations.LocomotionAnimations.IsEmpty())
	{
		TArray<TObjectPtr<UAnimSequence>> LoadedLocomotionAnimations;
		LoadAnimationOptions(VariantDefinition.Animations.LocomotionAnimations, LoadedLocomotionAnimations);
		if (UAnimSequence* SelectedLocomotion = ChooseRandomAnimation(LoadedLocomotionAnimations))
		{
			LocomotionAnimation = SelectedLocomotion;
		}
	}

	if (!VariantDefinition.Animations.AttackAnimations.IsEmpty())
	{
		LoadAnimationOptions(VariantDefinition.Animations.AttackAnimations, AttackAnimationOptions);
	}

	if (!VariantDefinition.Animations.HitAnimations.IsEmpty())
	{
		LoadAnimationOptions(VariantDefinition.Animations.HitAnimations, HitAnimationOptions);
	}

	if (!VariantDefinition.Animations.DeathAnimations.IsEmpty())
	{
		LoadAnimationOptions(VariantDefinition.Animations.DeathAnimations, DeathAnimationOptions);
	}

	if (!VariantDefinition.Animations.SpawnAnimations.IsEmpty())
	{
		LoadAnimationOptions(VariantDefinition.Animations.SpawnAnimations, SpawnAnimationOptions);
	}
	else
	{
		SpawnAnimationOptions.Reset();
	}
}

UAnimSequence* AZombieCharacter::ChooseRandomAnimation(const TArray<TObjectPtr<UAnimSequence>>& AnimationOptions) const
{
	if (AnimationOptions.IsEmpty())
	{
		return nullptr;
	}

	return AnimationOptions[FMath::RandHelper(AnimationOptions.Num())];
}

void AZombieCharacter::LoadAnimationOptions(const TArray<TSoftObjectPtr<UAnimSequence>>& SoftAnimations, TArray<TObjectPtr<UAnimSequence>>& LoadedAnimations)
{
	LoadedAnimations.Reset();

	for (const TSoftObjectPtr<UAnimSequence>& SoftAnimation : SoftAnimations)
	{
		if (UAnimSequence* Animation = SoftAnimation.LoadSynchronous())
		{
			LoadedAnimations.Add(Animation);
		}
	}
}

bool AZombieCharacter::PlaySpawnAnimationIfNeeded()
{
	if (UAnimSequence* SpawnAnimation = ChooseRandomAnimation(SpawnAnimationOptions))
	{
		bIsSpawnAnimationActive = true;
		PlayAnimation(SpawnAnimation, false);
		GetWorldTimerManager().SetTimer(
			ResumeWalkTimerHandle,
			this,
			&AZombieCharacter::ResumeLocomotionAnimation,
			SpawnAnimation->GetPlayLength(),
			false);
		return true;
	}

	return false;
}

