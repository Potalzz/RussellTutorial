// Fill out your copyright notice in the Description page of Project Settings.

#include "ZombiePlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "CombatHealthComponent.h"
#include "WeaponComponent.h"
#include "UObject/ConstructorHelpers.h"

AZombiePlayerCharacter::AZombiePlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;
	BaseEyeHeight = 64.0f;
	bIsDead = false;
	HandTint = FLinearColor(0.78f, 0.54f, 0.38f, 1.0f);
	StartingWeaponMode = EWeaponMode::Shotgun;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->JumpZVelocity = 700.0f;
	GetCharacterMovement()->AirControl = 0.35f;

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.0f, 0.0f, BaseEyeHeight));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	HealthComponent = CreateDefaultSubobject<UCombatHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->MaxHealth = 100.0f;

	ShotgunComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("ShotgunComponent"));

	ShotgunMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShotgunMesh"));
	ShotgunMeshComponent->SetupAttachment(FirstPersonCameraComponent);
	ShotgunMeshComponent->SetRelativeLocation(FVector(58.0f, 22.0f, -24.0f));
	ShotgunMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ShotgunMeshComponent->SetRelativeScale3D(FVector(0.12f));
	ShotgunMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShotgunMeshComponent->CastShadow = false;

	ShotgunMuzzleComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ShotgunMuzzle"));
	ShotgunMuzzleComponent->SetupAttachment(ShotgunMeshComponent);
	ShotgunMuzzleComponent->SetRelativeLocation(FVector(508.0f, 0.0f, -6.0f));

	RightForearmMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightForearm"));
	RightForearmMeshComponent->SetupAttachment(FirstPersonCameraComponent);
	RightForearmMeshComponent->SetRelativeLocation(FVector(36.0f, 31.0f, -44.0f));
	RightForearmMeshComponent->SetRelativeRotation(FRotator(90.0f, -8.0f, 0.0f));
	RightForearmMeshComponent->SetRelativeScale3D(FVector(0.055f, 0.055f, 0.42f));
	RightForearmMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightForearmMeshComponent->CastShadow = false;

	RightHandMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightHand"));
	RightHandMeshComponent->SetupAttachment(FirstPersonCameraComponent);
	RightHandMeshComponent->SetRelativeLocation(FVector(58.0f, 24.0f, -31.0f));
	RightHandMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, -12.0f));
	RightHandMeshComponent->SetRelativeScale3D(FVector(0.11f, 0.08f, 0.07f));
	RightHandMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightHandMeshComponent->CastShadow = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShotgunMeshAsset(TEXT("/Game/Improved_Shotgun-d4b6fea0/fbx/improved-shotgun_extracted/source/1.1"));
	if (ShotgunMeshAsset.Succeeded())
	{
		ShotgunStaticMesh = ShotgunMeshAsset.Object;
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> FallbackShotgunMeshAsset(TEXT("/Game/Improved_Shotgun/SM_ImprovedShotgun.SM_ImprovedShotgun"));
		if (FallbackShotgunMeshAsset.Succeeded())
		{
			ShotgunStaticMesh = FallbackShotgunMeshAsset.Object;
		}
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> RocketLauncherMeshAsset(TEXT("/Game/RPG7-ada63d7d/fbx/rpg7_extracted/source/RPG7_extracted/RPG7/RPG71.RPG71"));
	if (RocketLauncherMeshAsset.Succeeded())
	{
		RPG7StaticMesh = RocketLauncherMeshAsset.Object;
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> FallbackRocketLauncherMeshAsset(TEXT("/Game/sA_Megapack_v1/sA_ShootingVfxPack/Meshes/SM_RocketLauncher.SM_RocketLauncher"));
		if (FallbackRocketLauncherMeshAsset.Succeeded())
		{
			RPG7StaticMesh = FallbackRocketLauncherMeshAsset.Object;
		}
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMeshAsset.Succeeded())
	{
		RightForearmMeshComponent->SetStaticMesh(CylinderMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		RightHandMeshComponent->SetStaticMesh(SphereMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HandMaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (HandMaterialAsset.Succeeded())
	{
		HandBaseMaterial = HandMaterialAsset.Object;
	}

	ApplyShotgunVisual();

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AZombiePlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnHealthDepleted.AddDynamic(this, &AZombiePlayerCharacter::HandleHealthDepleted);
	}

	InitializeStartingWeapon();
	ApplyFirstPersonHandMaterial();
}

void AZombiePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AZombiePlayerCharacter::FireWeapon);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AZombiePlayerCharacter::ReloadWeapon);
	PlayerInputComponent->BindAction("Restart", IE_Pressed, this, &AZombiePlayerCharacter::RestartLevel);

	PlayerInputComponent->BindAxis("MoveForward", this, &AZombiePlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AZombiePlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AZombiePlayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AZombiePlayerCharacter::LookUpAtRate);
}

float AZombiePlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	const float DamageToApply = ActualDamage > 0.0f ? ActualDamage : DamageAmount;
	const float AppliedDamage = HealthComponent ? HealthComponent->ApplyDamage(DamageToApply) : 0.0f;
	return AppliedDamage > 0.0f ? AppliedDamage : ActualDamage;
}

float AZombiePlayerCharacter::GetCurrentHealth() const
{
	return HealthComponent ? HealthComponent->CurrentHealth : 0.0f;
}

float AZombiePlayerCharacter::GetMaxHealth() const
{
	return HealthComponent ? HealthComponent->MaxHealth : 0.0f;
}

int32 AZombiePlayerCharacter::GetCurrentAmmo() const
{
	return ShotgunComponent ? ShotgunComponent->GetCurrentAmmo() : 0;
}

int32 AZombiePlayerCharacter::GetMaxAmmo() const
{
	return ShotgunComponent ? ShotgunComponent->GetMaxAmmo() : 0;
}

bool AZombiePlayerCharacter::HasInfiniteAmmo() const
{
	return ShotgunComponent ? ShotgunComponent->HasInfiniteAmmo() : false;
}

FString AZombiePlayerCharacter::GetCurrentWeaponLabel() const
{
	return ShotgunComponent ? ShotgunComponent->GetWeaponModeLabel() : TEXT("Unarmed");
}

void AZombiePlayerCharacter::EquipWeapon(EWeaponMode NewWeaponMode)
{
	if (!ShotgunComponent)
	{
		return;
	}

	ShotgunComponent->SetWeaponMode(NewWeaponMode);
	ApplyCurrentWeaponVisual();
}

void AZombiePlayerCharacter::EquipRPG7()
{
	EquipWeapon(EWeaponMode::RPG7);
}

bool AZombiePlayerCharacter::IsUsingRPG7() const
{
	return ShotgunComponent ? ShotgunComponent->IsUsingRPG7() : false;
}

void AZombiePlayerCharacter::MoveForward(float Value)
{
	if (bIsDead)
	{
		return;
	}

	if (Controller && !FMath::IsNearlyZero(Value))
	{
		const FRotator ControlRotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
	}
}

void AZombiePlayerCharacter::MoveRight(float Value)
{
	if (bIsDead)
	{
		return;
	}

	if (Controller && !FMath::IsNearlyZero(Value))
	{
		const FRotator ControlRotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
	}
}

void AZombiePlayerCharacter::TurnAtRate(float Rate)
{
	if (bIsDead)
	{
		return;
	}

	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AZombiePlayerCharacter::LookUpAtRate(float Rate)
{
	if (bIsDead)
	{
		return;
	}

	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AZombiePlayerCharacter::FireWeapon()
{
	if (bIsDead || !ShotgunComponent || !FirstPersonCameraComponent)
	{
		return;
	}

	const FVector MuzzleLocation = ShotgunMuzzleComponent ? ShotgunMuzzleComponent->GetComponentLocation() : FirstPersonCameraComponent->GetComponentLocation();
	ShotgunComponent->FireWithVisualStart(GetController(), FirstPersonCameraComponent->GetComponentLocation(), FirstPersonCameraComponent->GetComponentRotation(), MuzzleLocation);
}

void AZombiePlayerCharacter::ReloadWeapon()
{
	if (ShotgunComponent)
	{
		ShotgunComponent->Reload();
	}
}

void AZombiePlayerCharacter::RestartLevel()
{
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (!CurrentLevelName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
	}
}

void AZombiePlayerCharacter::InitializeStartingWeapon()
{
	EquipWeapon(StartingWeaponMode);
}

void AZombiePlayerCharacter::HandleHealthDepleted()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		DisableInput(PlayerController);
	}
}

void AZombiePlayerCharacter::ApplyFirstPersonHandMaterial()
{
	if (!HandBaseMaterial)
	{
		return;
	}

	UStaticMeshComponent* HandMeshes[] = { RightForearmMeshComponent.Get(), RightHandMeshComponent.Get() };
	for (UStaticMeshComponent* HandMesh : HandMeshes)
	{
		if (!HandMesh)
		{
			continue;
		}

		UMaterialInstanceDynamic* HandMaterial = UMaterialInstanceDynamic::Create(HandBaseMaterial, this);
		if (HandMaterial)
		{
			HandMaterial->SetVectorParameterValue(TEXT("Color"), HandTint);
			HandMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.7f);
			HandMesh->SetMaterial(0, HandMaterial);
		}
	}
}

