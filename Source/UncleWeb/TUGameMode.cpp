// Copyright Epic Games, Inc. All Rights Reserved.

#include "TUGameMode.h"
#include "TUCharacterPlayer.h"
#include "UObject/ConstructorHelpers.h"

ATUGameMode::ATUGameMode()
{
	// Use Blueprint
	// static ConstructorHelpers::FClassFinder<APawn> CharacterPlayer(TEXT("/Game/Characters/BP_TUCharacterPlayer.BP_TUCharacterPlayer_C"));
	// if (CharacterPlayer.Succeeded())
	// {
	// 	DefaultPawnClass = CharacterPlayer.Class;
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[%s()] Failed Load DefaultPawnClass"), *FString(__FUNCTION__));
	// }

	// Use C++
	DefaultPawnClass = ATUCharacterPlayer::StaticClass();
}
