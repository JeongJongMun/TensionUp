#include "UIManager.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
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
}

void AUIManager::UpdateSteamUI(float Current, float Max)
{
    if (SteamWidget)
    {
        if (UFunction* Func = SteamWidget->FindFunction(TEXT("UpdateStaminaBar")))
        {
            struct FParams { float Current; float Max; };
            FParams Params{ Current, Max };
            SteamWidget->ProcessEvent(Func, &Params);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] UpdateStaminaBar function not found in SteamWidget"), CURRENT_CONTEXT);
        }
    }
}

void AUIManager::SetActiveCrosshair(const ESlateVisibility Visibility)
{
    if (!CrosshairWidget) return;
    
    CrosshairWidget->SetVisibility(Visibility);
}

void AUIManager::SetCrosshairColor(ECrosshairStateType State)
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
