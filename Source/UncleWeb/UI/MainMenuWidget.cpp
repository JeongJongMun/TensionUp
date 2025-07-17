#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartGameClicked);
	}

	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}
}

void UMainMenuWidget::OnStartGameClicked()
{
	UGameplayStatics::OpenLevel(this, FName(*GameLevelName));
}

void UMainMenuWidget::OnSettingsClicked()
{
	// TODO : implement Settings UI
}

void UMainMenuWidget::OnQuitClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}