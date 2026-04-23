// Fill out your copyright notice in the Description page of Project Settings.

#include "RussellSurvivalHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "RussellFirstPersonCharacter.h"
#include "RussellTutorialGameMode.h"

ARussellSurvivalHUD::ARussellSurvivalHUD()
{
	TextColor = FLinearColor::White;
	WarningColor = FLinearColor(1.0f, 0.25f, 0.2f, 1.0f);
	TextScale = 1.25f;
	LineHeight = 28.0f;
}

void ARussellSurvivalHUD::DrawHUD()
{
	Super::DrawHUD();

	APlayerController* PlayerController = GetOwningPlayerController();
	ARussellFirstPersonCharacter* PlayerCharacter = PlayerController ? Cast<ARussellFirstPersonCharacter>(PlayerController->GetPawn()) : nullptr;
	ARussellTutorialGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ARussellTutorialGameMode>() : nullptr;

	if (PlayerCharacter)
	{
		DrawStatusLine(FString::Printf(TEXT("Health: %.0f / %.0f"), PlayerCharacter->GetCurrentHealth(), PlayerCharacter->GetMaxHealth()), 0, TextColor);
		DrawStatusLine(FString::Printf(TEXT("Ammo: %d / %d"), PlayerCharacter->GetCurrentAmmo(), PlayerCharacter->GetMaxAmmo()), 1, TextColor);

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
		DrawStatusLine(FString::Printf(TEXT("Wave: %d"), GameMode->GetWaveNumber()), 2, TextColor);
		DrawStatusLine(FString::Printf(TEXT("Kills: %d"), GameMode->GetKillCount()), 3, TextColor);
		DrawStatusLine(FString::Printf(TEXT("Alive: %d"), GameMode->GetAliveZombieCount()), 4, TextColor);
	}
}

void ARussellSurvivalHUD::DrawStatusLine(const FString& Text, int32 LineIndex, const FLinearColor& Color)
{
	DrawText(Text, Color, 24.0f, 24.0f + LineHeight * LineIndex, GEngine ? GEngine->GetSmallFont() : nullptr, TextScale);
}
