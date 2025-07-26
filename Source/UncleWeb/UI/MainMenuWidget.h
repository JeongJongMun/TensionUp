// MainMenuWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "MainMenuWidget.generated.h"

UCLASS()
class UNCLEWEB_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;
	
	UFUNCTION()
	void OnStartGameClicked();

	UFUNCTION()
	void OnQuitClicked();

public:
	FString GameLevelName = TEXT("Stage_2");
};