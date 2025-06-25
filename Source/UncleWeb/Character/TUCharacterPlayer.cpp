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
#include "Particles/ParticleSystemComponent.h"
#include "UncleWeb/UI/UIManager.h"
#include "UncleWeb/Util/TUDefines.h"
#include "Kismet/KismetMathLibrary.h"

ATUCharacterPlayer::ATUCharacterPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	FootSteamEffectL = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FootSteamEffect_L"));
	FootSteamEffectR = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FootSteamEffect_R"));
	FootSteamEffectL->SetupAttachment(GetMesh(), SteamEffectSocketNameL);
	FootSteamEffectR->SetupAttachment(GetMesh(), SteamEffectSocketNameR);
	FootSteamEffectL->SetAutoActivate(false);
	FootSteamEffectR->SetAutoActivate(false);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> SteamParticleObj(TEXT("/Script/Engine.ParticleSystem'/Game/VFX/P_Flamethrower.P_Flamethrower'"));
	if (SteamParticleObj.Succeeded())
	{
		SteamParticleSystem = SteamParticleObj.Object;
		FootSteamEffectL->SetTemplate(SteamParticleSystem);
		FootSteamEffectR->SetTemplate(SteamParticleSystem);
	}

	// Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	DynamicCameraComponent = CreateDefaultSubobject<UTUDynamicCamera>(TEXT("DynamicCameraComponent"));
	DynamicCameraComponent->TargetSpringArm = CameraBoom;
	DynamicCameraComponent->InitializeCamera();

	CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("CableComponent"));
	CableComponent->SetupAttachment(GetMesh(), CableSocketName);
	
	CableActionComponent = CreateDefaultSubobject<UCableActionComponent>(TEXT("CableActionComponent"));
	CableActionComponent->TargetCable = CableComponent;

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

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionSteamBoostRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_SteamBooster.IA_SteamBooster'"));
	if (InputActionSteamBoostRef.Succeeded())
	{
		SteamBoostAction = InputActionSteamBoostRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> SwingAnimMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/Animations/MT_Player_Swing.MT_Player_Swing'"));
	if (SwingAnimMontageRef.Succeeded())
	{
		MontageSwing = SwingAnimMontageRef.Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to load MontageSwing animation"), *GetName());
	}
}

bool ATUCharacterPlayer::IsCableAttached() const
{
	return CableActionComponent && CableActionComponent->IsCableAttaching();
}

void ATUCharacterPlayer::PlayAnimation(const FName SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance || !MontageSwing)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] AnimInstance or AnimMontage is null"), CURRENT_CONTEXT);
		return;
	}

	if (AnimInstance->Montage_IsPlaying(MontageSwing))
	{
		FName CurrentSection = AnimInstance->Montage_GetCurrentSection(MontageSwing);
		if (CurrentSection != NAME_None && CurrentSection != SectionName)
		{
			AnimInstance->Montage_Stop(0.5f, MontageSwing);
			AnimInstance->Montage_Play(MontageSwing);
			AnimInstance->Montage_JumpToSection(SectionName, MontageSwing); 
		}
	}
	else
	{
		AnimInstance->Montage_Play(MontageSwing);
		AnimInstance->Montage_JumpToSection(SectionName, MontageSwing); 
	}
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

	if (CableActionComponent->IsSteamBoosterActive())
	{
		SteamComponent->ConsumeSteam(SteamBoosterCost * DeltaTime);

		if (!SteamComponent->HasEnoughSteam(SteamBoosterCost))
		{
			HandleStopSteamBooster();
		}
	}

	if (ParkourState == EParkourState::Climbing)
	{
		ClimbInterpAlpha += DeltaTime * ClimbSpeed;
		FVector NewLocation = FMath::Lerp(ParkourStartLocation, ParkourTargetLocation, ClimbInterpAlpha);
		SetActorLocation(NewLocation);

		if (ClimbInterpAlpha >= 1.0f)
		{
			ParkourState = EParkourState::ClimbFinished;
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
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
	// Temporary Disabled
	// EnhancedInputComponent->BindAction(ShortenCableAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::HandleShortenCable);
	// EnhancedInputComponent->BindAction(ExtendCableAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::HandleExtendCable);
	EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::StartRunning);
	EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ATUCharacterPlayer::StopRunning);
	//for parkour
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::HandleJumpPressed);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATUCharacterPlayer::HandleJumpReleased);

	EnhancedInputComponent->BindAction(SteamBoostAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::HandleStartSteamBooster);
	EnhancedInputComponent->BindAction(SteamBoostAction, ETriggerEvent::Completed, this, &ATUCharacterPlayer::HandleStopSteamBooster);
	EnhancedInputComponent->BindAction(SteamBoostAction, ETriggerEvent::Canceled, this, &ATUCharacterPlayer::HandleStopSteamBooster);
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
	if (bIsTryingParkour) return;

	UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(GetRootComponent());
	FVector CurrentVelocity = RootComp->GetPhysicsLinearVelocity();
	float CurrentSpeed = CurrentVelocity.Size();

	// if IncidenceAngle close to 1, it means the character is hitting the wall almost perpendicular
	float IncidenceAngle = FVector::DotProduct(CurrentVelocity.GetSafeNormal(), -Hit.ImpactNormal);
	bool bIsFalling = GetCharacterMovement()->IsFalling();

	if (bIsJumpInputActive || bIsTryingParkour || CurrentSpeed < 500.0f || !bIsFalling || IncidenceAngle < 0.5f)
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
	CableActionComponent->TryAttachCable();
}

