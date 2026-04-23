// Fill out your copyright notice in the Description page of Project Settings.

#include "SurvivalHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "ZombiePlayerCharacter.h"
#include "ZombieShootingGameMode.h"

ASurvivalHUD::ASurvivalHUD()
{
	TextColor = FLinearColor::White;
	WarningColor = FLinearColor(1.0f, 0.25f, 0.2f, 1.0f);
	PerformanceGoodColor = FLinearColor(0.65f, 1.0f, 0.72f, 1.0f);
	PerformanceWarningColor = FLinearColor(1.0f, 0.82f, 0.32f, 1.0f);
	PerformanceCriticalColor = WarningColor;
	TextScale = 1.25f;
	LineHeight = 28.0f;
	PerformanceSmoothingAlpha = 0.12f;
	PerformanceWarningFPS = 50.0f;
	PerformanceCriticalFPS = 30.0f;
	SmoothedFrameTimeMs = 0.0f;
	SmoothedFPS = 0.0f;
}

void ASurvivalHUD::DrawHUD()
{
	Super::DrawHUD();

	if (const UWorld* World = GetWorld())
	{
		UpdatePerformanceMetrics(World->GetDeltaSeconds());
	}

	APlayerController* PlayerController = GetOwningPlayerController();
	AZombiePlayerCharacter* PlayerCharacter = PlayerController ? Cast<AZombiePlayerCharacter>(PlayerController->GetPawn()) : nullptr;
	AZombieShootingGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AZombieShootingGameMode>() : nullptr;

	if (PlayerCharacter)
	{
		DrawStatusLine(FString::Printf(TEXT("Health: %.0f / %.0f"), PlayerCharacter->GetCurrentHealth(), PlayerCharacter->GetMaxHealth()), 0, TextColor);
		const FString AmmoText = PlayerCharacter->HasInfiniteAmmo()
			? TEXT("Ammo: Infinite")
			: FString::Printf(TEXT("Ammo: %d / %d"), PlayerCharacter->GetCurrentAmmo(), PlayerCharacter->GetMaxAmmo());
		DrawStatusLine(AmmoText, 1, TextColor);
		DrawStatusLine(FString::Printf(TEXT("Weapon: %s"), *PlayerCharacter->GetCurrentWeaponLabel()), 2, TextColor);

		if (PlayerCharacter->IsDead())
		{
			const FString GameOverText(TEXT("GAME OVER - Press Enter to restart"));
			const float CenterX = Canvas ? Canvas->ClipX * 0.5f - 220.0f : 300.0f;
			const float CenterY = Canvas ? Canvas->ClipY * 0.5f : 300.0f;
			DrawText(GameOverText, WarningColor, CenterX, CenterY, GEngine ? GEngine->GetMediumFont() : nullptr, 1.5f);
		}
	}

	if (GameMode)
	{
		DrawStatusLine(FString::Printf(TEXT("Wave: %d"), GameMode->GetWaveNumber()), 3, TextColor);
		DrawStatusLine(FString::Printf(TEXT("Kills: %d"), GameMode->GetKillCount()), 4, TextColor);
		DrawStatusLine(FString::Printf(TEXT("Alive: %d"), GameMode->GetAliveZombieCount()), 5, TextColor);
	}

	const FLinearColor PerformanceColor = SmoothedFPS <= PerformanceCriticalFPS
		? PerformanceCriticalColor
		: (SmoothedFPS <= PerformanceWarningFPS ? PerformanceWarningColor : PerformanceGoodColor);

	DrawTopRightStatusLine(FString::Printf(TEXT("FPS: %.1f"), SmoothedFPS), 0, PerformanceColor);
	DrawTopRightStatusLine(FString::Printf(TEXT("Frame: %.1f ms"), SmoothedFrameTimeMs), 1, PerformanceColor);
}

void ASurvivalHUD::DrawStatusLine(const FString& Text, int32 LineIndex, const FLinearColor& Color)
{
	DrawText(Text, Color, 24.0f, 24.0f + LineHeight * LineIndex, GEngine ? GEngine->GetSmallFont() : nullptr, TextScale);
}

void ASurvivalHUD::DrawTopRightStatusLine(const FString& Text, int32 LineIndex, const FLinearColor& Color)
{
	if (!Canvas)
	{
		return;
	}

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	float TextWidth = 0.0f;
	float TextHeight = 0.0f;
	GetTextSize(Text, TextWidth, TextHeight, Font, TextScale);

	const float Padding = 24.0f;
	const float DrawX = Canvas->ClipX - TextWidth - Padding;
	const float DrawY = Padding + LineHeight * LineIndex;
	DrawText(Text, Color, DrawX, DrawY, Font, TextScale);
}

void ASurvivalHUD::UpdatePerformanceMetrics(float DeltaSeconds)
{
	if (DeltaSeconds <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float CurrentFrameTimeMs = DeltaSeconds * 1000.0f;
	if (SmoothedFrameTimeMs <= 0.0f)
	{
		SmoothedFrameTimeMs = CurrentFrameTimeMs;
	}
	else
	{
		SmoothedFrameTimeMs = FMath::Lerp(SmoothedFrameTimeMs, CurrentFrameTimeMs, PerformanceSmoothingAlpha);
	}

	SmoothedFPS = SmoothedFrameTimeMs > KINDA_SMALL_NUMBER ? 1000.0f / SmoothedFrameTimeMs : 0.0f;
}

