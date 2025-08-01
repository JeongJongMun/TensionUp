#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CutSceneImageWidget.generated.h"

UCLASS()
class UNCLEWEB_API UCutSceneImageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Cutscene")
	void ShowImageForSeconds(UTexture2D* InTexture, float Duration = 3.0f);

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	class UImage* IMG_Cutscene;

	FTimerHandle HideTimerHandle;

	void HideImage();
};
