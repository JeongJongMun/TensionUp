#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TUStaminaWidget.generated.h"

UCLASS()
class UNCLEWEB_API UTUStaminaWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateStaminaBar(float Current, float Max);
};
