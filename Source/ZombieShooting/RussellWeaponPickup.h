// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RussellWeaponPickup.generated.h"

class UNiagaraComponent;
class USceneComponent;
class USphereComponent;

UCLASS()
class ZOMBIESHOOTING_API ARussellWeaponPickup : public AActor
{
	GENERATED_BODY()

public:
	ARussellWeaponPickup();

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

	UFUNCTION()
	void HandlePickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bConsumed;

	void RefreshPickupCollision();
};
