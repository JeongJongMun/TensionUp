// Fill out your copyright notice in the Description page of Project Settings.

#include "TUCharacterPlayer.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UncleWeb/Component/CableActionComponent.h"
#include "UncleWeb/Component/SteamComponent.h"
#include "UncleWeb/Component/TUDynamicCamera.h"
#include "DrawDebugHelpers.h"

#include "UncleWeb/UI/UIManager.h"
#include "UncleWeb/Util/TUDefines.h"

ATUCharacterPlayer::ATUCharacterPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	DynamicCameraComponent = CreateDefaultSubobject<UTUDynamicCamera>(TEXT("DynamicCameraComponent"));
	DynamicCameraComponent->TargetSpringArm = CameraBoom;
	DynamicCameraComponent->InitializeCamera();

	CableActionComponent = CreateDefaultSubobject<UCableActionComponent>(TEXT("CableActionComponent"));
	CableActionComponent->TargetCable = CreateDefaultSubobject<UCableComponent>(TEXT("CableComponent"));
	CableActionComponent->TargetCable->SetupAttachment(GetRootComponent(), "Swing");
	CableActionComponent->TargetCable->NumSegments = 1;
	CableActionComponent->TargetCable->CableWidth = 2.0f;
	CableActionComponent->TargetCable->bAttachEnd = true;

	SteamComponent = CreateDefaultSubobject<USteamComponent>(TEXT("SteamComponent"));

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

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionRunRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Run.IA_Run'"));
	if (InputActionRunRef.Succeeded())
	{
		RunAction = InputActionRunRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShortenCableRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_ZoomIn.IA_ZoomIn'"));
	if (InputActionShortenCableRef.Succeeded())
	{
		ShortenCableAction = InputActionShortenCableRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionExtendCableRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_ZoomOut.IA_ZoomOut'"));
	if (InputActionExtendCableRef.Succeeded())
	{
		ExtendCableAction = InputActionExtendCableRef.Object;
	}
}

bool ATUCharacterPlayer::IsCableAttached() const
{
	return CableActionComponent && CableActionComponent->IsCableAttached();
}

void ATUCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
		//Subsystem->RemoveMappingContext(DefaultMappingContext);
	}

	UIManager = GetWorld()->SpawnActor<AUIManager>(AUIManager::StaticClass());
	UIManager->InitializeUI(PC);

	SteamComponent->OnSteamChanged.AddDynamic(this, &ATUCharacterPlayer::HandleUpdateSteamUI);
	CableActionComponent->OnCableAttachedAction.AddDynamic(this, &ATUCharacterPlayer::OnCableAttached);
	CableActionComponent->OnCableDetachedAction.AddDynamic(this, &ATUCharacterPlayer::OnCableDetached);

	// repulsive force
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ATUCharacterPlayer::OnHit);
}

void ATUCharacterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CableActionComponent->IsCanAttachCable())
	{
		UIManager->SetCrosshairColor(ECrosshairStateType::Active);
	}
	else
	{
		UIManager->SetCrosshairColor(ECrosshairStateType::Default);
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
	// Temporary Disabled 
	// EnhancedInputComponent->BindAction(ShortenCableAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::HandleShortenCable);
	// EnhancedInputComponent->BindAction(ExtendCableAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::HandleExtendCable);
	EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::StartRunning);
	EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ATUCharacterPlayer::StopRunning);
	//for parkour
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::TryParkour);

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
	if (!SteamComponent->HasEnoughSteam(DashSteamCost))
		return;

	if (SteamComponent)
	{
		FRotator CameraRotation = FollowCamera->GetComponentRotation();
		FRotator YawRotation(0, CameraRotation.Yaw, 0);
		FVector DashDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		LaunchCharacter(DashDirection * DashStrength, true, true);

		SteamComponent->ConsumeSteam(DashSteamCost);
	}
}

void ATUCharacterPlayer::HandleJumpOrParkour()
{
	if (GetCharacterMovement()->IsFalling())
	{
		TryParkour();
	}
	else
	{
		Jump();
	}
}


