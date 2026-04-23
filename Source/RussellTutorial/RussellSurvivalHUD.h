// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RussellSurvivalHUD.generated.h"

UCLASS()
class RUSSELLTUTORIAL_API ARussellSurvivalHUD : public AHUD
{
	GENERATED_BODY()

public:
	ARussellSurvivalHUD();

	virtual void DrawHUD() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	FLinearColor TextColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	FLinearColor WarningColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	float TextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	float LineHeight;

	void DrawStatusLine(const FString& Text, int32 LineIndex, const FLinearColor& Color);
};
