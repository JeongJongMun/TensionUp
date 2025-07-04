#include "ParkourComponent.h"
#include "UncleWeb/Character/TUCharacterPlayer.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UParkourComponent::UParkourComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UParkourComponent::Initialize(ATUCharacterPlayer* InOwner)
{
	OwnerCharacter = InOwner;
}

void UParkourComponent::TickParkour(float DeltaTime)
{
	// if (!OwnerCharacter || ParkourState != EParkourState::Climbing) return;
	//
	// ClimbInterpAlpha += DeltaTime * ClimbSpeed;
	// FVector NewLocation = FMath::Lerp(ParkourStartLocation, ParkourTargetLocation, ClimbInterpAlpha);
	// //OwnerCharacter->SetActorLocation(NewLocation);
	//
	// if (ClimbInterpAlpha >= 1.0f)
	// {
	// 	ParkourState = EParkourState::ClimbFinished;
	// 	OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	// }
}

void UParkourComponent::TryParkour()
{
	if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement()->IsFalling()) return;

	FVector TraceStart = OwnerCharacter->GetActorLocation() + FVector(0, 0, 50.f);
	FVector BoxHalfSize = BoxTraceHalfSize;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	TArray<FVector> Directions = {
		OwnerCharacter->GetActorForwardVector(),
		OwnerCharacter->GetActorForwardVector().RotateAngleAxis(30.f, FVector::UpVector),
		OwnerCharacter->GetActorForwardVector().RotateAngleAxis(-30.f, FVector::UpVector)
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
			Hit, TraceStart, TraceEnd, FQuat(Orientation),
			ECC_Visibility, FCollisionShape::MakeBox(BoxHalfSize), Params);

		if (bDrawDebugBox)
		{
			DrawDebugBox(GetWorld(), TraceEnd, BoxHalfSize, FQuat(Orientation), bHit ? FColor::Green : FColor::Red, false, 1.f);
		}

		if (bHit)
		{
			float Distance = FVector::Dist(OwnerCharacter->GetActorLocation(), Hit.ImpactPoint);
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
		const FVector ImpactPoint = BestHit.ImpactPoint;

		if (!IsSurfaceClearAboveForClimb(ImpactPoint, Params))
		{
			UE_LOG(LogTemp, Warning, TEXT("Surface blocked above - Cannot climb"));
			return;
		}

		BeginParkour(SurfaceNormal, ImpactPoint);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No climbable surface found"));
	}
	// for debug purposes, draw the best hit point and the target climb point
	if (bDrawDebugBox)
	{
		FVector EyeReference = OwnerCharacter->GetActorLocation() + FVector(0, 0, OwnerCharacter->BaseEyeHeight);
		FVector ClimbLimitZ = EyeReference + FVector(0, 0, ClimbHeightLimit);

		FVector AttachPoint = BestHit.ImpactPoint + BestHit.ImpactNormal * AttachOffset;
		FVector TargetTop = AttachPoint + FVector(0, 0, ClimbHeightOffset);

		DrawDebugSphere(GetWorld(), TargetTop, 10.f, 12, FColor::Yellow, false, 1.f);
		DrawDebugSphere(GetWorld(), ClimbLimitZ, 10.f, 12, FColor::Magenta, false, 1.f);
	}
}

bool UParkourComponent::IsSurfaceClearAboveForClimb(const FVector& ImpactPoint, const FCollisionQueryParams& Params) const
{
	if (!OwnerCharacter) return false;

	FVector EyeLocation = OwnerCharacter->GetActorLocation() + FVector(0, 0, OwnerCharacter->BaseEyeHeight);

	FVector ClimbTargetLocation = ImpactPoint + FVector(0, 0, ClimbHeightOffset);

	if (ClimbTargetLocation.Z <= EyeLocation.Z)
	{
		return false;
	}

	if (ClimbTargetLocation.Z > EyeLocation.Z + ClimbHeightLimit)
	{
		return false;
	}

	FVector CheckStart = ClimbTargetLocation;
	FVector CheckEnd = CheckStart + FVector(0, 0, OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 20.f);

	FHitResult Hit;
	bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, CheckStart, CheckEnd, ECC_Visibility, Params);

	FVector ForwardCheckStart = ClimbTargetLocation + OwnerCharacter->GetActorForwardVector() * 30.f;
	FVector ForwardCheckEnd = ForwardCheckStart + FVector(0, 0, OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	FHitResult ForwardHit;
	bool bForwardBlocked = GetWorld()->LineTraceSingleByChannel(ForwardHit, ForwardCheckStart, ForwardCheckEnd, ECC_Visibility, Params);
	
	//for debug purposes, draw the check lines
	/*
	if (bDrawDebugBox)
	{
		DrawDebugLine(GetWorld(), CheckStart, CheckEnd, bBlocked ? FColor::Red : FColor::Green, false, 2.f, 0, 2.f);
		if (bBlocked)
		{
			DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 12.f, FColor::Red, false, 2.f);
		}

		DrawDebugLine(GetWorld(), ForwardCheckStart, ForwardCheckEnd, bForwardBlocked ? FColor::Orange : FColor::Blue, false, 2.f, 0, 2.f);
		if (bForwardBlocked)
		{
			DrawDebugPoint(GetWorld(), ForwardHit.ImpactPoint, 10.f, FColor::Orange, false, 2.f);
		}
	}*/

	return !bBlocked && !bForwardBlocked;
}


void UParkourComponent::BeginParkour(const FVector& SurfaceNormal, const FVector& HitPoint)
{
	ParkourStartLocation = OwnerCharacter->GetActorLocation();
	ParkourTargetLocation = HitPoint + FVector(0, 0, ClimbHeightOffset);
	ClimbInterpAlpha = 0.0f;
	ParkourState = EParkourState::Climbing;

	OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	FRotator ForwardRotation = OwnerCharacter->GetActorForwardVector().Rotation();
	OwnerCharacter->SetActorRotation(ForwardRotation);

	if (ClimbUpMontage && OwnerCharacter->GetMesh())
	{
		float MontageLength = ClimbUpMontage->GetPlayLength();
		if (MontageLength > 0.f)
		{
			ClimbSpeed = 1.f / MontageLength;
		}

		if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(ClimbUpMontage);
		}
	}
}