void ATUCharacterPlayer::HandleDetachCable()
{
	CableActionComponent->TryDetachCable();
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
	PlayAnimation(TEXT("Default"));
	RotateToCameraDirection();
}

void ATUCharacterPlayer::OnCableDetached()
{
	GetCharacterMovement()->AirControl = JumpAirControl;
	HandleStopSteamBooster();
}

void ATUCharacterPlayer::HandleJumpPressed()
{
	bIsJumpInputActive = true;
	HandleJumpOrParkour();
}

void ATUCharacterPlayer::HandleJumpReleased()
{
	bIsJumpInputActive = false;
	StopJumping();
}

void ATUCharacterPlayer::HandleStartSteamBooster()
{
	if (!CableActionComponent->IsCableAttaching() || !SteamComponent->HasEnoughSteam(SteamBoosterCost))
		return;

	CableActionComponent->SetIsSteamBoosterActive(true);
	if (FootSteamEffectL && FootSteamEffectR)
	{
		FootSteamEffectL->Activate();
		FootSteamEffectR->Activate();
	}
}

void ATUCharacterPlayer::HandleStopSteamBooster()
{
	CableActionComponent->SetIsSteamBoosterActive(false);	
	if (FootSteamEffectL && FootSteamEffectR)
	{
		FootSteamEffectL->Deactivate();
		FootSteamEffectR->Deactivate();
	}
}

void ATUCharacterPlayer::RotateToCameraDirection()
{
	if (const TObjectPtr<APlayerController> PC = Cast<APlayerController>(GetController()))
	{
		const FRotator CameraRotation = PC->GetControlRotation();
		const FRotator TargetRotation = FRotator(0.0f, CameraRotation.Yaw, 0.0f);
        
		SetActorRotation(TargetRotation);
	}
}
void ATUCharacterPlayer::TryParkour()
{
	if (!GetCharacterMovement()->IsFalling()) return;

	FVector TraceStart = GetActorLocation() + FVector(0, 0, 50.f);
	FVector BoxHalfSize = FVector(30.f, 50.f, 40.f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	TArray<FVector> Directions = {
		GetActorForwardVector(),
		GetActorForwardVector().RotateAngleAxis(30.f, FVector::UpVector),
		GetActorForwardVector().RotateAngleAxis(-30.f, FVector::UpVector)
	};

	FHitResult BestHit;
	bool bAnyHit = false;
	float ClosestDistance = TNumericLimits<float>::Max();

	for (const FVector& Dir : Directions)
	{
		FVector TraceEnd = TraceStart + Dir * ParkourMaxDistance;
		FRotator Orientation = Dir.Rotation();
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

		DrawDebugBox(GetWorld(), TraceEnd, BoxHalfSize, FQuat(Orientation), bHit ? FColor::Green : FColor::Red, false, 1.f);

		if (bHit)
		{
			float Distance = FVector::Dist(GetActorLocation(), Hit.ImpactPoint);
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				BestHit = Hit;
				bAnyHit = true;
			}
		}
	}

	if (bAnyHit)
	{
		const FVector SurfaceNormal = BestHit.ImpactNormal;
		FVector AttachPoint = BestHit.ImpactPoint + SurfaceNormal * 30.f;
		FVector TargetTop = AttachPoint + FVector(0, 0, 120.f);
		float EyeHeight = GetActorLocation().Z + BaseEyeHeight;

		if (TargetTop.Z > EyeHeight + 100.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Too High - Cannot climb"));
			return;
		}

		ParkourStartLocation = GetActorLocation();
		ParkourTargetLocation = TargetTop;
		ClimbInterpAlpha = 0.f;
		ParkourState = EParkourState::Climbing;

		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		FRotator LookAtRotation = UKismetMathLibrary::MakeRotFromX(-SurfaceNormal);
		SetActorRotation(LookAtRotation);
		UE_LOG(LogTemp, Log, TEXT("Start Climbing Parkour"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No climbable surface found"));
	}
	bIsTryingParkour = false;
}

