// Fill out your copyright notice in the Description page of Project Settings.

#include "SpeedLineWidget.h"
#include "Components/Image.h"

void USpeedLineWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SpeedLineImages = {SpeedLineImage1, SpeedLineImage2, SpeedLineImage3};
    
	for (int32 i = 0; i < SpeedLineImages.Num(); i++)
	{
		SpeedLineImages[i]->SetVisibility(ESlateVisibility::Hidden);
	}
}

void USpeedLineWidget::UpdateSpeedLines(const float CurrentSpeed, const float MaxSpeed)
{
	TickCounter++;
	if (TickCounter < UpdateInterval)
		return;

	TickCounter = 0;
	
	const float SpeedRatio = FMath::Clamp(CurrentSpeed / MaxSpeed, 0.0f, 1.0f);
	if (SpeedRatio > SpeedRatioThreshold)
	{
		SetVisibility(ESlateVisibility::Visible);
		const float AdjustedRatio = (SpeedRatio - SpeedRatioThreshold) / (1.0f - SpeedRatioThreshold);
		const float AnimationInterval = FMath::Lerp(15.0f, 5.0f, AdjustedRatio);
		const float Alpha = FMath::Lerp(0.3f, 1.0f, AdjustedRatio);
		SetRenderOpacity(Alpha);
		
		UpdateInterval = FMath::RoundToInt(AnimationInterval);
		SpeedLineImages[CurrentImageIndex]->SetVisibility(ESlateVisibility::Hidden);
		CurrentImageIndex = (CurrentImageIndex + 1) % SpeedLineImages.Num();
		SpeedLineImages[CurrentImageIndex]->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}