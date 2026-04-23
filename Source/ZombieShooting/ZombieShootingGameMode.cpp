// Fill out your copyright notice in the Description page of Project Settings.

#include "ZombieShootingGameMode.h"

#include "Animation/AnimSequence.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "NavigationSystem.h"
#include "ZombiePlayerCharacter.h"
#include "SurvivalHUD.h"
#include "WeaponPickup.h"
#include "ZombieCharacter.h"
#include "Scalability.h"

namespace
{
	template <typename AssetType>
	TSoftObjectPtr<AssetType> MakeSoftAsset(const TCHAR* AssetPath)
	{
		return TSoftObjectPtr<AssetType>(FSoftObjectPath(AssetPath));
	}

	TArray<TSoftObjectPtr<UAnimSequence>> MakeAnimArray(std::initializer_list<const TCHAR*> AssetPaths)
	{
		TArray<TSoftObjectPtr<UAnimSequence>> Result;
		Result.Reserve(static_cast<int32>(AssetPaths.size()));
		for (const TCHAR* AssetPath : AssetPaths)
		{
			Result.Add(MakeSoftAsset<UAnimSequence>(AssetPath));
		}
		return Result;
	}

	TArray<TSoftObjectPtr<UMaterialInterface>> MakeMaterialArray(std::initializer_list<const TCHAR*> AssetPaths)
	{
		TArray<TSoftObjectPtr<UMaterialInterface>> Result;
		Result.Reserve(static_cast<int32>(AssetPaths.size()));
		for (const TCHAR* AssetPath : AssetPaths)
		{
			Result.Add(MakeSoftAsset<UMaterialInterface>(AssetPath));
		}
		return Result;
	}

	void WarmUpAnimationAssets(const TArray<TSoftObjectPtr<UAnimSequence>>& Animations)
	{
		for (const TSoftObjectPtr<UAnimSequence>& Animation : Animations)
		{
			Animation.LoadSynchronous();
		}
	}
}

AZombieShootingGameMode::AZombieShootingGameMode()
{
	DefaultPawnClass = AZombiePlayerCharacter::StaticClass();
	HUDClass = ASurvivalHUD::StaticClass();

	ZombieClass = AZombieCharacter::StaticClass();
	InitialZombiesPerWave = 4;
	AdditionalZombiesPerWave = 2;
	SpawnInterval = 1.25f;
	MinSpawnRadius = 1200.0f;
	MaxSpawnRadius = 2200.0f;
	NextWaveDelay = 4.0f;
	WeaponPickupClass = AWeaponPickup::StaticClass();
	WeaponPickupSpawnDelay = 15.0f;
	WeaponPickupSpawnRadius = 1800.0f;
	bApplyPerformanceProfile = true;
	PerformanceOverallQualityLevel = 2;
	PerformanceResolutionQuality = 85.0f;
	PerformanceShadowQuality = 1;
	PerformanceGlobalIlluminationQuality = 1;
	PerformanceReflectionQuality = 1;
	PerformancePostProcessQuality = 1;
	PerformanceFoliageQuality = 1;
	PerformanceShadowDistanceScale = 0.65f;
	bDisableMotionBlur = true;
	bDisableContactShadows = true;
	bUseMacBookAirPerformanceProfile = true;
	MacPerformanceOverallQualityLevel = 1;
	MacPerformanceResolutionQuality = 75.0f;
	MacPerformanceShadowQuality = 1;
	MacPerformanceGlobalIlluminationQuality = 0;
	MacPerformanceReflectionQuality = 0;
	MacPerformanceShadowDistanceScale = 0.5f;

	WaveNumber = 0;
	KillCount = 0;
	AliveZombies = 0;
	SpawnedThisWave = 0;
	TargetZombiesThisWave = 0;

	BuildDefaultZombieVariants();
}

void AZombieShootingGameMode::BeginPlay()
{
	Super::BeginPlay();

	WarmUpZombieVariantAssets();
	ApplyPerformanceProfile();
	StartNextWave();

	if (WeaponPickupClass && WeaponPickupSpawnDelay >= 0.0f)
	{
		GetWorldTimerManager().SetTimer(WeaponPickupTimerHandle, this, &AZombieShootingGameMode::SpawnWeaponPickup, WeaponPickupSpawnDelay, false);
	}
}

