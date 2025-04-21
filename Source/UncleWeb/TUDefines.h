// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

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

class UNCLEWEB_API TUDefines
{
public:
	TUDefines();
	~TUDefines();
};
