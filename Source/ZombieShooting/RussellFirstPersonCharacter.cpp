// Fill out your copyright notice in the Description page of Project Settings.

#include "RussellFirstPersonCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "RussellHealthComponent.h"
#include "RussellShotgunComponent.h"
#include "UObject/ConstructorHelpers.h"

ARussellFirstPersonCharacter::ARussellFirstPersonCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;
	BaseEyeHeight = 64.0f;
	bIsDead = false;

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

	HealthComponent = CreateDefaultSubobject<URussellHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->MaxHealth = 100.0f;

	ShotgunComponent = CreateDefaultSubobject<URussellShotgunComponent>(TEXT("ShotgunComponent"));

	ShotgunMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShotgunMesh"));
	ShotgunMeshComponent->SetupAttachment(FirstPersonCameraComponent);
	ShotgunMeshComponent->SetRelativeLocation(FVector(50.0f, 18.0f, -24.0f));
	ShotgunMeshComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ShotgunMeshComponent->SetRelativeScale3D(FVector(0.35f));
	ShotgunMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShotgunMeshComponent->CastShadow = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShotgunMeshAsset(TEXT("/Game/Improved_Shotgun/SM_ImprovedShotgun.SM_ImprovedShotgun"));
	if (ShotgunMeshAsset.Succeeded())
	{
		ShotgunMeshComponent->SetStaticMesh(ShotgunMeshAsset.Object);
	}

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ARussellFirstPersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnHealthDepleted.AddDynamic(this, &ARussellFirstPersonCharacter::HandleHealthDepleted);
	}
}

void ARussellFirstPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ARussellFirstPersonCharacter::FireShotgun);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ARussellFirstPersonCharacter::ReloadShotgun);
	PlayerInputComponent->BindAction("Restart", IE_Pressed, this, &ARussellFirstPersonCharacter::RestartLevel);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARussellFirstPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARussellFirstPersonCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ARussellFirstPersonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ARussellFirstPersonCharacter::LookUpAtRate);
}

float ARussellFirstPersonCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	const float AppliedDamage = HealthComponent ? HealthComponent->ApplyDamage(DamageAmount) : 0.0f;
	return AppliedDamage > 0.0f ? AppliedDamage : ActualDamage;
}

float ARussellFirstPersonCharacter::GetCurrentHealth() const
{
	return HealthComponent ? HealthComponent->CurrentHealth : 0.0f;
}

float ARussellFirstPersonCharacter::GetMaxHealth() const
{
	return HealthComponent ? HealthComponent->MaxHealth : 0.0f;
}

int32 ARussellFirstPersonCharacter::GetCurrentAmmo() const
{
	return ShotgunComponent ? ShotgunComponent->GetCurrentAmmo() : 0;
}

int32 ARussellFirstPersonCharacter::GetMaxAmmo() const
{
	return ShotgunComponent ? ShotgunComponent->GetMaxAmmo() : 0;
}

void ARussellFirstPersonCharacter::MoveForward(float Value)
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

void ARussellFirstPersonCharacter::MoveRight(float Value)
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

void ARussellFirstPersonCharacter::TurnAtRate(float Rate)
{
	if (bIsDead)
	{
		return;
	}

	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ARussellFirstPersonCharacter::LookUpAtRate(float Rate)
{
	if (bIsDead)
	{
		return;
	}

	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ARussellFirstPersonCharacter::FireShotgun()
{
	if (bIsDead || !ShotgunComponent || !FirstPersonCameraComponent)
	{
		return;
	}

	ShotgunComponent->Fire(GetController(), FirstPersonCameraComponent->GetComponentLocation(), FirstPersonCameraComponent->GetComponentRotation());
}

void ARussellFirstPersonCharacter::ReloadShotgun()
{
	if (ShotgunComponent)
	{
		ShotgunComponent->Reload();
	}
}

void ARussellFirstPersonCharacter::RestartLevel()
{
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (!CurrentLevelName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
	}
}

void ARussellFirstPersonCharacter::HandleHealthDepleted()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
