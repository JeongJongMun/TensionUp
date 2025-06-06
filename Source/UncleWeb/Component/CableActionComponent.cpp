// Fill out your copyright notice in the Description page of Project Settings.

#include "CableActionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UncleWeb/Character/TUCharacterPlayer.h"

UCableActionComponent::UCableActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCableActionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Owner = CastChecked<ATUCharacterPlayer>(GetOwner());
	CachedPC = CastChecked<APlayerController>(Owner->GetController());
	ResetCable();
}

void UCableActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsCableAttached)
	{
		OnCableAttaching(DeltaTime);
	}
}
bool UCableActionComponent::IsCanAttachCable(FHitResult &HitResult)
{
	int32 ViewportSizeX, ViewportSizeY;
	CachedPC->GetViewportSize(ViewportSizeX, ViewportSizeY);
	FVector WorldLocation, WorldDirection;
	if (CachedPC->DeprojectScreenPositionToWorld(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f, WorldLocation, WorldDirection))
	{
		FVector Start = WorldLocation;
		FVector End = Start + WorldDirection * CableMaxLength;
		
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);
        
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
		return bHit;
	}

	return false;
}

bool UCableActionComponent::IsCanAttachCable()
{
	FHitResult HitResult;
	return IsCanAttachCable(HitResult);
}

void UCableActionComponent::TryAttachCable()
{
	if (!Owner->GetCharacterMovement()->IsFalling())
		return;
	
	TryDetachCable();
	
	FHitResult HitResult;
	if (IsCanAttachCable(HitResult))
	{
		SetCable(HitResult.ImpactPoint, HitResult.GetActor());
		
		OnCableAttachedAction.Broadcast();
	}
}

void UCableActionComponent::TryDetachCable()
{
	if (!bIsCableAttached)
		return;
	
	OnDrivingForce();
	ResetCable();

	OnCableDetachedAction.Broadcast();
}

void UCableActionComponent::OnDrivingForce()
{
	APlayerController* PC = CastChecked<APlayerController>(Owner->GetController());
	
	bool bIsWPressed = PC->IsInputKeyDown(EKeys::W);
	bool bIsAPressed = PC->IsInputKeyDown(EKeys::A);
	bool bIsSPressed = PC->IsInputKeyDown(EKeys::S);
	bool bIsDPressed = PC->IsInputKeyDown(EKeys::D);
	bool bIsSpacePressed = PC->IsInputKeyDown(EKeys::SpaceBar);
	
	if (!bIsSpacePressed || (!bIsWPressed && !bIsAPressed && !bIsSPressed && !bIsDPressed))
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
	
	float InputRadialComponent = FVector::DotProduct(InputDirection, CableDirection);
	FVector InputTangential = InputDirection - (CableDirection * InputRadialComponent);
	InputTangential.Normalize();

	FVector Force = InputTangential * CableDrivingForce;
    
	Owner->LaunchCharacter(Force, true, true);
}

void UCableActionComponent::SetCable(const FVector& AttachLocation, AActor* HitActor)
{
	CableAttachPoint = AttachLocation;

	float MaxPossibleLength = FVector::Distance(Owner->GetActorLocation(), AttachLocation);
	float InitialLength = FMath::Clamp(MaxPossibleLength * InitialCableLengthRatio, CableMinLength, CableMaxLength);
	FVector LocalAttachLocation = HitActor->GetTransform().InverseTransformPosition(AttachLocation);

	CurrentCableLength = InitialLength;
	TargetCableLength = InitialLength;

	bIsCableAttached = true;
	TargetCable->CableLength = CurrentCableLength;
	TargetCable->SetAttachEndTo(HitActor, NAME_None);
	TargetCable->EndLocation = LocalAttachLocation;
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

void UCableActionComponent::OnCableAttaching(const float DeltaTime)
{
	if (!FMath::IsNearlyEqual(CurrentCableLength, TargetCableLength, 1.0f))
	{
		OnCableLengthAdjust(DeltaTime);
	}
	
	const FVector CharacterLocation = Owner->GetActorLocation();
	const FVector ToSwingPoint = CableAttachPoint - CharacterLocation;
	const float CurrentDistance = ToSwingPoint.Size();
	
	if (FMath::Abs(CurrentDistance - CurrentCableLength) > 0.1f)
	{
		// Set Position
		const FVector CableDirection = ToSwingPoint.GetSafeNormal();
		const FVector CorrectPosition = CableAttachPoint - (CableDirection * CurrentCableLength);
		const FVector NewPosition = FMath::VInterpTo(CharacterLocation, CorrectPosition, DeltaTime, CableInterpSpeed);
		Owner->SetActorLocation(NewPosition);
		
		// Remove Radial Velocity, Keep Tangential Velocity
		const FVector UpdatedDirection = (CableAttachPoint - CorrectPosition).GetSafeNormal();
		const FVector CurrentVelocity = Owner->GetVelocity();
		const float RadialVelocity = FVector::DotProduct(CurrentVelocity, UpdatedDirection);
		const FVector TangentialVelocity = CurrentVelocity - (UpdatedDirection * RadialVelocity);
		
		// Pendulum movement
		const FVector GravityInfluence = FVector(0, 0, GetWorld()->GetGravityZ());
		const FVector SwingAcceleration = FVector::CrossProduct(UpdatedDirection, FVector::CrossProduct(GravityInfluence, UpdatedDirection));
		FVector NewVelocity = TangentialVelocity + (SwingAcceleration * DeltaTime);

		// Steam Booster
		if (bIsSteamBoosterActive)
		{
			const FVector SteamBoosterDirection = Owner->GetActorForwardVector();
			NewVelocity += SteamBoosterDirection.GetSafeNormal() * SteamBoosterForce * DeltaTime;
			
			OnSteamBoosterActive.Broadcast();
		}
		
		Owner->GetCharacterMovement()->Velocity = NewVelocity;
	}

	OnCableAttachingAction.Broadcast();
}

void UCableActionComponent::OnCableLengthAdjust(const float DeltaTime)
{
	CurrentCableLength = FMath::FInterpTo(CurrentCableLength, TargetCableLength, DeltaTime, CableInterpSpeed);
	TargetCable->CableLength = CurrentCableLength;
}

void UCableActionComponent::ShortenCable()
{
	if (!bIsCableAttached) return;

	TargetCableLength = FMath::Clamp(TargetCableLength - CableAdjustSpeed, CableMinLength, CableMaxLength);
}

void UCableActionComponent::ExtendCable()
{
	if (!bIsCableAttached) return;

	TargetCableLength = FMath::Clamp(TargetCableLength + CableAdjustSpeed, CableMinLength, CableMaxLength);
}
