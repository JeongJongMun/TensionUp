// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "CableComponent.h"
#include "Components/ActorComponent.h"
#include "CableActionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableAttachedAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableDetachedAction);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNCLEWEB_API UCableActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCableActionComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void AttachCable();
	void DetachCable();
	
	UPROPERTY(BlueprintAssignable, Category = "CableAction")
	FOnCableAttachedAction OnCableAttachedAction;

	UPROPERTY(BlueprintAssignable, Category = "CableAction")
	FOnCableDetachedAction OnCableDetachedAction;
	
	TObjectPtr<UCableComponent> TargetCable;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CableAction")
	float CableMaxLength = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CableAction")
	float CableDrivingForce = 10000000.0f;

protected:
	virtual void BeginPlay() override;

private:
	TObjectPtr<ACharacter> Owner;
	bool bIsCableAttached = false;
	float CurrentCableLength = 0.0f;
	FVector CableAttachPoint = FVector::ZeroVector;

	void CalculateCableSwing();
	bool FindCableAttachPoint(FVector& OutLocation, AActor*& OutHitActor);
	void ApplyDetachDrivingForce();
	void SetCable(const FVector& AttachLocation, AActor* HitActor);
	void ResetCable();
		
};
