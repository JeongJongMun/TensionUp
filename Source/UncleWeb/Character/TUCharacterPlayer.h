// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "TUCharacterBase.h"
#include "TUCharacterPlayer.generated.h"

UCLASS()
class UNCLEWEB_API ATUCharacterPlayer : public ATUCharacterBase
{
	GENERATED_BODY()

// --------------------
// Functions
// --------------------
public:
	ATUCharacterPlayer();

private:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartRunning();
	void StopRunning();
	void Dash();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UFUNCTION()
	void HandleUpdateSteamUI(float Current, float Max);
	
	UFUNCTION()
	void HandleConsumeCableSteam();
	
	void HandleAttachCable();
	void HandleDetachCable();

	void HandleShortenCable();
	void HandleExtendCable();

// --------------------
// Variables
// --------------------
protected:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<class UTUDynamicCamera> DynamicCameraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<class UCableActionComponent> CableActionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<class USteamComponent> SteamComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<class UCameraComponent> FollowCamera;

private:
	UPROPERTY(EditAnywhere, Category = "Movement")
	float DashStrength = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Steam")
	float DashSteamCost = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Steam")
	float CableSteamCost = 10.0f;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float CableActionAirControl = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float CableFallingLateralFriction = 0.0f;
		
	UPROPERTY(EditAnywhere, Category = "CableAction")
	float AirControlChangeInterval = 0.5f;

	UPROPERTY(EditAnywhere, Category = "RepulsiveForce")
	float RepulsiveForceScaleFactor = 1.0f;
	
	TObjectPtr<class AUIManager> UIManager;
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;
	TObjectPtr<UInputAction> JumpAction;
	TObjectPtr<UInputAction> MoveAction;
	TObjectPtr<UInputAction> LookAction;
	TObjectPtr<UInputAction> LeftClickAction;
	TObjectPtr<UInputAction> DashAction;
	TObjectPtr<UInputAction> ShortenCableAction;
	TObjectPtr<UInputAction> ExtendCableAction;
	TObjectPtr<UInputAction> RunAction;
	
	bool bIsRunning = false;
};