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

	if (UMaterialInterface* CableMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Script/Engine.Material'/Game/Materials/M_Cable.M_Cable'")))
	{
		TargetCable->SetMaterial(0, CableMat);
	}
	TargetCable->NumSegments = 1;
	TargetCable->CableWidth = 4.0f;
	TargetCable->bAttachStart = true;
	TargetCable->bAttachEnd = true;
	
	Owner = CastChecked<ATUCharacterPlayer>(GetOwner());
	CachedPC = CastChecked<APlayerController>(Owner->GetController());
	ResetCable();
}

void UCableActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentCableState == ECableState::Launching || CurrentCableState == ECableState::Retracting)
	{
		PlayCableVfx(DeltaTime);
	}
	
	if (CurrentCableState == ECableState::Launching || CurrentCableState == ECableState::Attaching)
	{
		OnWhileAttach(DeltaTime);
	}
}
bool UCableActionComponent::IsCanLaunchCable(FHitResult &HitResult)
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

bool UCableActionComponent::IsCanLaunchCable()
{
	FHitResult HitResult;
	return IsCanLaunchCable(HitResult);
}

void UCableActionComponent::TryLaunchCable()
{
	if (!Owner->GetCharacterMovement()->IsFalling())
		return;

	if (CurrentCableState != ECableState::Idle)
		return;
	
	FHitResult HitResult;
	if (IsCanLaunchCable(HitResult))
	{
		LaunchCable(HitResult.ImpactPoint, HitResult.GetActor());
		
		OnBeginLaunchAction.Broadcast();
	}
}

void UCableActionComponent::TryRetractCable()
{
	if (CurrentCableState != ECableState::Launching && CurrentCableState != ECableState::Attaching)
		return;
	
	TryDrivingForce();
	RetractCable();

	OnBeginRetractAction.Broadcast();
}

void UCableActionComponent::TryDrivingForce()
{
	const bool bIsWPressed = CachedPC->IsInputKeyDown(EKeys::W);
	const bool bIsAPressed = CachedPC->IsInputKeyDown(EKeys::A);
	const bool bIsSPressed = CachedPC->IsInputKeyDown(EKeys::S);
	const bool bIsDPressed = CachedPC->IsInputKeyDown(EKeys::D);
	const bool bIsSpacePressed = CachedPC->IsInputKeyDown(EKeys::SpaceBar);
	
	if (!bIsSpacePressed || (!bIsWPressed && !bIsAPressed && !bIsSPressed && !bIsDPressed))
		return;

	const FVector CableDirection = (AttachedLocation - Owner->GetActorLocation()).GetSafeNormal();
	const FVector ForwardDir = Owner->GetActorForwardVector();
	const FVector RightDir = Owner->GetActorRightVector();
    
	FVector InputDirection = FVector::ZeroVector;
	if (bIsWPressed) InputDirection += ForwardDir;
	if (bIsAPressed) InputDirection -= RightDir;
	if (bIsSPressed) InputDirection -= ForwardDir;
	if (bIsDPressed) InputDirection += RightDir;
	InputDirection.Normalize();

	const float InputRadialComponent = FVector::DotProduct(InputDirection, CableDirection);
	FVector InputTangential = InputDirection - (CableDirection * InputRadialComponent);
	InputTangential.Normalize();

	const FVector Force = InputTangential * CableDrivingForce;
    
	Owner->LaunchCharacter(Force, true, true);
}

void UCableActionComponent::LaunchCable(const FVector& HitLocation, AActor* HitActor)
{
	if (CurrentCableState != ECableState::Idle)
		return;
	
	AttachedActor = HitActor;
	AttachedLocation = HitLocation;
	TargetCableLength = FVector::Distance(Owner->GetActorLocation(), HitLocation);

	CurrentCableState = ECableState::Launching;
	VfxProgress = 0.0f;
	
	TargetCable->SetAttachEndTo(HitActor, NAME_None);
	TargetCable->EndLocation = AttachedActor->GetTransform().InverseTransformPosition(Owner->GetActorLocation());
	TargetCable->SetVisibility(true);
}

void UCableActionComponent::ResetCable()
{
	AttachedActor = nullptr;
	
	AttachedLocation = FVector::ZeroVector;
	TargetCable->EndLocation = FVector::ZeroVector;
	
	TargetCableLength = 0.0f;
	
	TargetCable->SetAttachEndToComponent(nullptr);
	TargetCable->SetVisibility(false);

	// Deprecated : Cable Length Adjust Func
	// CurrentCableLength = 0.0f;
}

void UCableActionComponent::RetractCable()
{
	CurrentCableState = ECableState::Retracting;
	VfxProgress = 0.0f;
}

