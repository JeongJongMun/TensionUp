// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UIManager.generated.h"

UCLASS()
class UNCLEWEB_API AUIManager : public AActor
{
	GENERATED_BODY()
	
public:
	AUIManager();

	void InitializeUI(APlayerController* PC);
	void UpdateStaminaUI(float Current, float Max);
	void SetActiveCrosshair(ESlateVisibility Visibility);

protected:
	virtual void BeginPlay() override;

	// Widget Classes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> StaminaWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	// Widget Instances
	UPROPERTY()
	TObjectPtr<UUserWidget> StaminaWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> CrosshairWidget;

	UPROPERTY()
	TObjectPtr<APlayerController> CachedPC;

private:
	void CreateWidgets();
	void AddWidgetsToViewport();
};
