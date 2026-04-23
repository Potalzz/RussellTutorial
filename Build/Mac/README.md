# ZombieShooting MacBook Air Run Guide

This project was prepared for Unreal Engine `5.6.1` on macOS.

## Recommended setup

1. Install Unreal Engine `5.6.1`.
2. Install the Xcode version recommended by Epic for UE `5.6`, then open Xcode once to finish first-run setup.
3. Clone or copy this project to a writable local folder on the Mac.

## First open on macOS

1. Open `ZombieShooting.uproject`.
2. If Unreal asks to rebuild project modules, choose `Yes`.
3. Let shader compilation finish on the first launch.
4. Open `/Game/ThirdPersonBP/Maps/Forest_v01`.
5. Press `Play`.

## What was added for MacBook Air

- The project now explicitly enables the runtime plugins it depends on: `EnhancedInput` and `Niagara`.
- `PythonScriptPlugin` is limited to the `Editor` target so it does not leak into packaged game builds.
- `Config/DefaultDeviceProfiles.ini` adds a lower-cost `Mac` and `MacEditor` profile.
- `ZombieShootingGameMode` applies a lighter runtime performance profile on `PLATFORM_MAC`.
- `DefaultEngine.ini` now explicitly targets `SF_METAL_SM5` and modern Xcode generation for macOS.

## Packaging on macOS

1. In Unreal Editor, choose `Platforms > Mac`.
2. Use `Package Project` for a normal local build.
3. If you need Xcode project files, right-click `ZombieShooting.uproject` in Finder and choose `Generate Xcode Files`.
