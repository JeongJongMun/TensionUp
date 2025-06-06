// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "CableComponent.h"
#include "Components/ActorComponent.h"
#include "CableActionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableAttachedAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableAttachingAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableDetachedAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableBoosterAction);

/* Cable Attached : When the cable is successfully attached to a target actor.
   Cable Attaching : When the cable is in the process of being attached.
   Cable Detached : When the cable is detached from the target actor.
   Steam Booster Active : When the steam booster is activated.
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
	void TryAttachCable();
	void TryDetachCable();
	void ShortenCable();
	void ExtendCable();
	bool IsCanAttachCable();
	bool IsCableAttaching() const { return bIsCableAttached; }
	void SetIsSteamBoosterActive(const bool bActive) { bIsSteamBoosterActive = bActive; }
	bool IsSteamBoosterActive() const { return bIsSteamBoosterActive; }
	
protected:
	virtual void BeginPlay() override;

private:
	bool IsCanAttachCable(FHitResult &HitResult);
	void OnCableAttaching(float DeltaTime);
	void OnCableLengthAdjust(float DeltaTime);
	void OnDrivingForce();
	void SetCable(const FVector& AttachLocation, AActor* HitActor);
	void ResetCable();

// --------------------
// Variables
// --------------------
public:
	UPROPERTY(EditAnywhere, Category = "CableComponent")
	TObjectPtr<UCableComponent> TargetCable;
	
	FOnCableAttachedAction OnCableAttachedAction;
	FOnCableDetachedAction OnCableDetachedAction;
	FOnCableAttachingAction OnCableAttachingAction;
	
	FOnCableBoosterAction OnSteamBoosterActive;

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
	float InitialCableLengthRatio = 0.7f;

	UPROPERTY(EditAnywhere, Category = "Config|Cable")
	float SteamBoosterForce = 1000.0f;
	
	TObjectPtr<APlayerController> CachedPC;
	TObjectPtr<class ATUCharacterPlayer> Owner;
	bool bIsCableAttached = false;
	bool bIsSteamBoosterActive = false;
	float CurrentCableLength = 0.0f;
	float TargetCableLength = 0.0f;
	FVector CableAttachPoint = FVector::ZeroVector;
};
