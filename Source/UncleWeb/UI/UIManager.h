// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UncleWeb/Util/TUDefines.h"
#include "UIManager.generated.h"

UCLASS()
class UNCLEWEB_API AUIManager : public AActor
{
	GENERATED_BODY()

// --------------------
// Functions
// --------------------
public:
	AUIManager();

	void InitializeUI(APlayerController* PC);
	void UpdateSteamUI(float Current, float Max);
	void SetActiveCrosshair(ESlateVisibility Visibility);
	void SetCrosshairColor(ECrosshairStateType State);

protected:
	virtual void BeginPlay() override;

private:
	void CreateWidgets();
	void AddWidgetsToViewport();

// --------------------
// Variables
// --------------------
private:
	TSubclassOf<UUserWidget> SteamWidgetClass;
	TSubclassOf<UUserWidget> CrosshairWidgetClass;
	TObjectPtr<UUserWidget> SteamWidget;
	TObjectPtr<UUserWidget> CrosshairWidget;
	TObjectPtr<APlayerController> CachedPC;
	
	ECrosshairStateType CurrentCrosshairState = ECrosshairStateType::Default;
	FLinearColor DefaultCrosshairColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	FLinearColor ActiveCrosshairColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
};