void AZombieShootingGameMode::NotifyZombieKilled()
{
	KillCount++;
	AliveZombies = FMath::Max(0, AliveZombies - 1);

	if (SpawnedThisWave >= TargetZombiesThisWave && AliveZombies <= 0)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		GetWorldTimerManager().SetTimer(NextWaveTimerHandle, this, &AZombieShootingGameMode::StartNextWave, NextWaveDelay, false);
	}
}

void AZombieShootingGameMode::RestartCurrentLevel()
{
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (!CurrentLevelName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
	}
}

void AZombieShootingGameMode::StartNextWave()
{
	WaveNumber++;
	SpawnedThisWave = 0;
	TargetZombiesThisWave = InitialZombiesPerWave + (WaveNumber - 1) * AdditionalZombiesPerWave;

	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AZombieShootingGameMode::SpawnZombie, SpawnInterval, true, 0.2f);
}

void AZombieShootingGameMode::SpawnZombie()
{
	UWorld* World = GetWorld();
	if (!World || !ZombieClass || SpawnedThisWave >= TargetZombiesThisWave)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	const FVector SpawnLocation = FindSpawnLocation();
	const FRotator SpawnRotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);
	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	if (AZombieCharacter* SpawnedZombie = World->SpawnActorDeferred<AZombieCharacter>(
		ZombieClass,
		SpawnTransform,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn))
	{
		if (const FZombieVariantDefinition* VariantDefinition = ChooseZombieVariant())
		{
			SpawnedZombie->ApplyVariantDefinition(*VariantDefinition);
		}

		UGameplayStatics::FinishSpawningActor(SpawnedZombie, SpawnTransform);
		SpawnedThisWave++;
		AliveZombies++;
	}

	if (SpawnedThisWave >= TargetZombiesThisWave)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

void AZombieShootingGameMode::SpawnWeaponPickup()
{
	UWorld* World = GetWorld();
	if (!World || !WeaponPickupClass)
	{
		return;
	}

	const FVector SpawnLocation = FindWeaponPickupLocation();
	const FRotator SpawnRotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	World->SpawnActor<AWeaponPickup>(WeaponPickupClass, SpawnLocation, SpawnRotation, SpawnParams);
}

FVector AZombieShootingGameMode::FindSpawnLocation() const
{
	const ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	const FVector Origin = PlayerCharacter ? PlayerCharacter->GetActorLocation() : FVector::ZeroVector;

	if (UWorld* World = GetWorld())
	{
		if (const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World))
		{
			for (int32 AttemptIndex = 0; AttemptIndex < 16; ++AttemptIndex)
			{
				FNavLocation NavLocation;
				if (NavSystem->GetRandomReachablePointInRadius(Origin, MaxSpawnRadius, NavLocation))
				{
					const FVector FlatOffset(NavLocation.Location.X - Origin.X, NavLocation.Location.Y - Origin.Y, 0.0f);
					if (FlatOffset.SizeSquared() >= FMath::Square(MinSpawnRadius))
					{
						return NavLocation.Location + FVector(0.0f, 0.0f, 96.0f);
					}
				}
			}
		}
	}

	const float AngleRadians = FMath::FRandRange(0.0f, TWO_PI);
	const float SpawnDistance = FMath::FRandRange(MinSpawnRadius, MaxSpawnRadius);
	FVector Candidate = Origin + FVector(FMath::Cos(AngleRadians) * SpawnDistance, FMath::Sin(AngleRadians) * SpawnDistance, 350.0f);

	if (const UWorld* World = GetWorld())
	{
		FHitResult HitResult;
		const FVector TraceStart = Candidate + FVector(0.0f, 0.0f, 1200.0f);
		const FVector TraceEnd = Candidate - FVector(0.0f, 0.0f, 2500.0f);

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ZombieSpawnTrace), false);
		if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
		{
			Candidate = HitResult.Location + FVector(0.0f, 0.0f, 96.0f);
		}
	}

	return Candidate;
}

