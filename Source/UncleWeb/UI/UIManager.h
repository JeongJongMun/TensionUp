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
	void UpdateSteamUI(float Current, float Max) const;
	void SetActiveCrosshair(ESlateVisibility Visibility) const;
	void SetCrosshairColor(ECrosshairStateType State);
	void ShowSettingWidget() const;
	void ToggleControlGuideWidget() const;
	void UpdateSpeedLineWidget(float CurrentSpeed, float MaxSpeed) const;
	void ShowCutSceneImage(UTexture2D* Texture, float Duration = 3.0f);

protected:
	virtual void BeginPlay() override;

private:
	void CreateWidgets();
	void AddWidgetsToViewport() const;

// --------------------
// Variables
// --------------------
private:
	TSubclassOf<UUserWidget> SteamWidgetClass;
	TSubclassOf<UUserWidget> CrosshairWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> SteamWidget;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> CrosshairWidget;

	UPROPERTY()
	TObjectPtr<class USettingWidget> SettingWidget;

	UPROPERTY()
	TSubclassOf<class USettingWidget> SettingWidgetClass;
	
	UPROPERTY()
	TObjectPtr<class UControlGuideWidget> ControlGuideWidget;
	
	UPROPERTY()
	TSubclassOf<class UControlGuideWidget> ControlGuideWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> SpeedLineWidget;

	UPROPERTY()
	TObjectPtr<class UCutSceneImageWidget> CutSceneImageWidget;

	UPROPERTY()
	TSubclassOf<class UCutSceneImageWidget> CutSceneImageWidgetClass;

	UPROPERTY()
	TSubclassOf<class UUserWidget> SpeedLineWidgetClass;
	
	UPROPERTY()
	TObjectPtr<APlayerController> CachedPC;
	
	ECrosshairStateType CurrentCrosshairState = ECrosshairStateType::Default;
	FLinearColor DefaultCrosshairColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	FLinearColor ActiveCrosshairColor = FColor::FromHex(TEXT("F4672A")).ReinterpretAsLinear();
};
