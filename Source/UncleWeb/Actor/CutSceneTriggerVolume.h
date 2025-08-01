#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "CutSceneTriggerVolume.generated.h"

UCLASS()
class UNCLEWEB_API ACutSceneTriggerVolume : public ATriggerBox
{
    GENERATED_BODY()

public:
    ACutSceneTriggerVolume();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerEnter(AActor* OverlappedActor, AActor* OtherActor);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CutScene")
    UTexture2D* CutSceneTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CutScene")
    float CutSceneDuration = 6.f;
};
