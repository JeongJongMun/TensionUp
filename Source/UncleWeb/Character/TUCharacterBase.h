// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TUCharacterBase.generated.h"

UCLASS()
class UNCLEWEB_API ATUCharacterBase : public ACharacter
{
	GENERATED_BODY()

// --------------------
// Functions
// --------------------
public:
	ATUCharacterBase();
	
// --------------------
// Variables
// --------------------
protected:
	UPROPERTY(EditAnywhere, Category = "Config|Player")
	float MaxWalkSpeed = 450.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Player")
	float MaxRunSpeed = 800.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Player")
	float ZumpVelocity = 500.0f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Player")
	float AirControl = 0.75f;
	
	UPROPERTY(EditAnywhere, Category = "Config|Player")
	float FallingLateralFriction = 0.45f;

	UPROPERTY(EditAnywhere, Category = "Config|Player")
	float GravityScale = 1.0f;
};
