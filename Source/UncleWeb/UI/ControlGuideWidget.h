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

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void StartSlideInAnimation();
	void StartSlideOutAnimation();
	void UpdateSlideAnimation(float DeltaTime);
	void OnSlideInComplete();
	void OnSlideOutComplete();

private:
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
	float SlideDistance = 900.0f;
	bool bIsAnimating = false;
	bool bIsSlideIn = false;
	float CurrentAnimationTime = 0.0f;
	float StartOffset = 0.0f;
	float TargetOffset = 0.0f;

	bool bIsOn = false;
};