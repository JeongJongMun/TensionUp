// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "TUCharacterBase.h"
#include "TUDynamicCamera.h"
#include "TUCharacterPlayer.generated.h"

UCLASS()
class UNCLEWEB_API ATUCharacterPlayer : public ATUCharacterBase
{
	GENERATED_BODY()

public:
	ATUCharacterPlayer();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UTUDynamicCamera> DynamicCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable")
	TObjectPtr<class UCableComponent> CableComponent;
     
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable")
	float CableMaxLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable")
	float CableDrivingForce;

	// Dash 값 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashStrength = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashStaminaCost = 20.0f;

<<<<<<< Updated upstream
	// 스태미나 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	float CurrentStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaRecoveryRate = 10.0f;
=======
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	TObjectPtr<class UStaminaComponent> StaminaComponent;
	// Cable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CableAction")
	float CableStaminaCost = 10.0f;
>>>>>>> Stashed changes


	// HUD
	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<class UUserWidget> HUDClass;

	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<class UUserWidget> StaminaWidgetClass;

	UPROPERTY()
	TObjectPtr<class UUserWidget> StaminaWidget;

	UPROPERTY()
	TObjectPtr<class UUserWidget> HUDWidget;

private:
<<<<<<< Updated upstream
	FVector CableAttachPoint = FVector::ZeroVector;
	float CurrentCableLength = 0.0f;
	bool bIsCableAttached = false;
	void CalculateCableSwing();
	bool FindCableAttachPoint(FVector& OutLocation, AActor*& OutHitActor);
	void AttachCable();
	void DetachCable();
	void ApplyDetachDrivingForce();
	void SetCable(const FVector& AttachLocation, AActor* HitActor);
	void ResetCable();
	void Dash();
	void ConsumeStamina(float Amount);
	bool HasEnoughStamina(float Amount) const;
	void UpdateStaminaUI();
=======
	void Dash();
	
	UFUNCTION()
	void ConsumeCableStamina();

	UFUNCTION()
	void UpdateStaminaUI(float Current, float Max);
	
	void HandleAttachCable();
	void HandleDetachCable();

	void ZoomInCable();
	void ZoomOutCable();

>>>>>>> Stashed changes

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> DashAction; // 임시 R키
};