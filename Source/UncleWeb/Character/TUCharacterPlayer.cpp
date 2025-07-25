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
#include "UncleWeb/Component/DynamicCamera.h"
#include "UncleWeb/Component/ParkourComponent.h"
#include "DrawDebugHelpers.h"
#include "UncleWeb/UI/UIManager.h"
#include "UncleWeb/Util/TUDefines.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "UncleWeb/Actor/SteamCharger.h"

ATUCharacterPlayer::ATUCharacterPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// Component
	NiagaraCompSteamVfxL = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp_L"));
	NiagaraCompSteamVfxR = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp_R"));
	NiagaraCompSteamVfxL->SetupAttachment(GetMesh(), SteamEffectSocketNameL);
	NiagaraCompSteamVfxR->SetupAttachment(GetMesh(), SteamEffectSocketNameR);
	NiagaraCompSteamVfxL->SetAutoActivate(false);
	NiagaraCompSteamVfxR->SetAutoActivate(false);
	
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> NiagaraSystemObj(TEXT("/Script/Niagara.NiagaraSystem'/Game/VFX/CartoonSmoke/Effects/NS_ToonSmoke4.NS_ToonSmoke4'"));
	if (NiagaraSystemObj.Succeeded())
	{
		NiagaraSystemSteamVfx = NiagaraSystemObj.Object;
		NiagaraCompSteamVfxL->SetAsset(NiagaraSystemSteamVfx);
		NiagaraCompSteamVfxR->SetAsset(NiagaraSystemSteamVfx);
	}

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	DynamicCameraComponent = CreateDefaultSubobject<UDynamicCamera>(TEXT("DynamicCameraComponent"));
	DynamicCameraComponent->TargetSpringArm = CameraBoom;
	DynamicCameraComponent->FollowCamera = FollowCamera;
	DynamicCameraComponent->InitializeCamera();
 
	CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("CableComponent"));
	CableComponent->SetupAttachment(GetMesh(), CableSocketName);
	
	CableActionComponent = CreateDefaultSubobject<UCableActionComponent>(TEXT("CableActionComponent"));
	CableActionComponent->TargetCable = CableComponent;

	SteamComponent = CreateDefaultSubobject<USteamComponent>(TEXT("SteamComponent"));
	ParkourComponent = CreateDefaultSubobject<UParkourComponent>(TEXT("ParkourComponent"));

	// Input
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Input/IMC_Default.IMC_Default'"));
	if (nullptr != InputMappingContextRef.Object)
	{
		DefaultMappingContext = InputMappingContextRef.Object;
	}
	InitializeInputActions();

	InitializeAnimations();
}

bool ATUCharacterPlayer::IsCableAttached() const
{
	return CableActionComponent && CableActionComponent->IsCableAttaching();
}

void ATUCharacterPlayer::PlayAnimation(const EPlayerAnimationMontageType AnimType, float InPlayRate, const FName SectionName)
{
	if (const TObjectPtr<UAnimMontage> AnimMontage = AnimMontageMap.FindRef(AnimType))
	{
		if (SectionName == NAME_None)
		{
			TArray<FName>& SectionNames = AnimSectionMap.FindOrAdd(AnimType);
			if (SectionNames.IsEmpty())
			{
				for (const auto& Section : AnimMontage->CompositeSections)
				{
					SectionNames.Add(Section.SectionName);
				}
			}

			if (SectionNames.Num() > 0)
			{
				const FName RandomSection = SectionNames[FMath::RandRange(0, SectionNames.Num() - 1)];
				
				PlayAnimMontage(AnimMontage, InPlayRate, RandomSection);
			}
		}
		else
		{
			PlayAnimMontage(AnimMontage, InPlayRate, SectionName);
		}
	}
}

void ATUCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	ParkourComponent->Initialize(this);

	APlayerController* PC = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
		//Subsystem->RemoveMappingContext(DefaultMappingContext);
	}

	UIManager = GetWorld()->SpawnActor<AUIManager>(AUIManager::StaticClass());
	UIManager->InitializeUI(PC);

	SteamComponent->OnSteamChanged.AddDynamic(this, &ATUCharacterPlayer::HandleUpdateSteamUI);
	CableActionComponent->OnBeginLaunchAction.AddDynamic(this, &ATUCharacterPlayer::OnBeginLaunch);
	CableActionComponent->OnBeginRetractAction.AddDynamic(this, &ATUCharacterPlayer::OnBeginRetract);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ATUCharacterPlayer::OnOverlapBegin);
	// repulsive force
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ATUCharacterPlayer::OnHit);

	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			SoundManager = GI->GetSubsystem<USoundManager>();
		}
	}
}

void ATUCharacterPlayer::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA<ASteamCharger>())
	{
		HandleSteamCharagerCollision();
	}
}

void ATUCharacterPlayer::HandleSteamCharagerCollision()
{
	SteamComponent->SetSteam(100.0f);
}

void ATUCharacterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CableActionComponent->IsCanLaunchCable())
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

	ParkourComponent->TickParkour(DeltaTime);

}

