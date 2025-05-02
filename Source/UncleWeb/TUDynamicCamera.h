// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TUDynamicCamera.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNCLEWEB_API UTUDynamicCamera : public UActorComponent
{
	GENERATED_BODY()
	

public:	
	UTUDynamicCamera();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TObjectPtr<USpringArmComponent> TargetSpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinArmLength = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxArmLength = 600.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float InterpolationSpeed = 5.0f;
	
protected:
	virtual void BeginPlay() override;

private:
	TObjectPtr<UCharacterMovementComponent> MovementComponent;
		
};
