// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MainMenuGameMode.generated.h"

class ACharacter;

UCLASS()
class UNCLEWEB_API AMainMenuGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AMainMenuGameMode();
	void ShowMainMenu();
	void HideMainMenu();

protected:
	virtual void BeginPlay() override;
	
private:
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> MainMenuWidget;

	UPROPERTY()
	TObjectPtr<class USoundManager> SoundManager;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ACharacter> MenuCharacterClass;
};
