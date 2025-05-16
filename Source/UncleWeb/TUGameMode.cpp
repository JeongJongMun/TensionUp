// Copyright Epic Games, Inc. All Rights Reserved.

#include "TUGameMode.h"
#include "TUCharacterPlayer.h"
#include "UObject/ConstructorHelpers.h"

ATUGameMode::ATUGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Characters/BP_TUCharacterPlayer.BP_TUCharacterPlayer_C"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
		UE_LOG(LogTemp, Log, TEXT("Blueprint Character Class Found: %s"), *PlayerPawnBPClass.Class->GetName());
	}
	else
	{
		DefaultPawnClass = ATUCharacterPlayer::StaticClass();
		UE_LOG(LogTemp, Warning, TEXT("Blueprint Character Class Not Found. Using C++ Character Class: %s"), *DefaultPawnClass->GetName());
	}
}
