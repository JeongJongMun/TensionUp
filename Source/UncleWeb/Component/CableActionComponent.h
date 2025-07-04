// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "CableComponent.h"
#include "Components/ActorComponent.h"
#include "UncleWeb/Util/TUDefines.h"
#include "CableActionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableBeginLaunchAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableWhileLaunchAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableEndLaunchAction);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableWhileAttachAction);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableBeginRetractAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableWhileRetractAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableEndRetractAction);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWhileSteamBoosterAction);

/* BeginLaunch : When the cable is launched towards a target.
 * WhileLaunch : During the launch process, the cable is moving towards the target.
 * EndLaunch : When the cable has reached the target

 * WhileAttach : When the cable is attached to the target actor.

 * BeginRetract : When the cable is being retracted.
 * WhileRetract : During the retraction process.
 * EndRetract : When the cable is fully retracted.

 * WhileSteamBooster : When the steam booster is activated.
 */
UCLASS()
class UNCLEWEB_API UCableActionComponent : public UActorComponent
{
	GENERATED_BODY()

// --------------------
// Functions
// --------------------
public:	
	UCableActionComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void TryLaunchCable();
	void TryRetractCable();
	bool IsCanLaunchCable();
	void SetIsSteamBoosterActive(const bool bActive) { bIsSteamBoosterActive = bActive; }
	bool IsSteamBoosterActive() const { return bIsSteamBoosterActive; }
	ECableState GetCableState() const { return CurrentCableState; }
	bool IsCableActive() const { return CurrentCableState != ECableState::Idle; }
	bool IsCableAttaching() const { return CurrentCableState == ECableState::Attaching; }
	
	// Deprecated : Cable Length Adjust Func
	void ShortenCable();
	void ExtendCable();
	
protected:
	virtual void BeginPlay() override;

private:
	bool IsCanLaunchCable(FHitResult &HitResult);
	void OnWhileAttach(float DeltaTime);
	void TryDrivingForce();
	void LaunchCable(const FVector& HitLocation, AActor* HitActor);
	void RetractCable();
	void ResetCable();
	void SetOwnerRotationToCable(const FVector& CableDirection, float DeltaTime);
	void PlayCableVfx(const float DeltaTime);

	// Deprecated : Cable Length Adjust Func
	void OnCableLengthAdjust(float DeltaTime);

// --------------------
// Variables
// --------------------
public:
	UPROPERTY(EditAnywhere, Category = "CableComponent")
	TObjectPtr<UCableComponent> TargetCable;

	FOnCableBeginLaunchAction OnBeginLaunchAction;
	FOnCableWhileLaunchAction OnWhileLaunchAction;
	FOnCableEndLaunchAction OnEndLaunchAction;

	FOnCableWhileAttachAction OnWhileAttachAction;

	FOnCableBeginRetractAction OnBeginRetractAction;
	FOnCableWhileRetractAction OnWhileRetractAction;
	FOnCableEndRetractAction OnEndRetractAction;

	FOnWhileSteamBoosterAction OnWhileSteamBoosterAction;

private:
	UPROPERTY(EditAnywhere, Category = "Config|Cable")
	float CableMaxLength = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Cable")
	float CableMinLength = 300.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Cable")
	float CableDrivingForce = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Cable")
	float CableAdjustSpeed = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Cable")
	float CableInterpSpeed = 5.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Cable")
	float SteamBoosterForce = 1000.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Cable")
	float CableLaunchSpeed = 2500.0f;

	UPROPERTY()
	TObjectPtr<APlayerController> CachedPC;
	
	UPROPERTY()
	TObjectPtr<class ATUCharacterPlayer> Owner;
	
	UPROPERTY()
	TObjectPtr<AActor> AttachedActor;
	
	bool bIsSteamBoosterActive = false;
	
	float TargetCableLength = 0.0f;
	float VfxProgress = 0.0f;
	
	FVector AttachedLocation = FVector::ZeroVector;
	ECableState CurrentCableState = ECableState::Idle;
	
	// Deprecated : Cable Length Adjust Func
	float CurrentCableLength = 0.0f;
};