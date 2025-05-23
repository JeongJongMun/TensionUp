#include "UIManager.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "UncleWeb/Util/TUDefines.h"

AUIManager::AUIManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    static ConstructorHelpers::FClassFinder<UUserWidget> StaminaFinder(TEXT("WidgetBlueprint'/Game/UI/WBP_Stamina.WBP_Stamina_C'"));
    if (StaminaFinder.Succeeded())
    {
        StaminaWidgetClass = StaminaFinder.Class;
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
    
    if (StaminaWidgetClass && !StaminaWidget)
    {
        StaminaWidget = CreateWidget<UUserWidget>(CachedPC, StaminaWidgetClass);
    }

    if (CrosshairWidgetClass && !CrosshairWidget)
    {
        CrosshairWidget = CreateWidget<UUserWidget>(CachedPC, CrosshairWidgetClass);
    }
}

void AUIManager::AddWidgetsToViewport()
{
    if (StaminaWidget)
    {
        StaminaWidget->AddToViewport();
    }

    if (CrosshairWidget)
    {
        CrosshairWidget->AddToViewport();
    }
}

void AUIManager::UpdateStaminaUI(float Current, float Max)
{
    if (StaminaWidget)
    {
        if (UFunction* Func = StaminaWidget->FindFunction(TEXT("UpdateStaminaBar")))
        {
            struct FParams { float Current; float Max; };
            FParams Params{ Current, Max };
            StaminaWidget->ProcessEvent(Func, &Params);
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
