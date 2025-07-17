// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#define CURRENT_CONTEXT *FString(__FUNCTION__)

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
	SteamBoost
};

UENUM()
enum class EPlayerAnimationMontageType : uint8
{
	None,
	Parkour,
	Swinging,
	SwingEnd,
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
