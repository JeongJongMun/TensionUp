// Fill out your copyright notice in the Description page of Project Settings.

#include "UWCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"

AUWCharacterPlayer::AUWCharacterPlayer()
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

	CableEndLocation = FVector::ZeroVector;
	// 케이블 컴포넌트 생성
	CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("CableComponent"));
	CableComponent->SetupAttachment(GetRootComponent(), "Swing");
	CableComponent->EndLocation = FVector::ZeroVector;
	CableComponent->CableLength = 0;
	CableComponent->SetVisibility(false);
    
	// 케이블 외관 설정
	CableComponent->CableWidth = 2.0f;
	CableComponent->NumSegments = 10; // 케이블 세그먼트 수 - 부드러운 곡선을 위해
	CableComponent->bAttachEnd = true;

	// Widget
	static ConstructorHelpers::FClassFinder<UUserWidget>HUD(TEXT("WidgetBlueprint'/Game/UI/WBP_Crosshair.WBP_Crosshair_C'"));
	if (HUD.Succeeded())
	{
		HUDClass = HUD.Class;
		UE_LOG(LogTemp, Log, TEXT("HUD created"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HUD not created"));
	}
}

void AUWCharacterPlayer::BeginPlay()
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
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Widget could not be created"));
	}
}

void AUWCharacterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsCableAttached)
	{
		CalculateSwingForce();
		UpdateCableEndLocation();
	}
}

void AUWCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AUWCharacterPlayer::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AUWCharacterPlayer::StopJumping);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUWCharacterPlayer::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUWCharacterPlayer::Look);

	EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Started, this, &AUWCharacterPlayer::FireCable);
	EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Completed, this, &AUWCharacterPlayer::ReleaseCable);
}

void AUWCharacterPlayer::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void AUWCharacterPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

bool AUWCharacterPlayer::FindCableAttachPoint(FVector& OutLocation, FVector& OutNormal)
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AUWCharacterPlayer::FindCableAttachPoint] PlayerController is null"));
        return false;
    }

	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
    FVector WorldLocation, WorldDirection;
	if (PC->DeprojectScreenPositionToWorld(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f, WorldLocation, WorldDirection))
    {
        // 레이캐스트 매개변수 설정
        FVector Start = WorldLocation;
        FVector End = Start + WorldDirection * MaxCableLength;
        
        DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 1.0f);
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this); // 플레이어 자신은 무시
        
        FHitResult HitResult;
    	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
        if (bHit)
        {
            DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Green, false, 2.0f);

            OutLocation = HitResult.ImpactPoint;
            OutNormal = HitResult.ImpactNormal;
            UE_LOG(LogTemp, Log, TEXT("[AUWCharacterPlayer::FindCableAttachPoint] Cable Attach Point Found: %s"), *OutLocation.ToString());
            return true;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[AUWCharacterPlayer::FindCableAttachPoint] Cable Attach Point Not Found"));
    return false;
}

// 마우스 클릭 시 케이블 발사
void AUWCharacterPlayer::FireCable()
{
    if (bIsCableAttached)
	{
		ReleaseCable();
		return;
	}
    
	FVector AttachLocation, AttachNormal;
	if (FindCableAttachPoint(AttachLocation, AttachNormal))
	{
		bIsCableAttached = true;
		CableEndLocation = AttachLocation;
		NewCableLength = FVector::Dist(GetActorLocation(), CableEndLocation);
		CableComponent->SetVisibility(true);
	}
}

// 케이블 해제
void AUWCharacterPlayer::ReleaseCable()
{
	if (bIsCableAttached)
	{
		bIsCableAttached = false;
		CableComponent->SetVisibility(false);
	}
}

void AUWCharacterPlayer::CalculateSwingForce()
{
	FVector CharacterLocation = GetActorLocation();
	FVector ToSwingPoint = CableEndLocation - CharacterLocation;
	float CurrentDistance = ToSwingPoint.Size();

	if (CurrentDistance > NewCableLength)
	{
		FVector Direction = ToSwingPoint.GetSafeNormal();
		float Stretch = CurrentDistance - NewCableLength;
		FVector SpringForce = Direction * Stretch * CableStrength;
		FVector DampingForce = -GetVelocity() * DampingFactor;

		GetCharacterMovement()->AddForce(SpringForce + DampingForce);
	}
}

void AUWCharacterPlayer::UpdateCableEndLocation()
{
	FVector LocalEndLocation = CableComponent->GetComponentTransform().InverseTransformPosition(CableEndLocation);
	CableComponent->EndLocation = LocalEndLocation;
}
