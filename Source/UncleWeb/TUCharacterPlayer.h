// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "TUCharacterBase.h"
#include "TUCharacterPlayer.generated.h"

UCLASS()
class UNCLEWEB_API ATUCharacterPlayer : public ATUCharacterBase
{
	GENERATED_BODY()

public:
	ATUCharacterPlayer();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DynamicCamera")
	TObjectPtr<class UTUDynamicCamera> DynamicCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CableAction")
	TObjectPtr<class UCableActionComponent> CableActionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashStrength = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashStaminaCost = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float MaxStamina = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaRecoveryRate = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float CableStaminaCost = 10.0f;

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
	float CurrentStamina = 100.0f;
	
	void Dash();
	void ConsumeStamina(float Amount);
	bool HasEnoughStamina(float Amount) const;
	void UpdateStaminaUI();
	
	UFUNCTION()
	void ConsumeCableStamina();
	
	void HandleAttachCable();
	void HandleDetachCable();

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
	TObjectPtr<class UInputAction> DashAction;
};