FVector AZombieShootingGameMode::FindWeaponPickupLocation() const
{
	const ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	const FVector Origin = PlayerCharacter ? PlayerCharacter->GetActorLocation() : FVector::ZeroVector;

	UWorld* World = GetWorld();
	if (!World)
	{
		return Origin;
	}

	FVector Candidate = Origin;
	if (const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World))
	{
		FNavLocation NavLocation;
		if (NavSystem->GetRandomReachablePointInRadius(Origin, WeaponPickupSpawnRadius, NavLocation))
		{
			Candidate = NavLocation.Location;
		}
	}

	if (Candidate.Equals(Origin))
	{
		const float AngleRadians = FMath::FRandRange(0.0f, TWO_PI);
		const float SpawnDistance = FMath::FRandRange(300.0f, WeaponPickupSpawnRadius);
		Candidate = Origin + FVector(FMath::Cos(AngleRadians) * SpawnDistance, FMath::Sin(AngleRadians) * SpawnDistance, 250.0f);
	}

	FHitResult HitResult;
	const FVector TraceStart = Candidate + FVector(0.0f, 0.0f, 1200.0f);
	const FVector TraceEnd = Candidate - FVector(0.0f, 0.0f, 2500.0f);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WeaponPickupSpawnTrace), false);
	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		return HitResult.Location + FVector(0.0f, 0.0f, 18.0f);
	}

	return Candidate + FVector(0.0f, 0.0f, 18.0f);
}

const FZombieVariantDefinition* AZombieShootingGameMode::ChooseZombieVariant() const
{
	if (ZombieVariants.IsEmpty())
	{
		return nullptr;
	}

	int32 TotalWeight = 0;
	for (const FZombieVariantDefinition& VariantDefinition : ZombieVariants)
	{
		TotalWeight += FMath::Max(1, VariantDefinition.SpawnWeight);
	}

	if (TotalWeight <= 0)
	{
		return &ZombieVariants[0];
	}

	int32 Pick = FMath::RandRange(0, TotalWeight - 1);
	for (const FZombieVariantDefinition& VariantDefinition : ZombieVariants)
	{
		Pick -= FMath::Max(1, VariantDefinition.SpawnWeight);
		if (Pick < 0)
		{
			return &VariantDefinition;
		}
	}

	return &ZombieVariants.Last();
}

void AZombieShootingGameMode::ApplyPerformanceProfile()
{
	if (!bApplyPerformanceProfile || !GetWorld())
	{
		return;
	}

	int32 OverallQualityLevel = PerformanceOverallQualityLevel;
	float ResolutionQuality = PerformanceResolutionQuality;
	int32 ShadowQuality = PerformanceShadowQuality;
	int32 GlobalIlluminationQuality = PerformanceGlobalIlluminationQuality;
	int32 ReflectionQuality = PerformanceReflectionQuality;
	float ShadowDistanceScale = PerformanceShadowDistanceScale;

#if PLATFORM_MAC
	if (bUseMacBookAirPerformanceProfile)
	{
		OverallQualityLevel = MacPerformanceOverallQualityLevel;
		ResolutionQuality = MacPerformanceResolutionQuality;
		ShadowQuality = MacPerformanceShadowQuality;
		GlobalIlluminationQuality = MacPerformanceGlobalIlluminationQuality;
		ReflectionQuality = MacPerformanceReflectionQuality;
		ShadowDistanceScale = MacPerformanceShadowDistanceScale;
	}
#endif

	Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();
	QualityLevels.SetFromSingleQualityLevel(FMath::Clamp(OverallQualityLevel, 0, 4));
	QualityLevels.ResolutionQuality = ResolutionQuality;
	QualityLevels.SetShadowQuality(FMath::Clamp(ShadowQuality, 0, 4));
	QualityLevels.SetGlobalIlluminationQuality(FMath::Clamp(GlobalIlluminationQuality, 0, 4));
	QualityLevels.SetReflectionQuality(FMath::Clamp(ReflectionQuality, 0, 4));
	QualityLevels.SetPostProcessQuality(FMath::Clamp(PerformancePostProcessQuality, 0, 4));
	QualityLevels.SetFoliageQuality(FMath::Clamp(PerformanceFoliageQuality, 0, 4));
	Scalability::SetQualityLevels(QualityLevels, true);

	if (!GEngine)
	{
		return;
	}

	GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.ScreenPercentage %.0f"), ResolutionQuality));
	GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.Shadow.DistanceScale %.2f"), ShadowDistanceScale));

	if (bDisableMotionBlur)
	{
		GEngine->Exec(GetWorld(), TEXT("r.MotionBlurQuality 0"));
	}

	if (bDisableContactShadows)
	{
		GEngine->Exec(GetWorld(), TEXT("r.ContactShadows 0"));
	}
}

