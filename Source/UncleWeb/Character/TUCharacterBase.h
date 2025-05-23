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
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxWalkSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxRunSpeed = 1000.0f;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float ZumpVelocity = 500.0f;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float JumpAirControl = 0.35f;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float JumpFallingLateralFriction = 0.0f;
};
