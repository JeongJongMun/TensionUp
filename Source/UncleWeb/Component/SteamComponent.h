#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UncleWeb/Character/TUCharacterPlayer.h"
#include "SteamComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSteamChanged, float, CurrentSteam, float, MaxSteam);

UCLASS()
class UNCLEWEB_API USteamComponent : public UActorComponent
{
	GENERATED_BODY()

// --------------------
// Functions
// --------------------
public:
	USteamComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ConsumeSteam(float Amount);
	bool HasEnoughSteam(float Amount) const;
	void RecoverSteam(float DeltaTime);
	float GetCurrentSteam() const { return CurrentSteam; }

protected:
	virtual void BeginPlay() override;

// --------------------
// Variables
// --------------------
public:
	FOnSteamChanged OnSteamChanged;

private:
	UPROPERTY(EditAnywhere, Category = "Config|Steam")
	float MaxSteam = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Steam")
	float SteamRecoveryRate = 10.0f;
	
	TObjectPtr<ATUCharacterPlayer> Owner;
	float CurrentSteam = 100.0f;
};
