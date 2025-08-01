#include "CutSceneImageWidget.h"
#include "Components/Image.h"
#include "TimerManager.h"

void UCutSceneImageWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Hidden);
}

void UCutSceneImageWidget::ShowImageForSeconds(UTexture2D* InTexture, float Duration)
{
	if (!IMG_Cutscene || !InTexture) return;

	IMG_Cutscene->SetBrushFromTexture(InTexture);
	SetVisibility(ESlateVisibility::Visible);

	GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(HideTimerHandle, this, &UCutSceneImageWidget::HideImage, Duration, false);
}

void UCutSceneImageWidget::HideImage()
{
	SetVisibility(ESlateVisibility::Hidden);
}
