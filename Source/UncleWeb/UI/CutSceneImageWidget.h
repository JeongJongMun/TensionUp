#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CutSceneImageWidget.generated.h"

class UImage;
class UTexture2D;

USTRUCT()
struct FFadeInfo
{
    GENERATED_BODY()

    TObjectPtr<UImage> Target = nullptr;
    float StartOpacity = 0.f;
    float EndOpacity = 0.f;
    int32 CurrentStep = 0;
    int32 TotalSteps = 0;
    bool bActive = false;
};

UCLASS()
class UNCLEWEB_API UCutSceneImageWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable)
    void PlayCutScene(UTexture2D* InTexture, float TotalDuration);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> CutsceneImage;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> BlackOverlay;

private:
    FTimerHandle FadeInHandle;
    FTimerHandle FadeOutHandle;
    FTimerHandle FadeTickHandle;

    float FadeStepTime = 0.05f;

    TArray<FFadeInfo> ActiveFades;

    void BeginFadeIn();
    void BeginFadeOut();
    void TickFade();
    void StartFade(UImage* TargetImage, float StartOpacity, float EndOpacity, float Duration);
    void ClearAllFades();
};