void UCableActionComponent::PlayCableVfx(const float DeltaTime)
{
	if (!AttachedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("CableActionComponent::PlayCableVfx - AttachedActor is null!"));
		return;
	}
	
	FVector StartPos, EndPos;
	
	switch (CurrentCableState)
	{
		case ECableState::Launching:
			StartPos = Owner->GetActorLocation();
			EndPos = AttachedLocation;
			OnWhileLaunchAction.Broadcast();
			break;
	            
		case ECableState::Retracting:
			StartPos = AttachedLocation;
			EndPos = Owner->GetActorLocation();
			OnWhileRetractAction.Broadcast();
			break;
	            
		default:
			return;
	}
	
	VfxProgress += (CableLaunchSpeed * DeltaTime) / TargetCableLength;
	VfxProgress = FMath::Clamp(VfxProgress, 0.0f, 1.0f);

	const FVector CurrentEndLocation = FMath::Lerp(StartPos, EndPos, VfxProgress);
	TargetCable->EndLocation = AttachedActor->GetTransform().InverseTransformPosition(CurrentEndLocation);

	if (VfxProgress >= 1.0f)
	{
		TargetCable->EndLocation = AttachedActor->GetTransform().InverseTransformPosition(EndPos);

		if (CurrentCableState == ECableState::Launching)
		{
			CurrentCableState = ECableState::Attaching;
			OnEndLaunchAction.Broadcast();
		}
		else if (CurrentCableState == ECableState::Retracting)
		{
			CurrentCableState = ECableState::Idle;
			ResetCable();
			OnEndRetractAction.Broadcast();
		}
	}
}

void UCableActionComponent::OnWhileAttach(const float DeltaTime)
{
	// Deprecated : Cable Length Adjust Func
	// if (!FMath::IsNearlyEqual(CurrentCableLength, TargetCableLength, 1.0f))
	// {
	// 	OnCableLengthAdjust(DeltaTime);
	// }
	
	const FVector CharacterLocation = Owner->GetActorLocation();
	const FVector ToSwingPoint = AttachedLocation - CharacterLocation;
	const float CurrentDistance = ToSwingPoint.Size();
	
	if (FMath::Abs(CurrentDistance - TargetCableLength) > 0.1f)
	{
		// Set Position
		const FVector CableDirection = ToSwingPoint.GetSafeNormal();
		const FVector CorrectPosition = AttachedLocation - (CableDirection * TargetCableLength);
		const FVector NewPosition = FMath::VInterpTo(CharacterLocation, CorrectPosition, DeltaTime, CableInterpSpeed);
		Owner->SetActorLocation(NewPosition);

		SetOwnerRotationToCable(CableDirection, DeltaTime);
		
		// Remove Radial Velocity, Keep Tangential Velocity
		const FVector UpdatedDirection = (AttachedLocation - CorrectPosition).GetSafeNormal();
		const FVector CurrentVelocity = Owner->GetVelocity();
		const float RadialVelocity = FVector::DotProduct(CurrentVelocity, UpdatedDirection);
		const FVector TangentialVelocity = CurrentVelocity - (UpdatedDirection * RadialVelocity);
		
		// Pendulum movement
		const FVector GravityInfluence = FVector(0, 0, GetWorld()->GetGravityZ());
		const FVector SwingAcceleration = FVector::CrossProduct(UpdatedDirection, FVector::CrossProduct(GravityInfluence, UpdatedDirection));
		FVector NewVelocity = TangentialVelocity + (SwingAcceleration * DeltaTime);

		if (bIsSteamBoosterActive)
		{
			const FVector SteamBoosterDirection = Owner->GetActorForwardVector();
			NewVelocity += SteamBoosterDirection.GetSafeNormal() * SteamBoosterForce * DeltaTime;
			
			OnWhileSteamBoosterAction.Broadcast();
		}
		
		Owner->GetCharacterMovement()->Velocity = NewVelocity;
	}

	OnWhileAttachAction.Broadcast();
}

void UCableActionComponent::SetOwnerRotationToCable(const FVector& CableDirection, float DeltaTime)
{
	if (!Owner || !Owner->GetMesh()) return;

	const FVector UpDirection = CableDirection.GetSafeNormal();
    const FVector ForwardDirection = Owner->GetActorForwardVector();

	const FMatrix RotationMatrix = FRotationMatrix::MakeFromZX(UpDirection, ForwardDirection);
	
	FQuat TargetRotation = FQuat(RotationMatrix);
	TargetRotation.Normalize();
	const FQuat CurrentQuatRotation = Owner->GetActorRotation().Quaternion();

	FQuat InterpolatedRotation = FMath::QInterpTo(CurrentQuatRotation, TargetRotation, DeltaTime, CableInterpSpeed);
	InterpolatedRotation.Normalize();
    
	Owner->SetActorRotation(InterpolatedRotation);
}

// Deprecated : Cable Length Adjust Func
void UCableActionComponent::OnCableLengthAdjust(const float DeltaTime)
{
	CurrentCableLength = FMath::FInterpTo(CurrentCableLength, TargetCableLength, DeltaTime, CableInterpSpeed);
	TargetCable->CableLength = CurrentCableLength;
}


void UCableActionComponent::ShortenCable()
{
	if (CurrentCableState != ECableState::Attaching) return;

	TargetCableLength = FMath::Clamp(TargetCableLength - CableAdjustSpeed, CableMinLength, CableMaxLength);
}

void UCableActionComponent::ExtendCable()
{
	if (CurrentCableState != ECableState::Attaching) return;

	TargetCableLength = FMath::Clamp(TargetCableLength + CableAdjustSpeed, CableMinLength, CableMaxLength);
}
