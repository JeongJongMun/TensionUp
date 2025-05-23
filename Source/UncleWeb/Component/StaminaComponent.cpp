#include "StaminaComponent.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentStamina = MaxStamina;
}

void UStaminaComponent::ConsumeStamina(float Amount)
{
	if (!HasEnoughStamina(Amount)) return;

	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.0f, MaxStamina);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

bool UStaminaComponent::HasEnoughStamina(float Amount) const
{
	return CurrentStamina >= Amount;
}

void UStaminaComponent::RecoverStamina(float DeltaTime)
{
	if (CurrentStamina < MaxStamina)
	{
		CurrentStamina = FMath::Clamp(CurrentStamina + StaminaRecoveryRate * DeltaTime, 0.0f, MaxStamina);
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}
