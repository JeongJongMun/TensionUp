#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "UncleWeb/Manager/SoundManager.h"
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

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}
}

void UMainMenuWidget::OnStartGameClicked()
{
	if (USoundManager* SM = GetGameInstance()->GetSubsystem<USoundManager>())
	{
		SM->PlaySound2D("SW_Sys_Button");
	}
	// sound issue, so we use a timer to delay the level loading
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			UGameplayStatics::OpenLevel(this, FName(*GameLevelName));
		}, 0.2f, false);
}

void UMainMenuWidget::OnQuitClicked()
{
	if (USoundManager* SM = GetGameInstance()->GetSubsystem<USoundManager>())
	{
		SM->PlaySound2D("SW_Sys_Button");
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}