void AZombiePlayerCharacter::ApplyCurrentWeaponVisual()
{
	if (ShotgunComponent && ShotgunComponent->GetWeaponMode() == EWeaponMode::RPG7)
	{
		ApplyRPG7Visual();
		return;
	}

	ApplyShotgunVisual();
}

void AZombiePlayerCharacter::ApplyShotgunVisual()
{
	if (ShotgunMeshComponent)
	{
		if (ShotgunStaticMesh)
		{
			ShotgunMeshComponent->SetStaticMesh(ShotgunStaticMesh);
			ApplyWeaponMaterials(ShotgunStaticMesh);
		}

		ShotgunMeshComponent->SetRelativeLocation(FVector(58.0f, 22.0f, -24.0f));
		ShotgunMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
		ShotgunMeshComponent->SetRelativeScale3D(FVector(0.12f));
	}

	if (ShotgunMuzzleComponent)
	{
		ShotgunMuzzleComponent->SetRelativeLocation(FVector(508.0f, 0.0f, -6.0f));
	}

	if (RightForearmMeshComponent)
	{
		RightForearmMeshComponent->SetRelativeLocation(FVector(36.0f, 31.0f, -44.0f));
		RightForearmMeshComponent->SetRelativeRotation(FRotator(90.0f, -8.0f, 0.0f));
	}

	if (RightHandMeshComponent)
	{
		RightHandMeshComponent->SetRelativeLocation(FVector(58.0f, 24.0f, -31.0f));
		RightHandMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, -12.0f));
	}
}

void AZombiePlayerCharacter::ApplyRPG7Visual()
{
	if (ShotgunMeshComponent)
	{
		if (RPG7StaticMesh)
		{
			ShotgunMeshComponent->SetStaticMesh(RPG7StaticMesh);
			ApplyWeaponMaterials(RPG7StaticMesh);
		}
	}

	const bool bUseImportedRPG7Mesh = RPG7StaticMesh && RPG7StaticMesh->GetPathName().Contains(TEXT("/Game/RPG7-ada63d7d/"));
	if (ShotgunMeshComponent)
	{
		ShotgunMeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

		if (bUseImportedRPG7Mesh)
		{
			ShotgunMeshComponent->SetRelativeLocation(FVector(46.0f, 23.0f, -26.0f));
			ShotgunMeshComponent->SetRelativeScale3D(FVector(2.2f));
		}
		else
		{
			ShotgunMeshComponent->SetRelativeLocation(FVector(50.0f, 24.0f, -24.0f));
			ShotgunMeshComponent->SetRelativeScale3D(FVector(0.85f));
		}
	}

	if (ShotgunMuzzleComponent)
	{
		ShotgunMuzzleComponent->SetRelativeLocation(bUseImportedRPG7Mesh ? FVector(0.0f, 23.5f, 3.0f) : FVector(0.0f, 62.0f, 0.0f));
	}

	if (RightForearmMeshComponent)
	{
		RightForearmMeshComponent->SetRelativeLocation(FVector(35.0f, 31.0f, -43.0f));
		RightForearmMeshComponent->SetRelativeRotation(FRotator(90.0f, -8.0f, 2.0f));
	}

	if (RightHandMeshComponent)
	{
		RightHandMeshComponent->SetRelativeLocation(FVector(53.0f, 25.0f, -31.0f));
		RightHandMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, -10.0f));
	}
}

void AZombiePlayerCharacter::ApplyWeaponMaterials(UStaticMesh* WeaponMesh)
{
	if (!ShotgunMeshComponent || !WeaponMesh)
	{
		return;
	}

	const TArray<FStaticMaterial>& StaticMaterials = WeaponMesh->GetStaticMaterials();
	const int32 MaterialSlotCount = FMath::Max(ShotgunMeshComponent->GetNumMaterials(), StaticMaterials.Num());
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotCount; ++MaterialIndex)
	{
		UMaterialInterface* MaterialToApply = StaticMaterials.IsValidIndex(MaterialIndex) ? StaticMaterials[MaterialIndex].MaterialInterface : nullptr;
		ShotgunMeshComponent->SetMaterial(MaterialIndex, MaterialToApply);
	}
}

