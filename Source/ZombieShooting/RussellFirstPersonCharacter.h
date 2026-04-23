// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RussellFirstPersonCharacter.generated.h"

class UCameraComponent;
class URussellHealthComponent;
class URussellShotgunComponent;
class UMaterialInterface;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS()
class ZOMBIESHOOTING_API ARussellFirstPersonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARussellFirstPersonCharacter();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UFUNCTION(BlueprintPure, Category = "Player")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	int32 GetCurrentAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	int32 GetMaxAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	bool HasInfiniteAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FString GetCurrentWeaponLabel() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipRPG7();

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsUsingRPG7() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	bool IsDead() const { return bIsDead; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URussellHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URussellShotgunComponent> ShotgunComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ShotgunMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> ShotgunMuzzleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> RightForearmMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> RightHandMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Visual")
	TObjectPtr<UStaticMesh> ShotgunStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Visual")
	TObjectPtr<UStaticMesh> RPG7StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UMaterialInterface> HandBaseMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FLinearColor HandTint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	float BaseTurnRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	float BaseLookUpRate;

	UFUNCTION()
	void HandleHealthDepleted();

	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void FireShotgun();
	void ReloadShotgun();
	void RestartLevel();
	void ApplyFirstPersonHandMaterial();
	void ApplyShotgunVisual();
	void ApplyRPG7Visual();
	void ApplyWeaponMaterials(UStaticMesh* WeaponMesh);

private:
	bool bIsDead;
};
