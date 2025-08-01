#include "CutSceneImageWidget.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"

void UCutSceneImageWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetVisibility(ESlateVisibility::Hidden);
    SetRenderOpacity(0.f);
}

void UCutSceneImageWidget::ShowImageForSeconds(UTexture2D* InTexture, float Duration)
{
    if (!CutsceneImage || !InTexture) return;

    CutsceneImage->SetBrushFromTexture(InTexture);
    SetVisibility(ESlateVisibility::Visible);
    SetRenderOpacity(1.f);

    FadeOutDelay = Duration;
    FadeDuration = 1.0f;
    ElapsedTime = 0.f;
    bIsFadingOut = false;
}

void UCutSceneImageWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (FadeOutDelay > 0.f)
    {
        FadeOutDelay -= InDeltaTime;
        if (FadeOutDelay <= 0.f)
        {
            bIsFadingOut = true;
            ElapsedTime = 0.f;
        }
    }

    if (bIsFadingOut)
    {
        ElapsedTime += InDeltaTime;
        float Alpha = FMath::Clamp(1.f - (ElapsedTime / FadeDuration), 0.f, 1.f);
        SetRenderOpacity(Alpha);

        if (Alpha <= 0.f)
        {
            SetVisibility(ESlateVisibility::Hidden);
            bIsFadingOut = false;
        }
    }
}
