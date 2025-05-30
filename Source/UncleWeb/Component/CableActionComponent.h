// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "CableComponent.h"
#include "Components/ActorComponent.h"
#include "CableActionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableAttachedAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableDetachedAction);

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
	void AttachCable();
	void DetachCable();
	void ShortenCable();
	void ExtendCable();
	bool IsCanAttachCable();
	bool IsCableAttached() const { return bIsCableAttached; }
	
protected:
	virtual void BeginPlay() override;

private:
	bool IsCanAttachCable(FHitResult &HitResult);
	void CalculateCableSwing(float DeltaTime);
	void ApplyDetachDrivingForce();
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
	
	TObjectPtr<APlayerController> CachedPC;
	TObjectPtr<class ATUCharacterPlayer> Owner;
	bool bIsCableAttached = false;
	float CurrentCableLength = 0.0f;
	float TargetCableLength = 0.0f;
	FVector CableAttachPoint = FVector::ZeroVector;
};
