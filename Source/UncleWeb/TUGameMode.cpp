// Copyright Epic Games, Inc. All Rights Reserved.

#include "TUGameMode.h"
#include "TUCharacterPlayer.h"
#include "UObject/ConstructorHelpers.h"

ATUGameMode::ATUGameMode()
{
	DefaultPawnClass = ATUCharacterPlayer::StaticClass();
}
