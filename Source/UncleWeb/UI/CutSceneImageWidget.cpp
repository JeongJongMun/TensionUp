#include "CutSceneImageWidget.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

void UCutSceneImageWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetVisibility(ESlateVisibility::Hidden);
    if (CutsceneImage) CutsceneImage->SetRenderOpacity(0.f);
    if (BlackOverlay) BlackOverlay->SetRenderOpacity(0.f);
}

void UCutSceneImageWidget::PlayCutScene(UTexture2D* InTexture, float TotalDuration)
{
    if (!InTexture || !CutsceneImage || !BlackOverlay) return;

    ClearAllFades();

    SetVisibility(ESlateVisibility::Visible);
    CutsceneImage->SetBrushFromTexture(InTexture);
    CutsceneImage->SetRenderOpacity(0.f);
    BlackOverlay->SetRenderOpacity(1.f);


    GetWorld()->GetTimerManager().SetTimer(FadeInHandle, this, &UCutSceneImageWidget::BeginFadeIn, 0.5f, false);

    const float FadeOutStart = TotalDuration - 1.0f;
    GetWorld()->GetTimerManager().SetTimer(FadeOutHandle, this, &UCutSceneImageWidget::BeginFadeOut, FadeOutStart, false);
}

void UCutSceneImageWidget::BeginFadeIn()
{
    StartFade(CutsceneImage, 0.f, 1.f, 1.0f);
}

void UCutSceneImageWidget::BeginFadeOut()
{
    StartFade(CutsceneImage, 1.f, 0.f, 1.0f);
    StartFade(BlackOverlay, 1.f, 0.f, 1.0f);
}

void UCutSceneImageWidget::StartFade(UImage* TargetImage, float StartOpacity, float EndOpacity, float Duration)
{
    if (!TargetImage) return;

    FFadeInfo NewFade;
    NewFade.Target = TargetImage;
    NewFade.StartOpacity = StartOpacity;
    NewFade.EndOpacity = EndOpacity;
    NewFade.CurrentStep = 0;
    NewFade.TotalSteps = FMath::CeilToInt(Duration / FadeStepTime);
    NewFade.bActive = true;

    ActiveFades.Add(NewFade);

    if (!GetWorld()->GetTimerManager().IsTimerActive(FadeTickHandle))
    {
        GetWorld()->GetTimerManager().SetTimer(FadeTickHandle, this, &UCutSceneImageWidget::TickFade, FadeStepTime, true);
    }
}

void UCutSceneImageWidget::TickFade()
{
    bool bHasActiveFades = false;
    bool bShouldHideWidget = false;

    for (int32 i = ActiveFades.Num() - 1; i >= 0; i--)
    {
        FFadeInfo& FadeInfo = ActiveFades[i];

        if (!FadeInfo.bActive || !FadeInfo.Target)
        {
            ActiveFades.RemoveAt(i);
            continue;
        }

        float Alpha = FMath::Lerp(FadeInfo.StartOpacity, FadeInfo.EndOpacity,
            static_cast<float>(FadeInfo.CurrentStep) / FadeInfo.TotalSteps);
        FadeInfo.Target->SetRenderOpacity(Alpha);

        FadeInfo.CurrentStep++;

        if (FadeInfo.CurrentStep > FadeInfo.TotalSteps)
        {
            FadeInfo.bActive = false;

            if (FadeInfo.Target == CutsceneImage && FadeInfo.EndOpacity <= 0.f)
            {
                bShouldHideWidget = true;
            }

            ActiveFades.RemoveAt(i);
        }
        else
        {
            bHasActiveFades = true;
        }
    }


    if (!bHasActiveFades)
    {
        GetWorld()->GetTimerManager().ClearTimer(FadeTickHandle);

        if (bShouldHideWidget)
        {
            SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void UCutSceneImageWidget::ClearAllFades()
{
    ActiveFades.Empty();
    GetWorld()->GetTimerManager().ClearTimer(FadeTickHandle);
}