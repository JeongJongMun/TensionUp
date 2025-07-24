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
	void InitializeCamera() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float CalculateTargetArmLength(float CurrentSpeed) const;
	void UpdatePostProcessVolume(const float DeltaTime) const;
	
// --------------------
// Variables
// --------------------
public:
	UPROPERTY()
	TObjectPtr<class USpringArmComponent> TargetSpringArm;

	UPROPERTY()
	TObjectPtr<class UCameraComponent> FollowCamera;
	
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
	
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float MotionBlurAmountMax = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float MotionBlurDistortionMax = 15.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float FOVMin = 90.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float FOVMax = 100.0f;

	UPROPERTY()
	TObjectPtr<class ATUCharacterPlayer> Owner;

	UPROPERTY()
	TObjectPtr<APostProcessVolume> PostProcessVolume;

	UPROPERTY()
	TObjectPtr<UMaterialParameterCollection> MaterialParameterCollection;

	UPROPERTY()
	TObjectPtr<UMaterialParameterCollectionInstance> MaterialParameterCollectionInstance;

	FName RadialBlurIntensityName = FName("BlurIntensity");
	FName RadialBlurRadiusName = FName("BlurRadius");

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float RadialBlurIntensityMax = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float RadialBlurRadiusMax = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Config|DynamicCamera")
	float PostProcessingInterpolationSpeed = 1.0f;
};
