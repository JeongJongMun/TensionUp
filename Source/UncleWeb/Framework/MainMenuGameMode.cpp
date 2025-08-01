#include "MainMenuGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UncleWeb/Manager/SoundManager.h"
#include "CineCameraActor.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "MovieSceneSequencePlayer.h"
#include "GameFramework/Character.h"
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

	static ConstructorHelpers::FClassFinder<ACharacter> BPMenuCharClass(TEXT("/Game/Characters/BP_MenuCharacter.BP_MenuCharacter_C"));
	if (BPMenuCharClass.Succeeded())
	{
		MenuCharacterClass = BPMenuCharClass.Class;
	}
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Load the stage asynchronously
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;

	UGameplayStatics::LoadStreamLevel(this, "Stage_2", true, false, LatentInfo);

	ShowMainMenu();

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		PC->bAutoManageActiveCameraTarget = false;

		TArray<AActor*> FoundCameras;
		UGameplayStatics::GetAllActorsOfClass(this, ACineCameraActor::StaticClass(), FoundCameras);

		if (FoundCameras.Num() > 0)
		{
			PC->SetViewTarget(FoundCameras[0]);
			//UE_LOG(LogTemp, Warning, TEXT("Instant ViewTarget Set: %s"), *FoundCameras[0]->GetName());
		}

		ULevelSequence* SequenceAsset = LoadObject<ULevelSequence>(nullptr, TEXT("/Game/Cinematic/LS_MainMenuCam.LS_MainMenuCam"));
		if (SequenceAsset)
		{
			FMovieSceneSequencePlaybackSettings Settings;
			Settings.bAutoPlay = true;
			Settings.LoopCount.Value = -1;

			ALevelSequenceActor* OutActor = nullptr;

			ULevelSequencePlayer* Player = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), SequenceAsset, Settings, OutActor);
		}

		if (MenuCharacterClass)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ACharacter* MenuChar = GetWorld()->SpawnActor<ACharacter>(MenuCharacterClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);

			if (MenuChar && FoundCameras.Num() > 0)
			{
				MenuChar->AttachToActor(FoundCameras[0], FAttachmentTransformRules::KeepRelativeTransform);
				MenuChar->SetActorRelativeLocation(FVector(369.f, 245.f, -42.f));
				MenuChar->SetActorRelativeRotation(FRotator(0.f, 120.f, 0.f));
				//UE_LOG(LogTemp, Warning, TEXT("Menu Character Spawned and Attached."));
			}
			else
			{
				//UE_LOG(LogTemp, Error, TEXT("Failed to spawn Menu Character or camera not found."));
			}

		}

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