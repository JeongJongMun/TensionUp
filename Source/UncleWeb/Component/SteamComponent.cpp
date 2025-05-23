#include "SteamComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

USteamComponent::USteamComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USteamComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Owner = CastChecked<ATUCharacterPlayer>(GetOwner());
	CurrentSteam = MaxSteam;
}

void USteamComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Owner->GetCharacterMovement()->IsMovingOnGround())
	{
		RecoverSteam(DeltaTime);
	}
}

void USteamComponent::ConsumeSteam(float Amount)
{
	if (!HasEnoughSteam(Amount)) return;

	CurrentSteam = FMath::Clamp(CurrentSteam - Amount, 0.0f, MaxSteam);
	OnSteamChanged.Broadcast(CurrentSteam, MaxSteam);
}

bool USteamComponent::HasEnoughSteam(float Amount) const
{
	return CurrentSteam >= Amount;
}

void USteamComponent::RecoverSteam(float DeltaTime)
{
	if (CurrentSteam < MaxSteam)
	{
		CurrentSteam = FMath::Clamp(CurrentSteam + SteamRecoveryRate * DeltaTime, 0.0f, MaxSteam);
		OnSteamChanged.Broadcast(CurrentSteam, MaxSteam);
	}
}
