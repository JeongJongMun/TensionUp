// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "Kismet/KismetMathLibrary.h"
#include "UncleWeb/Manager/SoundManager.h"
#include "SettingWidget.generated.h"

UCLASS()
class UNCLEWEB_API USettingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USettingWidget(const FObjectInitializer& ObjectInitializer);
	UFUNCTION()
	void ToggleWidget();
	

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void ShowWidget();
	void HideWidget();
	void StartSlideInAnimation();
	void StartSlideOutAnimation();
	void UpdateSlideAnimation(float DeltaTime);
	void OnSlideInComplete();
	void OnSlideOutComplete();
	
	UFUNCTION()
	void OnBgmVolumeChanged(float Value);
	
	UFUNCTION()
	void OnSfxVolumeChanged(float Value);
	
	UFUNCTION()
	void OnMouseSensitivityChanged(float Value);
	
	UFUNCTION()
	void OnExitButtonClicked();
	void UpdateGearRotation(float DeltaTime);

	void UpdateSliderText(const TObjectPtr<class UTextBlock>& TextBlock, float Value);
	void InitBgmSlider();
	void InitSfxSlider();
	void InitMouseSensitivitySlider();

private:
	UPROPERTY()
	TArray<TObjectPtr<class UImage>> LeftGears;
    
	UPROPERTY()
	TArray<TObjectPtr<class UImage>> RightGears;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> LeftGear_0;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> RightGear_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> RightGear_1;
	
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

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> TopPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> BottomPanel;

	UPROPERTY()
	TObjectPtr<class UCanvasPanelSlot> TopPanelSlot;
    
	UPROPERTY()
	TObjectPtr<class UCanvasPanelSlot> BottomPanelSlot;

	TEnumAsByte<EEasingFunc::Type> EasingFunction = EEasingFunc::EaseOut;

	float AnimationDuration = 0.5f;
	float SlideDistance = 1080.0f;
	bool bIsAnimating = false;
	bool bIsSlideIn = false;
	float CurrentAnimationTime = 0.0f;
	float StartOffset = 0.0f;
	float TargetOffset = 0.0f;

	bool bIsOn = false;
};