void ATUCharacterPlayer::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(GetRootComponent());
	FVector CurrentVelocity = RootComp->GetPhysicsLinearVelocity();
	float CurrentSpeed = CurrentVelocity.Size();

	// if IncidenceAngle close to 1, it means the character is hitting the wall almost perpendicular
	float IncidenceAngle = FVector::DotProduct(CurrentVelocity.GetSafeNormal(), -Hit.ImpactNormal);
	bool bIsFalling = GetCharacterMovement()->IsFalling();

	if (CurrentSpeed < 500.0f || !bIsFalling || IncidenceAngle < 0.5f)
		return;

	FVector ReflectionDirection = FMath::GetReflectionVector(CurrentVelocity.GetSafeNormal(), Hit.ImpactNormal);

	LaunchCharacter(ReflectionDirection * CurrentSpeed * RepulsiveForceScaleFactor, true, true);
	UE_LOG(LogTemp, Log, TEXT("[%s] Hit occurred in speed: %f"), CURRENT_CONTEXT, CurrentSpeed);
}

void ATUCharacterPlayer::StartRunning()
{
	bIsRunning = true;
	GetCharacterMovement()->MaxWalkSpeed = MaxRunSpeed;
}

void ATUCharacterPlayer::StopRunning()
{
	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
}

// ----- Handler
void ATUCharacterPlayer::HandleAttachCable()
{
	CableActionComponent->AttachCable();
}

void ATUCharacterPlayer::HandleDetachCable()
{
	CableActionComponent->DetachCable();
}

void ATUCharacterPlayer::HandleShortenCable()
{
	CableActionComponent->ShortenCable();
}

void ATUCharacterPlayer::HandleExtendCable()
{
	CableActionComponent->ExtendCable();
}

void ATUCharacterPlayer::HandleUpdateSteamUI(const float Current, const float Max)
{
	UIManager->UpdateSteamUI(Current, Max);
}

void ATUCharacterPlayer::OnCableAttached()
{
	FTimerHandle TimerHandle;
	Owner->GetWorldTimerManager().SetTimer(TimerHandle, [this]()
		{
			GetCharacterMovement()->AirControl = CableActionAirControl;
		}, AirControlChangeIntervalSeconds, false);

	SteamComponent->ConsumeSteam(CableSteamCost);
}

void ATUCharacterPlayer::OnCableDetached()
{
	GetCharacterMovement()->AirControl = JumpAirControl;
}

void ATUCharacterPlayer::TryParkour()
{
	//UE_LOG(LogTemp, Warning, TEXT("TryParkour!"));

	if (!GetCharacterMovement()->IsFalling())
		return;

	// 기준 위치를 약간 상향 (허리 또는 가슴 위치)
	FVector TraceStart = GetActorLocation() + FVector(0, 0, 35);
	FVector TraceEnd = TraceStart + GetActorForwardVector() * ParkourMaxDistance;

	// 박스 크기 설정 (X는 작게, YZ는 캐릭터 어깨보다 조금 작게)
	FVector BoxHalfSize = FVector(10.f, 30.f, 30.f); // 전방 얇은 박스

	// 회전은 Forward 방향 기준
	FRotator Orientation = GetActorRotation();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		FQuat(Orientation),
		ECC_Visibility,
		FCollisionShape::MakeBox(BoxHalfSize),
		Params
	);

	DrawDebugBox(GetWorld(), TraceEnd, BoxHalfSize, FQuat(Orientation), FColor::Green, false, 1.0f);

	if (bHit)
	{
		float ObstacleHeight = Hit.ImpactPoint.Z - GetActorLocation().Z;

		UE_LOG(LogTemp, Warning, TEXT("Wall detected. Height: %f"), ObstacleHeight);

		if (ObstacleHeight > 0.f && ObstacleHeight <= ParkourMaxHeight)
		{
			FVector Direction = (Hit.ImpactNormal * -1.f + FVector::UpVector).GetSafeNormal();
			FVector VaultVelocity = Direction * ParkourVaultForwardForce;
			VaultVelocity.Z += ParkourVaultUpForce;

			LaunchCharacter(VaultVelocity, true, true);
			UE_LOG(LogTemp, Warning, TEXT(">> Parkour!!!"));
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("no wall height: %f"), ObstacleHeight);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("no wall (BoxTrace)"));
	}
}
