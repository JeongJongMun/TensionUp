// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#define CURRENT_CONTEXT *FString(__FUNCTION__)

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
