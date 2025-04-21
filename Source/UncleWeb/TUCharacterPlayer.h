// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "CableComponent.h"
#include "TUCharacterBase.h"
#include "TUCharacterPlayer.generated.h"

UCLASS()
class UNCLEWEB_API ATUCharacterPlayer : public ATUCharacterBase
{
	GENERATED_BODY()

public:
	ATUCharacterPlayer();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable")
	UCableComponent* CableComponent;
     
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable")
	float MaxCableLength = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable")
	float CableStrength = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable")
	float DampingFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable")
	float CableDrivingForce = 100000.0f;

	// HUD
	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<class UUserWidget> HUDClass;

	UPROPERTY()
	TObjectPtr<class UUserWidget> HUDWidget;

private:
	FVector CableEndLocation;
	float NewCableLength = 0.0f;
	bool bIsCableAttached = false;
	void CalculateSwingForce();
	void UpdateCableEndLocation();
	bool FindCableAttachPoint(FVector& OutLocation, FVector& OutNormal);
	void FireCable();
	void ReleaseCable();

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	// Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	// Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LeftClickAction;
};