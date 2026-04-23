// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class ZombieShootingTarget : TargetRules
{
    public ZombieShootingTarget(TargetInfo Target) : base(Target)
    {
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;

		ExtraModuleNames.AddRange(new string[] { "ZombieShooting" });
	}
}
