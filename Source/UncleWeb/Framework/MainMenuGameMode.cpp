#include "MainMenuGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UncleWeb/Manager/SoundManager.h"
#include "UncleWeb/Util/TUDefines.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	DefaultPawnClass = nullptr;
	MainMenuWidget = nullptr;

	const ConstructorHelpers::FClassFinder<UUserWidget> MainMenuWidgetBPClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/WBP_Main.WBP_Main_C'"));
	if (MainMenuWidgetBPClass.Class != nullptr)
	{
		MainMenuWidgetClass = MainMenuWidgetBPClass.Class;
	}
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	ShowMainMenu();

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		PC->SetInputMode(FInputModeUIOnly());
	}

	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			SoundManager = GI->GetSubsystem<USoundManager>();
		}
	}
	SoundManager->PlayBGM("SW_Main_Jazz");
}

void AMainMenuGameMode::ShowMainMenu()
{
	if (!MainMenuWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] MainMenuWidgetClass is not set!"), CURRENT_CONTEXT);
		return;
	}

	if (MainMenuWidget)
	{
		MainMenuWidget->RemoveFromParent();
		MainMenuWidget = nullptr;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		MainMenuWidget = CreateWidget<UUserWidget>(PC, MainMenuWidgetClass);
		if (MainMenuWidget)
		{
			MainMenuWidget->AddToViewport();
		}
	}
}

void AMainMenuGameMode::HideMainMenu()
{
	if (MainMenuWidget)
	{
		MainMenuWidget->RemoveFromParent();
		MainMenuWidget = nullptr;
	}
}