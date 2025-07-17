// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DynamicCamera.generated.h"

UCLASS()
class UNCLEWEB_API UDynamicCamera : public UActorComponent
{
	GENERATED_BODY()
	
// --------------------
// Functions
// --------------------
public:	
	UDynamicCamera();
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
	UPROPERTY()
	TObjectPtr<class USpringArmComponent> TargetSpringArm;
	
private:
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float MinSpringArmLength = 350.0f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float MidSpringArmLength = 750.0f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float MaxSpringArmLength = 1400.0f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float SpeedThreshold_A = 500.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float SpeedThreshold_B = 1300.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float SpeedThreshold_C = 1800.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float SpeedThreshold_D = 2500.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float CameraInterpolationSpeed = 12.0f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	FVector CameraOffset = FVector(0.0f, 0.0f, 120.0f);

	UPROPERTY()
	TObjectPtr<class ATUCharacterPlayer> Owner;
};
