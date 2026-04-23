// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SurvivalHUD.generated.h"

UCLASS()
class ZOMBIESHOOTING_API ASurvivalHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASurvivalHUD();

	virtual void DrawHUD() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	FLinearColor TextColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	FLinearColor WarningColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Performance")
	FLinearColor PerformanceGoodColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Performance")
	FLinearColor PerformanceWarningColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Performance")
	FLinearColor PerformanceCriticalColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	float TextScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	float LineHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Performance", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float PerformanceSmoothingAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Performance", meta = (ClampMin = "1.0"))
	float PerformanceWarningFPS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Performance", meta = (ClampMin = "1.0"))
	float PerformanceCriticalFPS;

	void DrawStatusLine(const FString& Text, int32 LineIndex, const FLinearColor& Color);
	void DrawTopRightStatusLine(const FString& Text, int32 LineIndex, const FLinearColor& Color);
	void UpdatePerformanceMetrics(float DeltaSeconds);

	float SmoothedFrameTimeMs;
	float SmoothedFPS;
};

