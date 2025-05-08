// Fill out your copyright notice in the Description page of Project Settings.

#include "CableActionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCableActionComponent::UCableActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCableActionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Owner = CastChecked<ACharacter>(GetOwner());
	ResetCable();
}

void UCableActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsCableAttached)
	{
		CalculateCableSwing();
	}
}

bool UCableActionComponent::FindCableAttachPoint(FVector& OutLocation, AActor*& OutHitActor)
{
	APlayerController* PC = CastChecked<APlayerController>(Owner->GetController());

	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
    FVector WorldLocation, WorldDirection;
	if (PC->DeprojectScreenPositionToWorld(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f, WorldLocation, WorldDirection))
    {
        FVector Start = WorldLocation;
        FVector End = Start + WorldDirection * CableMaxLength;
        
        DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 1.0f);
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Owner);
        
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

void UCableActionComponent::AttachCable()
{
	if (!Owner->GetCharacterMovement()->IsFalling())
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
		OnCableAttachedAction.Broadcast();
	}
}

void UCableActionComponent::DetachCable()
{
	if (!bIsCableAttached)
		return;
	
	ApplyDetachDrivingForce();

	FTimerHandle DetachTimerHandle;
	Owner->GetWorldTimerManager().SetTimer(DetachTimerHandle, this, &UCableActionComponent::ResetCable, 0.01f, false);

	OnCableDetachedAction.Broadcast();
}

void UCableActionComponent::ApplyDetachDrivingForce()
{
	APlayerController* PC = CastChecked<APlayerController>(Owner->GetController());
	
	bool bIsWPressed = PC->IsInputKeyDown(EKeys::W);
	bool bIsAPressed = PC->IsInputKeyDown(EKeys::A);
	bool bIsSPressed = PC->IsInputKeyDown(EKeys::S);
	bool bIsDPressed = PC->IsInputKeyDown(EKeys::D);
	bool bIsSpacePressed = PC->IsInputKeyDown(EKeys::SpaceBar);

	if (!bIsWPressed && !bIsAPressed && !bIsSPressed && !bIsDPressed && !bIsSpacePressed)
		return;

	FVector CableDirection = (CableAttachPoint - Owner->GetActorLocation()).GetSafeNormal();
	FVector ForwardDir = Owner->GetActorForwardVector();
	FVector RightDir = Owner->GetActorRightVector();
    
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
    
	FVector Force = InputTangential * CableDrivingForce;
	// FVector Force = InputTangential * 1e7;
    
	Owner->GetCharacterMovement()->AddForce(Force);
	UE_LOG(LogTemp, Log, TEXT("[%s] ControlForce: %s"), *FString(__FUNCTION__), *Force.ToString());
}

void UCableActionComponent::SetCable(const FVector& AttachLocation, AActor* HitActor)
{
	CableAttachPoint = AttachLocation;
	CurrentCableLength = FVector::Distance(Owner->GetActorLocation(), AttachLocation);
	bIsCableAttached = true;
	TargetCable->CableLength = CurrentCableLength;
	TargetCable->SetAttachEndTo(HitActor, NAME_None);
	TargetCable->SetVisibility(true);
}

void UCableActionComponent::ResetCable()
{
	CableAttachPoint = FVector::ZeroVector;
	CurrentCableLength = 0;
	bIsCableAttached = false;
	TargetCable->CableLength = 0;
	TargetCable->SetAttachEndToComponent(nullptr);
	TargetCable->SetVisibility(false);
}

void UCableActionComponent::CalculateCableSwing()
{
	FVector CharacterLocation = Owner->GetActorLocation();
	FVector ToSwingPoint = CableAttachPoint - CharacterLocation;
	float CurrentDistance = ToSwingPoint.Size();
	
	if (FMath::Abs(CurrentDistance - CurrentCableLength) > 0.1f)
	{
		FVector CableDirection = ToSwingPoint.GetSafeNormal();
		
		// circumference of a circle
		FVector CorrectPosition = CableAttachPoint - (CableDirection * CurrentCableLength);
		FVector CurrentVelocity = Owner->GetVelocity();
		Owner->SetActorLocation(CorrectPosition);
		
		// 케이블 방향의 속도 성분 제거 (원의 접선 방향 속도만 유지)
		FVector UpdatedDirection = (CableAttachPoint - CorrectPosition).GetSafeNormal();
		float RadialVelocity = FVector::DotProduct(CurrentVelocity, UpdatedDirection);
		FVector TangentialVelocity = CurrentVelocity - (UpdatedDirection * RadialVelocity);
		
		// 원심력에 의한 추가 접선 속도 (스윙감 향상)
		FVector GravityInfluence = FVector(0, 0, GetWorld()->GetGravityZ());
		FVector SwingAcceleration = FVector::CrossProduct(UpdatedDirection, FVector::CrossProduct(GravityInfluence, UpdatedDirection));
		
		// 새 속도 설정 (접선 방향만)
		Owner->GetCharacterMovement()->Velocity = TangentialVelocity + (SwingAcceleration * 0.016f); // 약 1프레임 가속도
	}
}