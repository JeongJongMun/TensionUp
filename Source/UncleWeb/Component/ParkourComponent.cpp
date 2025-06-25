#include "ParkourComponent.h"
#include "UncleWeb/Character/TUCharacterPlayer.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
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
	if (!OwnerCharacter || ParkourState != EParkourState::Climbing) return;

	ClimbInterpAlpha += DeltaTime * ClimbSpeed;
	FVector NewLocation = FMath::Lerp(ParkourStartLocation, ParkourTargetLocation, ClimbInterpAlpha);
	OwnerCharacter->SetActorLocation(NewLocation);

	if (ClimbInterpAlpha >= 1.0f)
	{
		ParkourState = EParkourState::ClimbFinished;
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
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
		FVector AttachPoint = BestHit.ImpactPoint + SurfaceNormal * AttachOffset;
		FVector TargetTop = AttachPoint + FVector(0, 0, ClimbHeightOffset);
		float EyeHeight = OwnerCharacter->GetActorLocation().Z + OwnerCharacter->BaseEyeHeight;

		if (TargetTop.Z > EyeHeight + ClimbHeightLimit)
		{
			UE_LOG(LogTemp, Warning, TEXT("Too High - Cannot climb"));
			return;
		}

		BeginClimb(SurfaceNormal, BestHit.ImpactPoint);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No climbable surface found"));
	}
}

void UParkourComponent::BeginClimb(const FVector& SurfaceNormal, const FVector& HitPoint)
{
	ParkourStartLocation = OwnerCharacter->GetActorLocation();
	ParkourTargetLocation = HitPoint + FVector(0, 0, 120.f);
	ClimbInterpAlpha = 0.0f;
	ParkourState = EParkourState::Climbing;

	OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	FRotator LookAtRotation = UKismetMathLibrary::MakeRotFromX(-SurfaceNormal);
	OwnerCharacter->SetActorRotation(LookAtRotation);

}
