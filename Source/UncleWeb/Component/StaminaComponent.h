#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, CurrentStamina, float, MaxStamina);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UNCLEWEB_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStaminaComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaRecoveryRate = 10.0f;

	UPROPERTY(BlueprintAssignable, Category = "Stamina")
	FOnStaminaChanged OnStaminaChanged;

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void ConsumeStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool HasEnoughStamina(float Amount) const;

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void RecoverStamina(float DeltaTime);

	float GetCurrentStamina() const { return CurrentStamina; }

protected:
	virtual void BeginPlay() override;

private:
	float CurrentStamina;
};
