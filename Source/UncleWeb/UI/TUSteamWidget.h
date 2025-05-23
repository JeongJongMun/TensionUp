#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TUSteamWidget.generated.h"

UCLASS()
class UNCLEWEB_API UTUSteamWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateStaminaBar(float Current, float Max);
};
