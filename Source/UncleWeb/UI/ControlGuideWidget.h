// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "ControlGuideWidget.generated.h"

UCLASS()
class UNCLEWEB_API UControlGuideWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    UControlGuideWidget(const FObjectInitializer& ObjectInitializer);
	void ToggleWidget();
	bool IsOn() const { return bIsOn; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void StartSlideInAnimation();
	void StartSlideOutAnimation();
	void UpdateSlideAnimation(float DeltaTime);
	void OnSlideInComplete();
	void OnSlideOutComplete();
	void UpdateGearRotation(float DeltaTime);

private:
	UPROPERTY()
	TArray<TObjectPtr<class UImage>> LeftGears;
    
	UPROPERTY()
	TArray<TObjectPtr<class UImage>> RightGears;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> LeftGear_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> LeftGear_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> LeftGear_2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> LeftGear_3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> RightGear_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> RightGear_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> RightGear_2;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> RightGear_3;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> LeftPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> RightPanel;

	UPROPERTY()
	TObjectPtr<class UCanvasPanelSlot> LeftPanelSlot;
    
	UPROPERTY()
	TObjectPtr<class UCanvasPanelSlot> RightPanelSlot;

	TEnumAsByte<EEasingFunc::Type> EasingFunction = EEasingFunc::EaseOut;
	
	float AnimationDuration = 0.5f;
	float SlideDistance = 1300.0f;
	bool bIsAnimating = false;
	bool bIsSlideIn = false;
	float CurrentAnimationTime = 0.0f;
	float StartOffset = 0.0f;
	float TargetOffset = 0.0f;

	bool bIsOn = false;

	UPROPERTY()
	TObjectPtr<class USoundManager> SoundManager;
};