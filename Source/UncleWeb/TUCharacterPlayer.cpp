// Fill out your copyright notice in the Description page of Project Settings.

#include "TUCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "CableComponent.h"
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
	
	// Cable
	CableMaxLength = 2000.0f;
	CableDrivingForce = 1000000.0f;
	CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("CableComponent"));
	CableComponent->SetupAttachment(GetRootComponent(), "Swing");
	CableComponent->NumSegments = 1;
	CableComponent->CableWidth = 2.0f;
	CableComponent->bAttachEnd = true;
	
	// Widget
	static ConstructorHelpers::FClassFinder<UUserWidget>HUD(TEXT("WidgetBlueprint'/Game/UI/WBP_Crosshair.WBP_Crosshair_C'"));
	if (HUD.Succeeded())
	{
		HUDClass = HUD.Class;
	}
}

void ATUCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	ResetCable();
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
}

void ATUCharacterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsCableAttached)
	{
		CalculateCableSwing();
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
	EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Started, this, &ATUCharacterPlayer::AttachCable);
	EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Completed, this, &ATUCharacterPlayer::DetachCable);
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

bool ATUCharacterPlayer::FindCableAttachPoint(FVector& OutLocation, AActor*& OutHitActor)
{
    APlayerController* PC = CastChecked<APlayerController>(GetController());

	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
    FVector WorldLocation, WorldDirection;
	if (PC->DeprojectScreenPositionToWorld(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f, WorldLocation, WorldDirection))
    {
        // 레이캐스트 매개변수 설정
        FVector Start = WorldLocation;
        FVector End = Start + WorldDirection * CableMaxLength;
        
        DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 1.0f);
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this); // 플레이어 자신은 무시
        
        FHitResult HitResult;
    	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
        if (bHit)
        {
            DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Green, false, 2.0f);

            OutLocation = HitResult.ImpactPoint;
        	OutHitActor = HitResult.GetActor();
            return true;
        }
    }

    return false;
}

void ATUCharacterPlayer::AttachCable()
{
	if (!GetCharacterMovement()->IsFalling())
		return;
	
    if (bIsCableAttached)
	{
		DetachCable();
		return;
	}
    
	FVector AttachLocation;
	AActor* HitActor = nullptr;
	if (FindCableAttachPoint(AttachLocation, HitActor))
	{
		SetCable(AttachLocation, HitActor);
	}
}

void ATUCharacterPlayer::DetachCable()
{
	if (!bIsCableAttached)
		return;
	
	ApplyDetachDrivingForce();

	FTimerHandle DetachTimerHandle;
	GetWorldTimerManager().SetTimer(DetachTimerHandle, this, &ATUCharacterPlayer::ResetCable, 0.01f, false);
}

void ATUCharacterPlayer::ApplyDetachDrivingForce()
{
	APlayerController* PC = CastChecked<APlayerController>(GetController());
	
	bool bIsWPressed = PC->IsInputKeyDown(EKeys::W);
	bool bIsAPressed = PC->IsInputKeyDown(EKeys::A);
	bool bIsSPressed = PC->IsInputKeyDown(EKeys::S);
	bool bIsDPressed = PC->IsInputKeyDown(EKeys::D);
	bool bIsSpacePressed = PC->IsInputKeyDown(EKeys::SpaceBar);

	if (!bIsWPressed && !bIsAPressed && !bIsSPressed && !bIsDPressed && !bIsSpacePressed)
		return;

	FVector CableDirection = (CableAttachPoint - GetActorLocation()).GetSafeNormal();
	FVector ForwardDir = GetActorForwardVector();
	FVector RightDir = GetActorRightVector();
    
	FVector InputDirection = FVector::ZeroVector;
	if (bIsWPressed) InputDirection += ForwardDir;
	if (bIsAPressed) InputDirection -= RightDir;
	if (bIsSPressed) InputDirection -= ForwardDir;
	if (bIsDPressed) InputDirection += RightDir;
	InputDirection.Normalize();
    
	// 입력 방향의 접선 성분 계산
	float InputRadialComponent = FVector::DotProduct(InputDirection, CableDirection);
	FVector InputTangential = InputDirection - (CableDirection * InputRadialComponent);
	InputTangential.Normalize();
    
	// FVector Force = InputTangential * CableDrivingForce;
	FVector Force = InputTangential * 1e7;
    
	GetCharacterMovement()->AddForce(Force);
	UE_LOG(LogTemp, Log, TEXT("[%s] ControlForce: %s"), *FString(__FUNCTION__), *Force.ToString());
}

void ATUCharacterPlayer::SetCable(const FVector& AttachLocation, AActor* HitActor)
{
	CableAttachPoint = AttachLocation;
	CurrentCableLength = FVector::Distance(GetActorLocation(), AttachLocation);
	bIsCableAttached = true;
	CableComponent->CableLength = CurrentCableLength;
	CableComponent->SetAttachEndTo(HitActor, NAME_None);
	CableComponent->SetVisibility(true);
}

void ATUCharacterPlayer::ResetCable()
{
	CableAttachPoint = FVector::ZeroVector;
	CurrentCableLength = 0;
	bIsCableAttached = false;
	CableComponent->CableLength = 0;
	CableComponent->SetAttachEndToComponent(nullptr);
	CableComponent->SetVisibility(false);
}

void ATUCharacterPlayer::CalculateCableSwing()
{
	FVector CharacterLocation = GetActorLocation();
	FVector ToSwingPoint = CableAttachPoint - CharacterLocation;
	float CurrentDistance = ToSwingPoint.Size();
	
	if (FMath::Abs(CurrentDistance - CurrentCableLength) > 0.1f)
	{
		FVector CableDirection = ToSwingPoint.GetSafeNormal();
		
		// circumference of a circle
		FVector CorrectPosition = CableAttachPoint - (CableDirection * CurrentCableLength);
		FVector CurrentVelocity = GetVelocity();
		SetActorLocation(CorrectPosition);
		
		// 케이블 방향의 속도 성분 제거 (원의 접선 방향 속도만 유지)
		FVector UpdatedDirection = (CableAttachPoint - CorrectPosition).GetSafeNormal();
		float RadialVelocity = FVector::DotProduct(CurrentVelocity, UpdatedDirection);
		FVector TangentialVelocity = CurrentVelocity - (UpdatedDirection * RadialVelocity);
		
		// 원심력에 의한 추가 접선 속도 (스윙감 향상)
		FVector GravityInfluence = FVector(0, 0, GetWorld()->GetGravityZ());
		FVector SwingAcceleration = FVector::CrossProduct(UpdatedDirection, FVector::CrossProduct(GravityInfluence, UpdatedDirection));
		
		// 새 속도 설정 (접선 방향만)
		GetCharacterMovement()->Velocity = TangentialVelocity + (SwingAcceleration * 0.016f); // 약 1프레임 가속도
	}
}