void AZombieShootingGameMode::WarmUpZombieVariantAssets()
{
	for (const FZombieVariantDefinition& VariantDefinition : ZombieVariants)
	{
		VariantDefinition.SkeletalMesh.LoadSynchronous();

		for (const TSoftObjectPtr<UMaterialInterface>& MaterialOverride : VariantDefinition.MaterialOverrides)
		{
			MaterialOverride.LoadSynchronous();
		}

		WarmUpAnimationAssets(VariantDefinition.Animations.LocomotionAnimations);
		WarmUpAnimationAssets(VariantDefinition.Animations.AttackAnimations);
		WarmUpAnimationAssets(VariantDefinition.Animations.HitAnimations);
		WarmUpAnimationAssets(VariantDefinition.Animations.DeathAnimations);
		WarmUpAnimationAssets(VariantDefinition.Animations.SpawnAnimations);
	}
}

void AZombieShootingGameMode::BuildDefaultZombieVariants()
{
	if (!ZombieVariants.IsEmpty())
	{
		return;
	}

	FZombieVariantDefinition WalkerZombie;
	WalkerZombie.VariantId = TEXT("walker_zombie");
	WalkerZombie.SpawnWeight = 4;
	WalkerZombie.SkeletalMesh = MakeSoftAsset<USkeletalMesh>(TEXT("/Game/UndeadPack/Zombie/Mesh/SK_Zombie.SK_Zombie"));
	WalkerZombie.Animations.LocomotionAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/Zombie/Animations/Anim_Walk.Anim_Walk"),
	});
	WalkerZombie.Animations.AttackAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/Zombie/Animations/Anim_Attack1.Anim_Attack1"),
		TEXT("/Game/UndeadPack/Zombie/Animations/Anim_Attack2.Anim_Attack2"),
	});
	WalkerZombie.Animations.HitAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/Zombie/Animations/Anim_Hit.Anim_Hit"),
	});
	WalkerZombie.Animations.DeathAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/Zombie/Animations/Anim_Death.Anim_Death"),
	});
	WalkerZombie.MaxHealth = 80.0f;
	WalkerZombie.WalkSpeed = 180.0f;
	WalkerZombie.AttackRange = 135.0f;
	WalkerZombie.AttackDamage = 12.0f;
	WalkerZombie.AttackInterval = 1.15f;
	WalkerZombie.CapsuleRadius = 42.0f;
	WalkerZombie.CapsuleHalfHeight = 92.0f;
	WalkerZombie.MeshRelativeLocation = FVector(0.0f, 0.0f, -92.0f);
	WalkerZombie.MeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	ZombieVariants.Add(WalkerZombie);

	FZombieVariantDefinition RotterZombie = WalkerZombie;
	RotterZombie.VariantId = TEXT("rotter_zombie");
	RotterZombie.SpawnWeight = 3;
	RotterZombie.MaterialOverrides = MakeMaterialArray({
		TEXT("/Game/UndeadPack/Zombie/Materials/MI_Zombie_2.MI_Zombie_2"),
	});
	RotterZombie.WalkSpeed = 205.0f;
	RotterZombie.AttackInterval = 1.0f;
	RotterZombie.MaxHealth = 72.0f;
	ZombieVariants.Add(RotterZombie);

	FZombieVariantDefinition FeralGhoul;
	FeralGhoul.VariantId = TEXT("feral_ghoul");
	FeralGhoul.SpawnWeight = 3;
	FeralGhoul.SkeletalMesh = MakeSoftAsset<USkeletalMesh>(TEXT("/Game/UndeadPack/Ghoul/Mesh/SK_Ghoul.SK_Ghoul"));
	FeralGhoul.MaterialOverrides = MakeMaterialArray({
		TEXT("/Game/UndeadPack/Ghoul/Materials/MI_Ghoul.MI_Ghoul"),
	});
	FeralGhoul.Animations.LocomotionAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/Ghoul/Animations/Anim_Run.Anim_Run"),
	});
	FeralGhoul.Animations.AttackAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/Ghoul/Animations/Anim_Attack_Left.Anim_Attack_Left"),
		TEXT("/Game/UndeadPack/Ghoul/Animations/Anim_Attack_Right.Anim_Attack_Right"),
	});
	FeralGhoul.Animations.HitAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/Ghoul/Animations/Anim_Hit.Anim_Hit"),
	});
	FeralGhoul.Animations.DeathAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/Ghoul/Animations/Anim_Death.Anim_Death"),
	});
	FeralGhoul.MaxHealth = 70.0f;
	FeralGhoul.WalkSpeed = 255.0f;
	FeralGhoul.AttackRange = 140.0f;
	FeralGhoul.AttackDamage = 10.0f;
	FeralGhoul.AttackInterval = 0.95f;
	FeralGhoul.CapsuleRadius = 35.37243f;
	FeralGhoul.CapsuleHalfHeight = 87.79968f;
	FeralGhoul.MeshRelativeLocation = FVector(0.0f, -0.000001f, -88.300438f);
	FeralGhoul.MeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	ZombieVariants.Add(FeralGhoul);

	FZombieVariantDefinition BoneStalker;
	BoneStalker.VariantId = TEXT("bone_stalker");
	BoneStalker.SpawnWeight = 2;
	BoneStalker.SkeletalMesh = MakeSoftAsset<USkeletalMesh>(TEXT("/Game/UndeadPack/SkeletonEnemy/Mesh/SK_Skeleton.SK_Skeleton"));
	BoneStalker.MaterialOverrides = MakeMaterialArray({
		TEXT("/Game/UndeadPack/SkeletonEnemy/Materials/MI_Skeleton_02.MI_Skeleton_02"),
		TEXT("/Game/UndeadPack/SkeletonEnemy/Materials/MI_Skeleton_02.MI_Skeleton_02"),
	});
	BoneStalker.Animations.LocomotionAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/SkeletonEnemy/Animations/Anim_Run.Anim_Run"),
	});
	BoneStalker.Animations.AttackAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/SkeletonEnemy/Animations/Anim_Attack.Anim_Attack"),
		TEXT("/Game/UndeadPack/SkeletonEnemy/Animations/Anim_Attack_1.Anim_Attack_1"),
		TEXT("/Game/UndeadPack/SkeletonEnemy/Animations/Anim_Attack_2.Anim_Attack_2"),
	});
	BoneStalker.Animations.HitAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/SkeletonEnemy/Animations/Anim_Hit.Anim_Hit"),
	});
	BoneStalker.Animations.DeathAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/SkeletonEnemy/Animations/Anim_Death.Anim_Death"),
	});
	BoneStalker.MaxHealth = 65.0f;
	BoneStalker.WalkSpeed = 230.0f;
	BoneStalker.AttackRange = 145.0f;
	BoneStalker.AttackDamage = 11.0f;
	BoneStalker.AttackInterval = 1.0f;
	BoneStalker.CapsuleRadius = 35.641388f;
	BoneStalker.CapsuleHalfHeight = 80.0f;
	BoneStalker.MeshRelativeLocation = FVector(0.0f, 0.000856f, -79.461548f);
	BoneStalker.MeshRelativeRotation = FRotator(0.0f, 270.0f, 0.0f);
	ZombieVariants.Add(BoneStalker);

	FZombieVariantDefinition RoaringLich;
	RoaringLich.VariantId = TEXT("roaring_lich");
	RoaringLich.SpawnWeight = 2;
	RoaringLich.SkeletalMesh = MakeSoftAsset<USkeletalMesh>(TEXT("/Game/UndeadPack/Lich/Mesh/SK_Lich_Full.SK_Lich_Full"));
	RoaringLich.MaterialOverrides = MakeMaterialArray({
		TEXT("/Game/UndeadPack/Lich/Materials/MI_Lich_1.MI_Lich_1"),
	});
	RoaringLich.Animations.LocomotionAnimations = MakeAnimArray({
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieWalk.RT_Lich_ZombieWalk"),
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieRun.RT_Lich_ZombieRun"),
	});
	RoaringLich.Animations.AttackAnimations = MakeAnimArray({
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieAttackBoth.RT_Lich_ZombieAttackBoth"),
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieAttackLeft.RT_Lich_ZombieAttackLeft"),
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieAttackRight.RT_Lich_ZombieAttackRight"),
	});
	RoaringLich.Animations.HitAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/Lich/Animations/Anim_Hit.Anim_Hit"),
	});
	RoaringLich.Animations.DeathAnimations = MakeAnimArray({
		TEXT("/Game/UndeadPack/Lich/Animations/Anim_Death.Anim_Death"),
	});
	RoaringLich.Animations.SpawnAnimations = MakeAnimArray({
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieRoar.RT_Lich_ZombieRoar"),
	});
	RoaringLich.MaxHealth = 105.0f;
	RoaringLich.WalkSpeed = 195.0f;
	RoaringLich.AttackRange = 150.0f;
	RoaringLich.AttackDamage = 15.0f;
	RoaringLich.AttackInterval = 1.25f;
	RoaringLich.CapsuleRadius = 51.484489f;
	RoaringLich.CapsuleHalfHeight = 109.046074f;
	RoaringLich.MeshRelativeLocation = FVector(0.0f, 0.0f, -110.0f);
	RoaringLich.MeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	ZombieVariants.Add(RoaringLich);

	FZombieVariantDefinition CrawlLich = RoaringLich;
	CrawlLich.VariantId = TEXT("crawl_lich");
	CrawlLich.SpawnWeight = 1;
	CrawlLich.MaterialOverrides = MakeMaterialArray({
		TEXT("/Game/UndeadPack/Lich/Materials/MI_Lich_2.MI_Lich_2"),
	});
	CrawlLich.Animations.LocomotionAnimations = MakeAnimArray({
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieCrawlSlow.RT_Lich_ZombieCrawlSlow"),
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieCrawlFast.RT_Lich_ZombieCrawlFast"),
	});
	CrawlLich.Animations.AttackAnimations = MakeAnimArray({
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieCrawlAttackLeft.RT_Lich_ZombieCrawlAttackLeft"),
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieCrawlAttackRight.RT_Lich_ZombieCrawlAttackRight"),
	});
	CrawlLich.Animations.SpawnAnimations.Reset();
	CrawlLich.MaxHealth = 92.0f;
	CrawlLich.WalkSpeed = 145.0f;
	CrawlLich.AttackRange = 115.0f;
	CrawlLich.AttackDamage = 13.0f;
	CrawlLich.AttackInterval = 1.05f;
	CrawlLich.CapsuleRadius = 48.0f;
	CrawlLich.CapsuleHalfHeight = 72.0f;
	ZombieVariants.Add(CrawlLich);

	FZombieVariantDefinition HopLeftLich = RoaringLich;
	HopLeftLich.VariantId = TEXT("hop_left_lich");
	HopLeftLich.SpawnWeight = 1;
	HopLeftLich.MaterialOverrides = MakeMaterialArray({
		TEXT("/Game/UndeadPack/Lich/Materials/MI_Lich_3.MI_Lich_3"),
	});
	HopLeftLich.Animations.LocomotionAnimations = MakeAnimArray({
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieHopLeftSlow.RT_Lich_ZombieHopLeftSlow"),
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieHopLeftFast.RT_Lich_ZombieHopLeftFast"),
	});
	HopLeftLich.Animations.SpawnAnimations.Reset();
	HopLeftLich.MaxHealth = 98.0f;
	HopLeftLich.WalkSpeed = 185.0f;
	HopLeftLich.AttackDamage = 14.0f;
	HopLeftLich.AttackInterval = 1.1f;
	ZombieVariants.Add(HopLeftLich);

	FZombieVariantDefinition HopRightLich = HopLeftLich;
	HopRightLich.VariantId = TEXT("hop_right_lich");
	HopRightLich.Animations.LocomotionAnimations = MakeAnimArray({
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieHopRightSlow.RT_Lich_ZombieHopRightSlow"),
		TEXT("/Game/ZombieShooting/Animations/LichZombieMotion/RT_Lich_ZombieHopRightFast.RT_Lich_ZombieHopRightFast"),
	});
	ZombieVariants.Add(HopRightLich);
}

