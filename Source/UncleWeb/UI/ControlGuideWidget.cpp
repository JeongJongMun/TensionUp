// Fill out your copyright notice in the Description page of Project Settings.
#include "ControlGuideWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/KismetMathLibrary.h"

UControlGuideWidget::UControlGuideWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UControlGuideWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LeftPanel)
	{
		LeftPanelSlot = Cast<UCanvasPanelSlot>(LeftPanel->Slot);
	}
	if (RightPanel)
	{
		RightPanelSlot = Cast<UCanvasPanelSlot>(RightPanel->Slot);
	}

	if (LeftPanelSlot && RightPanelSlot)
	{
		LeftPanelSlot->SetOffsets(FMargin(-SlideDistance, 0.0f, SlideDistance, 0.0f));
		RightPanelSlot->SetOffsets(FMargin(SlideDistance, 0.0f, -SlideDistance, 0.0f));
		TargetOffset = 0.0f;
	}
}

void UControlGuideWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsAnimating)
	{
		UpdateSlideAnimation(InDeltaTime);
	}
}

void UControlGuideWidget::ToggleWidget()
{
	if (bIsAnimating)
		return;

	if (!bIsOn)
	{
		SetVisibility(ESlateVisibility::Visible);
		StartSlideInAnimation();
		bIsOn = true;
	}
	else
	{
		StartSlideOutAnimation();
		bIsOn = false;
	}
}

void UControlGuideWidget::StartSlideInAnimation()
{
	if (!LeftPanelSlot || !RightPanelSlot)
		return;
    
	bIsAnimating = true;
	bIsSlideIn = true;
	CurrentAnimationTime = 0.0f;
    
	StartOffset = SlideDistance;
	TargetOffset = 0.0f;

	LeftPanelSlot->SetOffsets(FMargin(-StartOffset, 0.0f, StartOffset, 0.0f));
	RightPanelSlot->SetOffsets(FMargin(StartOffset, 0.0f, -StartOffset, 0.0f));
}

void UControlGuideWidget::StartSlideOutAnimation()
{
	if (!LeftPanelSlot || !RightPanelSlot)
		return;
    
	bIsAnimating = true;
	bIsSlideIn = false;
	CurrentAnimationTime = 0.0f;
    
	StartOffset = LeftPanelSlot->GetPosition().X;
	TargetOffset = SlideDistance;
}

void UControlGuideWidget::UpdateSlideAnimation(const float DeltaTime)
{
	CurrentAnimationTime += DeltaTime;
	
	const float Progress = FMath::Clamp(CurrentAnimationTime / AnimationDuration, 0.0f, 1.0f);
	const float EasedProgress = UKismetMathLibrary::Ease(0.0f, 1.0f, Progress, EasingFunction);
    
	const float CurrentOffset = FMath::Lerp(StartOffset, TargetOffset, EasedProgress);
    
	if (LeftPanelSlot)
	{
		LeftPanelSlot->SetOffsets(FMargin(-CurrentOffset, 0.0f, CurrentOffset, 0.0f));
	}
    
	if (RightPanelSlot)
	{
		RightPanelSlot->SetOffsets(FMargin(CurrentOffset, 0.0f, -CurrentOffset, 0.0f));
	}
    
	if (Progress >= 1.0f)
	{
		bIsAnimating = false;

		bIsSlideIn ? OnSlideInComplete() : OnSlideOutComplete();
	}
}

void UControlGuideWidget::OnSlideInComplete()
{
	if (LeftPanelSlot && RightPanelSlot)
	{
		LeftPanelSlot->SetOffsets(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
		RightPanelSlot->SetOffsets(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
	}
}

void UControlGuideWidget::OnSlideOutComplete()
{
	SetVisibility(ESlateVisibility::Hidden);
    
	if (LeftPanelSlot && RightPanelSlot)
	{
		LeftPanelSlot->SetOffsets(FMargin(-SlideDistance, 0.0f, SlideDistance, 0.0f));
		RightPanelSlot->SetOffsets(FMargin(SlideDistance, 0.0f, -SlideDistance, 0.0f));
	}
}