void ATUCharacterPlayer::InitializeInputActions()
{
	InputActionPaths = {
		{ EInputActionType::Move, TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Move.IA_Move'"))) },
		{ EInputActionType::Jump, TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Jump.IA_Jump'"))) },
		{ EInputActionType::Look, TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Look.IA_Look'"))) },
		{ EInputActionType::LeftClick, TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_LeftClick.IA_LeftClick'"))) },
		{ EInputActionType::Dash, TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Dash.IA_Dash'"))) },
		{ EInputActionType::Run, TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Run.IA_Run'"))) },
		{ EInputActionType::ShortenCable, TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_ZoomIn.IA_ZoomIn'"))) },
		{ EInputActionType::ExtendCable, TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_ZoomOut.IA_ZoomOut'"))) },
		{ EInputActionType::SteamBoost, TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_SteamBooster.IA_SteamBooster'"))) }
	};

	InputActionMap = {
		{ EInputActionType::Move, MoveAction},
		{ EInputActionType::Jump, JumpAction },
		{ EInputActionType::Look, LookAction },
		{ EInputActionType::LeftClick, LeftClickAction },
		{ EInputActionType::Dash, DashAction },
		{ EInputActionType::Run, RunAction },
		{ EInputActionType::ShortenCable, ShortenCableAction },
		{ EInputActionType::ExtendCable, ExtendCableAction },
		{ EInputActionType::SteamBoost, SteamBoostAction }
	};

	for (const TPair<EInputActionType, TSoftObjectPtr<UInputAction>>& Pair : InputActionPaths)
	{
		if (Pair.Value.IsValid())
		{
			if (UInputAction* LoadedAction = Pair.Value.LoadSynchronous())
			{
				InputActionMap.Add(Pair.Key, LoadedAction);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to load InputAction: %s"), CURRENT_CONTEXT, *Pair.Value.ToString());
			}
		}
	}

	MoveAction = InputActionMap.FindRef(EInputActionType::Move);
	JumpAction = InputActionMap.FindRef(EInputActionType::Jump);
	LookAction = InputActionMap.FindRef(EInputActionType::Look);
	LeftClickAction = InputActionMap.FindRef(EInputActionType::LeftClick);
	DashAction = InputActionMap.FindRef(EInputActionType::Dash);
	ShortenCableAction = InputActionMap.FindRef(EInputActionType::ShortenCable);
	ExtendCableAction = InputActionMap.FindRef(EInputActionType::ExtendCable);
	RunAction = InputActionMap.FindRef(EInputActionType::Run);
	SteamBoostAction = InputActionMap.FindRef(EInputActionType::SteamBoost);
}

void ATUCharacterPlayer::InitializeAnimations()
{
	TMap<EPlayerAnimationMontageType, FString> AnimMontagePaths = {
		{ EPlayerAnimationMontageType::Swinging, TEXT("/Script/Engine.AnimMontage'/Game/Animations/MT_Player_Swing.MT_Player_Swing'") },
		{ EPlayerAnimationMontageType::SwingEnd, TEXT("/Script/Engine.AnimMontage'/Game/Animations/MT_Player_SwingEnd.MT_Player_SwingEnd'") }
	};

	for (const auto& [Type, Path] : AnimMontagePaths)
	{
		if (ConstructorHelpers::FObjectFinder<UAnimMontage> MontageFinder(*Path); MontageFinder.Succeeded())
		{
			AnimMontageMap.Add(Type, MontageFinder.Object);
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
	EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::HandleLaunchCable);
	EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Completed, this, &ATUCharacterPlayer::HandleRetractCable);
	EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Canceled, this, &ATUCharacterPlayer::HandleRetractCable);

	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::Dash);
	// Deprecated : Cable Length Adjust Func
	//EnhancedInputComponent->BindAction(ShortenCableAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::HandleShortenCable);
	//EnhancedInputComponent->BindAction(ExtendCableAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::HandleExtendCable);
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

	bIsDashing = true;

	if (SteamComponent)
	{
		FRotator CameraRotation = FollowCamera->GetComponentRotation();
		FRotator YawRotation(0, CameraRotation.Yaw, 0);
		FVector DashDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		//FVector DashDirection = GetActorForwardVector(); // player forward direction
		LaunchCharacter(DashDirection * DashStrength*1.0f, true, true);

		SteamComponent->ConsumeSteam(DashSteamCost);

		// deactive dash effect
		FTimerHandle DashEffectHandle;
		GetWorldTimerManager().SetTimer(DashEffectHandle, [this]()
			{
				bIsDashing = false;
			}, 0.5f, false);
	}
}
bool ATUCharacterPlayer::IsDashing() const
{
	return bIsDashing;
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
void ATUCharacterPlayer::HandleLaunchCable()
{
	if (!SteamComponent->HasEnoughSteam(CableSteamCost))
		return;
	
	CableActionComponent->TryLaunchCable();
}

void ATUCharacterPlayer::HandleRetractCable()
{
	CableActionComponent->TryRetractCable();
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

void ATUCharacterPlayer::OnBeginLaunch()
{
	FTimerHandle TimerHandle;
	Owner->GetWorldTimerManager().SetTimer(TimerHandle, [this]()
		{
			GetCharacterMovement()->AirControl = CableActionAirControl;
		}, MovementModeTransitionDelay, false);

	SteamComponent->ConsumeSteam(CableSteamCost);
	PlayAnimation(EPlayerAnimationMontageType::Swinging);
	RotateToCameraDirection();

	GetCharacterMovement()->FallingLateralFriction = CableFallingLateralFriction;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void ATUCharacterPlayer::OnBeginRetract()
{
	GetCharacterMovement()->AirControl = AirControl;
	GetCharacterMovement()->FallingLateralFriction = FallingLateralFriction;
	HandleStopSteamBooster();
	PlayAnimation(EPlayerAnimationMontageType::SwingEnd);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
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

void ATUCharacterPlayer::HandleJumpOrParkour()
{
	if (GetCharacterMovement()->IsFalling())
	{
		ParkourComponent->TryParkour();
	}
	else
	{
		Jump();
	}
}

void ATUCharacterPlayer::HandleStartSteamBooster()
{
	if (!CableActionComponent->IsCableActive() || !SteamComponent->HasEnoughSteam(SteamBoosterCost))
		return;

	CableActionComponent->SetIsSteamBoosterActive(true);

	if (NiagaraCompSteamVfxL && NiagaraCompSteamVfxR)
	{
		NiagaraCompSteamVfxL->Activate();
		NiagaraCompSteamVfxR->Activate();
	}

	SoundManager->PlaySound2D("SW_Cable_SteamBoost", 1.0f, 1.0f, true);
}

void ATUCharacterPlayer::HandleStopSteamBooster()
{
	CableActionComponent->SetIsSteamBoosterActive(false);	

	if (NiagaraCompSteamVfxL && NiagaraCompSteamVfxR)
	{
		NiagaraCompSteamVfxL->Deactivate();
		NiagaraCompSteamVfxR->Deactivate();
	}
	
	SoundManager->StopSound("SW_Cable_SteamBoost");
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