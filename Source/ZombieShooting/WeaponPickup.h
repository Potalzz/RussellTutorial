// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponComponent.h"
#include "WeaponPickup.generated.h"

class UNiagaraComponent;
class USceneComponent;
class USphereComponent;

UCLASS()
class ZOMBIESHOOTING_API AWeaponPickup : public AActor
{
	GENERATED_BODY()

public:
	AWeaponPickup();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USphereComponent> PickupCollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UNiagaraComponent> PickupFXComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (ClampMin = "1.0"))
	float CollisionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Weapon")
	EWeaponMode PickupWeaponMode;

	UFUNCTION()
	void HandlePickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bConsumed;

	void RefreshPickupCollision();
};

