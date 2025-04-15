// Copyright Epic Games, Inc. All Rights Reserved.

#include "UncleWebGameMode.h"
#include "UWCharacterPlayer.h"
#include "UObject/ConstructorHelpers.h"

AUncleWebGameMode::AUncleWebGameMode()
{
	DefaultPawnClass = AUWCharacterPlayer::StaticClass();
}
