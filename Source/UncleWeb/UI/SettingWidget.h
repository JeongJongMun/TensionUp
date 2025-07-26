// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "UncleWeb/Manager/SoundManager.h"
#include "SettingWidget.generated.h"

UCLASS()
class UNCLEWEB_API USettingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USettingWidget(const FObjectInitializer& ObjectInitializer);
	void ShowWidget();
	
	UFUNCTION()
	void HideWidget();
	

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnBgmVolumeChanged(float Value);
	
	UFUNCTION()
	void OnSfxVolumeChanged(float Value);
	
	UFUNCTION()
	void OnMouseSensitivityChanged(float Value);
	
	UFUNCTION()
	void OnExitButtonClicked();

	void UpdateSliderText(const TObjectPtr<class UTextBlock>& TextBlock, float Value);
	void InitBgmSlider();
	void InitSfxSlider();
	void InitMouseSensitivitySlider();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> ApplyButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> ExitButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> BgmSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> SfxSlider;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MouseSensitivitySlider;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BgmText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SfxText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MouseSensitivityText;

	float BgmValue = 1.0f;
	float SfxValue = 1.0f;
	float MouseSensitivityValue = 1.0f;

	FString SaveGameSlotName = TEXT("PlayerSaveSlot");
	int32 SaveGameSlotIndex = 0;

	UPROPERTY()
	TObjectPtr<USoundManager> SoundManager;

	FString MainMenuLevelName = TEXT("MainMenu");

	UPROPERTY()
	TObjectPtr<APlayerController> CachedPC;
};
