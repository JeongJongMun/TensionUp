// Fill out your copyright notice in the Description page of Project Settings.

#include "TUCharacterPlayer.h"
#include "CableActionComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "TUDynamicCamera.h"
#include "GameFramework/CharacterMovementComponent.h"

ATUCharacterPlayer::ATUCharacterPlayer()
{
	// Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	DynamicCameraComponent = CreateDefaultSubobject<UTUDynamicCamera>(TEXT("DynamicCameraComponent"));
	DynamicCameraComponent->TargetSpringArm = CameraBoom;

	CableActionComponent = CreateDefaultSubobject<UCableActionComponent>(TEXT("CableActionComponent"));
	CableActionComponent->TargetCable = CreateDefaultSubobject<UCableComponent>(TEXT("CableComponent"));
	CableActionComponent->TargetCable->SetupAttachment(GetRootComponent(), "Swing");
	CableActionComponent->TargetCable->NumSegments = 1;
	CableActionComponent->TargetCable->CableWidth = 2.0f;
	CableActionComponent->TargetCable->bAttachEnd = true;

	// Input
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Input/IMC_Default.IMC_Default'"));
	if (nullptr != InputMappingContextRef.Object)
	{
		DefaultMappingContext = InputMappingContextRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Move.IA_Move'"));
	if (nullptr != InputActionMoveRef.Object)
	{
		MoveAction = InputActionMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionJumpRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Jump.IA_Jump'"));
	if (nullptr != InputActionJumpRef.Object)
	{
		JumpAction = InputActionJumpRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionLookRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Look.IA_Look'"));
	if (nullptr != InputActionLookRef.Object)
	{
		LookAction = InputActionLookRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionLeftClickRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_LeftClick.IA_LeftClick'"));
	if (nullptr != InputActionLeftClickRef.Object)
	{
		LeftClickAction = InputActionLeftClickRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionDashRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Dash.IA_Dash'"));
	if (InputActionDashRef.Succeeded())
	{
		DashAction = InputActionDashRef.Object;
	}
	
	// Widget
	static ConstructorHelpers::FClassFinder<UUserWidget>HUD(TEXT("WidgetBlueprint'/Game/UI/WBP_Crosshair.WBP_Crosshair_C'"));
	if (HUD.Succeeded())
	{
		HUDClass = HUD.Class;
	}
	// Stamina Widget
	static ConstructorHelpers::FClassFinder<UUserWidget> StaminaHUD(TEXT("WidgetBlueprint'/Game/UI/WBP_Stamina.WBP_Stamina_C'"));
	if (StaminaHUD.Succeeded())
	{
		StaminaWidgetClass = StaminaHUD.Class;
	}
}

void ATUCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
		//Subsystem->RemoveMappingContext(DefaultMappingContext);
	}
	
	// HUD
	HUDWidget = CreateWidget<UUserWidget>(PlayerController, HUDClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport();
	}

	if (StaminaWidgetClass)
	{
		StaminaWidget = CreateWidget<UUserWidget>(PlayerController, StaminaWidgetClass);
		if (StaminaWidget)
		{
			StaminaWidget->AddToViewport();
		}
	}

	// Stamina
	CableActionComponent->OnCableAttachedAction.AddDynamic(this, &ATUCharacterPlayer::ConsumeCableStamina);
}

void ATUCharacterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetCharacterMovement()->IsMovingOnGround())
	{
		if (CurrentStamina < MaxStamina)
		{
			CurrentStamina = FMath::Clamp(CurrentStamina + StaminaRecoveryRate * DeltaTime, 0.0f, MaxStamina);
			UpdateStaminaUI();
		}
	}
}

void ATUCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATUCharacterPlayer::StopJumping);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATUCharacterPlayer::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATUCharacterPlayer::Look);
	EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::HandleAttachCable);
	EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Completed, this, &ATUCharacterPlayer::HandleDetachCable);
	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::Dash);
}

void ATUCharacterPlayer::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void ATUCharacterPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ATUCharacterPlayer::Dash()
{
	if (HasEnoughStamina(DashStaminaCost))
	{
		FVector DashDirection = GetActorForwardVector();
		LaunchCharacter(DashDirection * DashStrength, true, true);

		ConsumeStamina(DashStaminaCost);
	}
}

void ATUCharacterPlayer::ConsumeStamina(float Amount)
{
	if (!HasEnoughStamina(Amount)) return;
	
	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.0f, MaxStamina);
	UpdateStaminaUI();
}

bool ATUCharacterPlayer::HasEnoughStamina(float Amount) const
{
	return CurrentStamina >= Amount;
}

void ATUCharacterPlayer::UpdateStaminaUI()
{
	if (UFunction* Func = StaminaWidget->FindFunction(TEXT("UpdateStaminaBar")))
	{
		struct FParams
		{
			float Current;
			float Max;
		};

		FParams Params;
		Params.Current = CurrentStamina;
		Params.Max = MaxStamina;

		StaminaWidget->ProcessEvent(Func, &Params);
	}
}

void ATUCharacterPlayer::ConsumeCableStamina()
{
	ConsumeStamina(CableStaminaCost);
}

void ATUCharacterPlayer::HandleAttachCable()
{
	if (!CableActionComponent) return;
	
	CableActionComponent->AttachCable();
}

void ATUCharacterPlayer::HandleDetachCable()
{
	if (!CableActionComponent) return;
	
	CableActionComponent->DetachCable();
}

