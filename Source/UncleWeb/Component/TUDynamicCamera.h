// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

private:
	float CalculateTargetArmLength(float CurrentSpeed) const;
	
// --------------------
// Variables
// --------------------
public:
	TObjectPtr<class USpringArmComponent> TargetSpringArm;
	
private:
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float MinSpringArmLength = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float MidSpringArmLength = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float MaxSpringArmLength = 900.0f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float SpeedThreshold_A = 500.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float SpeedThreshold_B = 1000.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float SpeedThreshold_C = 1500.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float SpeedThreshold_D = 2000.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float CameraInterpolationSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	FVector CameraOffset = FVector(0.0f, 0.0f, 120.0f);
	
	TObjectPtr<class ATUCharacterPlayer> Owner;
};
