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
	bool IsCableAttached() const;

private:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartRunning();
	void StopRunning();
	void Dash();
	void TryParkour();
	void HandleJumpOrParkour();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void HandleUpdateSteamUI(float Current, float Max);

	UFUNCTION()
	void OnCableAttached();

	UFUNCTION()
	void OnCableDetached();

	void HandleAttachCable();
	void HandleDetachCable();

	void HandleShortenCable();
	void HandleExtendCable();

	void HandleJumpPressed();
	void HandleJumpReleased();

	void HandleStartSteamBooster();
	void HandleStopSteamBooster();

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
	UPROPERTY(EditAnywhere, Category = "Config|Dash")
	float DashStrength = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Steam")
	float DashSteamCost = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Steam")
	float CableSteamCost = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Steam")
	float SteamBoosterCost = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Player")
	float CableActionAirControl = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Config|Player")
	float CableFallingLateralFriction = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Player")
	float AirControlChangeIntervalSeconds = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Config|RepulsiveForce")
	float RepulsiveForceScaleFactor = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ParkourMaxDistance = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ParkourMaxHeight = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ParkourVaultUpForce = 200.f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ParkourVaultForwardForce = 400.f;


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
	TObjectPtr<UInputAction> SteamBoostAction;

	bool bIsRunning = false;
	bool bIsTryingParkour = false;
	bool bIsJumpInputActive = false;
};