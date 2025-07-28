// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpeedLineWidget.generated.h"

UCLASS()
class UNCLEWEB_API USpeedLineWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateSpeedLines(float CurrentSpeed, float MaxSpeed);

private:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> SpeedLineImage1;
    
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> SpeedLineImage2;
    
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> SpeedLineImage3;
	
	UPROPERTY()
	TArray<TObjectPtr<class UImage>> SpeedLineImages;

	float SpeedRatioThreshold = 0.5f;
	float AnimationSpeed = 1.0f;
	int32 CurrentImageIndex = 0;
	FTimerHandle AnimationTimer;
	int32 TickCounter = 0;
	int32 UpdateInterval = 5;
};