#include "UIManager.h"
#include "SettingWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UncleWeb/Util/TUDefines.h"

AUIManager::AUIManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    static ConstructorHelpers::FClassFinder<UUserWidget> SteamFinder(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/WBP_Steam.WBP_Steam_C'"));
    if (SteamFinder.Succeeded())
    {
        SteamWidgetClass = SteamFinder.Class;
    }
    static ConstructorHelpers::FClassFinder<UUserWidget> CrosshairFinder(TEXT("WidgetBlueprint'/Game/UI/WBP_Crosshair.WBP_Crosshair_C'"));
    if (CrosshairFinder.Succeeded())
    {
        CrosshairWidgetClass = CrosshairFinder.Class;
    }
    
    static ConstructorHelpers::FClassFinder<UUserWidget> SettingWidgetFinder(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/WBP_Setting.WBP_Setting_C'"));
    if (SettingWidgetFinder.Succeeded())
    {
        SettingWidgetClass = SettingWidgetFinder.Class;
    }
}

void AUIManager::BeginPlay()
{
    Super::BeginPlay();
}

void AUIManager::InitializeUI(APlayerController* PC)
{
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] PlayerController is null"), CURRENT_CONTEXT);
        return;
    }

    CachedPC = PC;
    CreateWidgets();
    AddWidgetsToViewport();
}

void AUIManager::CreateWidgets()
{
    if (!CachedPC) return;
    
    if (SteamWidgetClass && !SteamWidget)
    {
        SteamWidget = CreateWidget<UUserWidget>(CachedPC, SteamWidgetClass);
    }

    if (CrosshairWidgetClass && !CrosshairWidget)
    {
        CrosshairWidget = CreateWidget<UUserWidget>(CachedPC, CrosshairWidgetClass);
    }

    if (!SettingWidget && SettingWidgetClass)
    {
        SettingWidget = CreateWidget<USettingWidget>(CachedPC, SettingWidgetClass);
    }
}

void AUIManager::AddWidgetsToViewport()
{
    if (SteamWidget)
    {
        SteamWidget->AddToViewport();
    }

    if (CrosshairWidget)
    {
        CrosshairWidget->AddToViewport();
    }
    if (SettingWidget && !SettingWidget->IsInViewport())
    {
        SettingWidget->AddToViewport();
        SettingWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void AUIManager::UpdateSteamUI(const float Current, const float Max)
{
    if (SteamWidget)
    {
        if (UFunction* Func = SteamWidget->FindFunction(TEXT("UpdateStaminaBar")))
        {
            struct FParams { float Current; float Max; };
            FParams Params{ Current, Max };
            SteamWidget->ProcessEvent(Func, &Params);
        }
    }
}

void AUIManager::SetActiveCrosshair(const ESlateVisibility Visibility)
{
    if (!CrosshairWidget) return;
    
    CrosshairWidget->SetVisibility(Visibility);
}

void AUIManager::SetCrosshairColor(const ECrosshairStateType State)
{
    if (!CrosshairWidget || CurrentCrosshairState == State) return;

    UImage* CrosshairImage = Cast<UImage>(CrosshairWidget->GetWidgetFromName(TEXT("IMG_Crosshair")));
    if (!CrosshairImage) return;

    if (State == ECrosshairStateType::Default)
    {
        CrosshairImage->SetColorAndOpacity(DefaultCrosshairColor);
    }
    else if (State == ECrosshairStateType::Active)
    {
        CrosshairImage->SetColorAndOpacity(ActiveCrosshairColor);
    }
    CurrentCrosshairState = State;
}

void AUIManager::ShowSettingWidget()
{
    if (!SettingWidget)
        return;
    
    SettingWidget->ShowWidget();
}