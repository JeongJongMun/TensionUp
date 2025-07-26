// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SettingSaveGame.generated.h"

UCLASS()
class UNCLEWEB_API USettingSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	float BgmVolume = 1.0f;
	float SfxVolume = 1.0f;
	float MouseSensitivity = 1.0f;
};
