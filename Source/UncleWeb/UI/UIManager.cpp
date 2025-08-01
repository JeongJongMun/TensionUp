#include "UIManager.h"
#include "ControlGuideWidget.h"
#include "SettingWidget.h"
#include "SpeedLineWidget.h"
#include "CutSceneImageWidget.h"
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
    
    static ConstructorHelpers::FClassFinder<UUserWidget> SettingWidgetFinder(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/WBP_Setting.WBP_Setting_C'"));
    if (SettingWidgetFinder.Succeeded())
    {
        SettingWidgetClass = SettingWidgetFinder.Class;
    }

    static ConstructorHelpers::FClassFinder<UControlGuideWidget> ControlGuideFinder(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/WBP_ControlGuide.WBP_ControlGuide_C'"));
    if (ControlGuideFinder.Succeeded())
    {
        ControlGuideWidgetClass = ControlGuideFinder.Class;
    }

    static ConstructorHelpers::FClassFinder<UUserWidget> SpeedLineFinder(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/WBP_SpeedLine.WBP_SpeedLine_C'"));
    if (SpeedLineFinder.Succeeded())
    {
        SpeedLineWidgetClass = SpeedLineFinder.Class;
    }

    static ConstructorHelpers::FClassFinder<UCutSceneImageWidget> CutSceneFinder(TEXT("/Game/UI/WBP_CutSceneImage.WBP_CutSceneImage_C"));
    if (CutSceneFinder.Succeeded())
    {
        CutSceneImageWidgetClass = CutSceneFinder.Class;
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
    if (!ControlGuideWidget && ControlGuideWidgetClass)
    {
        ControlGuideWidget = CreateWidget<UControlGuideWidget>(CachedPC, ControlGuideWidgetClass);
    }
    if (!SpeedLineWidget && SpeedLineWidgetClass)
    {
        SpeedLineWidget = CreateWidget<UUserWidget>(CachedPC, SpeedLineWidgetClass);
    }

    if (!CutSceneImageWidget && CutSceneImageWidgetClass)
    {
        CutSceneImageWidget = CreateWidget<UCutSceneImageWidget>(CachedPC, CutSceneImageWidgetClass);
    }

}

void AUIManager::AddWidgetsToViewport() const
{
    if (SteamWidget)
    {
        SteamWidget->AddToViewport();
        CutSceneImageWidget->SetVisibility(ESlateVisibility::Hidden);
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
    if (ControlGuideWidget && !ControlGuideWidget->IsInViewport())
    {
        ControlGuideWidget->AddToViewport();
    }
    if (SpeedLineWidget && !SpeedLineWidget->IsInViewport())
    {
        SpeedLineWidget->AddToViewport();
    }
}

void AUIManager::UpdateSteamUI(const float Current, const float Max) const
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

void AUIManager::SetActiveCrosshair(const ESlateVisibility Visibility) const
{
    if (!CrosshairWidget) return;
    
    CrosshairWidget->SetVisibility(Visibility);
}

void AUIManager::SetCrosshairColor(const ECrosshairStateType State)
{
    if (!CrosshairWidget || CurrentCrosshairState == State) return;

    UImage* CrosshairImage = Cast<UImage>(CrosshairWidget->GetWidgetFromName(TEXT("IMG_Crosshair")));
    UImage* CrosshairFocusImage = Cast<UImage>(CrosshairWidget->GetWidgetFromName(TEXT("IMG_Crosshair_Focus")));
    
    if (!CrosshairImage || !CrosshairFocusImage) return;

    if (State == ECrosshairStateType::Default)
    {
        CrosshairImage->SetColorAndOpacity(DefaultCrosshairColor);
        CrosshairFocusImage->SetColorAndOpacity(DefaultCrosshairColor);
        CrosshairFocusImage->SetVisibility(ESlateVisibility::Hidden);
    }
    else if (State == ECrosshairStateType::Active)
    {
        CrosshairImage->SetColorAndOpacity(ActiveCrosshairColor);
        CrosshairFocusImage->SetColorAndOpacity(ActiveCrosshairColor);
        CrosshairFocusImage->SetVisibility(ESlateVisibility::Visible);
    }
    CurrentCrosshairState = State;
}

void AUIManager::ShowSettingWidget() const
{
    if (!SettingWidget || ControlGuideWidget->IsOn())
        return;
    
    SettingWidget->ToggleWidget();
}

void AUIManager::ToggleControlGuideWidget() const
{
    if (!ControlGuideWidget)
        return;

    ControlGuideWidget->ToggleWidget();
}

void AUIManager::UpdateSpeedLineWidget(const float CurrentSpeed, const float MaxSpeed) const
{
    if (!SpeedLineWidget)
        return;

    if (USpeedLineWidget* SpeedLine = Cast<USpeedLineWidget>(SpeedLineWidget))
    {
        SpeedLine->UpdateSpeedLines(CurrentSpeed, MaxSpeed);
    }
}

void AUIManager::ShowCutSceneImage(UTexture2D* Texture, float Duration)
{
    if (CutSceneImageWidget)
    {
        CutSceneImageWidget->ShowImageForSeconds(Texture, Duration);
    }
}
