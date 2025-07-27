// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "TUCharacterBase.h"
#include "UncleWeb/Manager/SoundManager.h"
#include "UncleWeb/Util/TUDefines.h"
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
	bool IsDashing() const;
	void PlayAnimation(const EPlayerAnimationMontageType AnimType, float InPlayRate = 1.0f, const FName SectionName = NAME_None);
	void SetMouseSensitivity(const float Sensitivity) { MouseSensitivity = Sensitivity; }
	virtual void Landed(const FHitResult& Hit) override;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	void InitializeInputActions();
	void InitializeAnimations();
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartRunning();
	void StopRunning();
	void Dash();
	void HandleJumpOrParkour();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void HandleUpdateSteamUI(float Current, float Max);

	UFUNCTION()
	void OnBeginLaunch();

	UFUNCTION()
	void OnBeginRetract();

	void HandleLaunchCable();
	void HandleRetractCable();

	void HandleShortenCable();
	void HandleExtendCable();

	void HandleJumpPressed();
	void HandleJumpReleased();

	void HandleStartSteamBooster();
	void HandleStopSteamBooster();

	void HandleSteamChargerCollision();

	void RotateToCameraDirection();
	void HandleShowSettingWidget();
	void ToggleControlGuideWidget();

// --------------------
// Variables
// --------------------
protected:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<class UDynamicCamera> DynamicCameraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<class UCableActionComponent> CableActionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<class USteamComponent> SteamComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<class UParkourComponent> ParkourComponent;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<class UCameraComponent> FollowCamera;

	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> NiagaraCompSteamVfxR;
	
	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> NiagaraCompSteamVfxL;
	
	UPROPERTY()
	TObjectPtr<class UNiagaraSystem> NiagaraSystemSteamVfx;


private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UCableComponent> CableComponent;
	
	UPROPERTY()
	TMap<EPlayerAnimationMontageType, TObjectPtr<UAnimMontage>> AnimMontageMap;
	TMap<EPlayerAnimationMontageType, TArray<FName>> AnimSectionMap;
	
	UPROPERTY(EditAnywhere, Category = "Config|Dash")
	float DashStrength = 1400.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Steam")
	float DashSteamCost = 35.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Steam")
	float CableSteamCost = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Steam")
	float SteamBoosterCost = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Player")
	float CableActionAirControl = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Config|Player")
	float CableFallingLateralFriction = -0.1f;

	UPROPERTY(EditAnywhere, Category = "Config|Player")
	float MovementModeTransitionDelay = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Config|RepulsiveForce")
	float RepulsiveForceScaleFactor = 0.75f;
	
	UPROPERTY()
	TObjectPtr<class AUIManager> UIManager;
	
	UPROPERTY()
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	UPROPERTY()
	TMap<EInputActionType, TSoftObjectPtr<UInputAction>> InputActionPaths;

	UPROPERTY()
	TMap<EInputActionType, TObjectPtr<UInputAction>> InputActionMap;
	
	UPROPERTY()
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY()
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY()
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY()
	TObjectPtr<UInputAction> LeftClickAction;
	
	UPROPERTY()
	TObjectPtr<UInputAction> DashAction;
	
	UPROPERTY()
	TObjectPtr<UInputAction> ShortenCableAction;
	
	UPROPERTY()
	TObjectPtr<UInputAction> ExtendCableAction;
	
	UPROPERTY()
	TObjectPtr<UInputAction> RunAction;
	
	UPROPERTY()
	TObjectPtr<UInputAction> SteamBoostAction;

	UPROPERTY()
	TObjectPtr<UInputAction> ShowSettingsAction;
	
	UPROPERTY()
	TObjectPtr<UInputAction> ToggleControlGuideAction;

	bool bIsTryingParkour = false;
	bool bIsJumpInputActive = false;
	bool bIsDashing = false;
	
	FName CableSocketName = TEXT("cable_r");
	FName SteamEffectSocketNameL = TEXT("foot_l_Socket");
	FName SteamEffectSocketNameR = TEXT("foot_r_Socket");

	float MouseSensitivity = 1.0f;

	// Sound
	UPROPERTY()
	TObjectPtr<USoundManager> SoundManager;

	FTimerHandle WalkSoundTimerHandle;

	float WalkSoundInterval = 1.0f;
	float RunSoundInterval = 0.25f;
	bool bIsRunningSound = false;

	UFUNCTION()
	void PlayWalkSound();

};