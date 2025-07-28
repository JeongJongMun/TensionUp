// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#define CURRENT_CONTEXT *FString(__FUNCTION__)
#define LOG_CALL() UE_LOG(LogTemp, Log, TEXT("[%s] Called"), CURRENT_CONTEXT)
#define LOG_CALL_NUM(Number) UE_LOG(LogTemp, Log, TEXT("[%s] Called %d"), CURRENT_CONTEXT, Number)
#define LOG_CALL_WARNING() UE_LOG(LogTemp, Warning, TEXT("[%s] Called"), CURRENT_CONTEXT)
#define LOG_CALL_WARNING_NUM(Number) UE_LOG(LogTemp, Warning, TEXT("[%s] Called %d"), CURRENT_CONTEXT, Number)
#define LOG_CALL_ERROR() UE_LOG(LogTemp, Error, TEXT("[%s] Called"), CURRENT_CONTEXT)
#define LOG_CALL_ERROR_NUM(Number) UE_LOG(LogTemp, Error, TEXT("[%s] Called %d"), CURRENT_CONTEXT, Number)

UENUM()
enum class ESteamChargerState : uint8
{
	None,
	Disabled,
	Enabled,
};

UENUM()
enum class EInputActionType : uint8
{
	Move,
	Jump,
	Look,
	LeftClick,
	Dash,
	Run,
	ShortenCable,
	ExtendCable,
	SteamBoost,
	ShowSetting,
	ToggleControlGuide,
	Dance,
};

UENUM()
enum class EPlayerAnimationMontageType : uint8
{
	None,
	Parkour,
	Swinging,
	SwingEnd,
	DashAir,
	DashGround,
	Dance,
};

UENUM()
enum class ECableState : uint8
{
	Idle,
	Launching,
	Attaching,
	Retracting,
};

enum class EDirectionType : uint8
{
	None,
	Left,
	Right,
	Up,
	Down,
	Forward,
	Backward
};

enum class ECrosshairStateType : uint8
{
	None,
	Default,
	Active,
};

class UNCLEWEB_API TUDefines
{
public:
	TUDefines();
	~TUDefines();
};
