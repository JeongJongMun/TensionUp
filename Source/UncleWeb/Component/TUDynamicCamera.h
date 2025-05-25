// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TUDynamicCamera.generated.h"

UCLASS()
class UNCLEWEB_API UTUDynamicCamera : public UActorComponent
{
	GENERATED_BODY()
	
// --------------------
// Functions
// --------------------
public:	
	UTUDynamicCamera();
	void InitializeCamera();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
// --------------------
// Variables
// --------------------
public:
	TObjectPtr<USpringArmComponent> TargetSpringArm;
	
private:
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float MinArmLength = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float MaxArmLength = 600.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float MaxSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float CameraInterpolationSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	FVector CameraOffset = FVector(0.0f, 0.0f, 120.0f);
	
	TObjectPtr<UCharacterMovementComponent> MovementComponent;
};
