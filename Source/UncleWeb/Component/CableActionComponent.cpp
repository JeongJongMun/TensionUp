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

	if (bIsCableAttached)
	{
		OnCableAttaching(DeltaTime);
		if (bIsCableLaunching)
		{
			OnLaunchCable(DeltaTime);
		}
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

	FVector CableDirection = (AttachedLocation - Owner->GetActorLocation()).GetSafeNormal();
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

void UCableActionComponent::SetCable(const FVector& HitLocation, AActor* HitActor)
{
	AttachedActor = HitActor;
	AttachedLocation = HitLocation;
	TargetCableLength = FVector::Distance(Owner->GetActorLocation(), HitLocation);
	
	TargetCable->SetAttachEndTo(HitActor, NAME_None);
	TargetCable->SetVisibility(true);
	
	bIsCableAttached = true;
	bIsCableLaunching = true;
}

void UCableActionComponent::ResetCable()
{
	AttachedActor = nullptr;
	
	bIsCableAttached = false;
	bIsCableLaunching = false;
	
	AttachedLocation = FVector::ZeroVector;
	TargetCable->EndLocation = FVector::ZeroVector;
	
	TargetCableLength = 0.0f;
	LaunchProgress = 0.0f;
	
	TargetCable->SetAttachEndToComponent(nullptr);
	TargetCable->SetVisibility(false);

	// Deprecated : Cable Length Adjust Func
	// CurrentCableLength = 0.0f;
}

void UCableActionComponent::OnLaunchCable(const float DeltaTime)
{
	LaunchProgress += (CableLaunchSpeed * DeltaTime) / TargetCableLength;
	LaunchProgress = FMath::Clamp(LaunchProgress, 0.0f, 1.0f);

	const FVector CurrentEndLocation = FMath::Lerp(Owner->GetActorLocation(), AttachedLocation, LaunchProgress);
	TargetCable->EndLocation = AttachedActor->GetTransform().InverseTransformPosition(CurrentEndLocation);

	if (LaunchProgress >= 1.0f)
	{
		TargetCable->EndLocation = AttachedActor->GetTransform().InverseTransformPosition(AttachedLocation);
		LaunchProgress = 1.0f;
		bIsCableLaunching = false;
	}
}

void UCableActionComponent::OnCableAttaching(const float DeltaTime)
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

void UCableActionComponent::SetOwnerRotationToCable(const FVector& CableDirection, float DeltaTime)
{
	if (!Owner || !Owner->GetMesh()) return;
	
	const FRotator CurrentRotation = Owner->GetActorRotation();

	const FVector TargetUp = CableDirection.GetSafeNormal();
	const FVector Forward = Owner->GetActorForwardVector();

	const FMatrix RotMatrix = FRotationMatrix::MakeFromZX(TargetUp, Forward);
	const FRotator TargetRotator = RotMatrix.Rotator();

	const FRotator NewRotator = FMath::RInterpTo(CurrentRotation, TargetRotator, DeltaTime, CableInterpSpeed);
	const FRotator FinalRotator(NewRotator.Pitch, CurrentRotation.Yaw, NewRotator.Roll);

	Owner->SetActorRotation(FinalRotator);
}

// Deprecated : Cable Length Adjust Func
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
