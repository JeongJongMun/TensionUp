#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CutSceneImageWidget.generated.h"

class UImage;

UCLASS()
class UNCLEWEB_API UCutSceneImageWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void ShowImageForSeconds(UTexture2D* InTexture, float Duration);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UPROPERTY(meta = (BindWidget))
    UImage* CutsceneImage;

    float FadeOutDelay = 0.f;
    float FadeDuration = 1.f;
    float ElapsedTime = 0.f;
    bool bIsFadingOut = false;
};
