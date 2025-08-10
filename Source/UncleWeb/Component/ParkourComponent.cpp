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

void UParkourComponent::TryParkour()
{
	if (!OwnerCharacter || !OwnerCharacter->GetCharacterMovement()->IsFalling()) return;

	if (OwnerCharacter->IsCableAttached())
	{
		return;
	}

	FParkourHitInfo BestHit;
	if (FindBestClimbableWall(BestHit))
	{
		BeginParkour(BestHit.SurfaceNormal, BestHit.ImpactPoint);
	}
}

bool UParkourComponent::FindBestClimbableWall(FParkourHitInfo& OutBestHit)
{
	if (!OwnerCharacter) return false;

	FVector CharacterLocation = OwnerCharacter->GetActorLocation();
	FVector TraceStart = CharacterLocation - FVector(0, 0, 10.f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);
	Params.bTraceComplex = false;

	TArray<FParkourHitInfo> ValidHits;

	for (int32 Angle = -45; Angle <= 45; Angle += 15)
	{
		FVector Direction = OwnerCharacter->GetActorForwardVector().RotateAngleAxis(Angle, FVector::UpVector);

		TArray<float> HeightOffsets = { 0.f, 25.f, -25.f };
		for (float HeightOffset : HeightOffsets)
		{
			FVector CurrentTraceStart = TraceStart + FVector(0, 0, HeightOffset);
			FVector TraceEnd = CurrentTraceStart + Direction * ParkourMaxDistance;

			FHitResult Hit;
			bool bHit = GetWorld()->SweepSingleByChannel(
				Hit, CurrentTraceStart, TraceEnd, FQuat::Identity,
				ECC_Visibility, FCollisionShape::MakeBox(BoxTraceHalfSize), Params);

			if (bHit)
			{
				FParkourHitInfo HitInfo = EvaluateWallHit(Hit, CharacterLocation);
				if (HitInfo.bIsValid)
				{
					ValidHits.Add(HitInfo);
				}
			}
			/*
			if (bDrawDebugBox)
			{
				DrawDebugBox(GetWorld(), TraceEnd, BoxTraceHalfSize,
					FQuat::Identity, bHit ? FColor::Green : FColor::Red, false, 0.5f);
			}
			*/
		}
	}

	if (ValidHits.Num() > 0)
	{
		ValidHits.Sort([](const FParkourHitInfo& A, const FParkourHitInfo& B) {
			return A.Quality > B.Quality;
			});

		OutBestHit = ValidHits[0];

		/*
		if (bDrawDebugBox)
		{
			FVector TargetTop = OutBestHit.ImpactPoint + OutBestHit.SurfaceNormal * AttachOffset + FVector(0, 0, ClimbHeightOffset);
			DrawDebugSphere(GetWorld(), TargetTop, 10.f, 12, FColor::Yellow, false, 2.f);
			DrawDebugString(GetWorld(), OutBestHit.ImpactPoint + FVector(0, 0, 50.f),
				FString::Printf(TEXT("Q: %.2f, H: %.0f"), OutBestHit.Quality, OutBestHit.WallHeight),
				nullptr, FColor::White, 2.f);
		}
		*/

		return true;
	}

	return false;
}

FParkourHitInfo UParkourComponent::EvaluateWallHit(const FHitResult& Hit, const FVector& CharacterLocation)
{
	FParkourHitInfo HitInfo;
	HitInfo.ImpactPoint = Hit.ImpactPoint;
	HitInfo.SurfaceNormal = Hit.ImpactNormal;

	float WallAngle = FMath::RadiansToDegrees(FMath::Acos(FMath::Abs(FVector::DotProduct(HitInfo.SurfaceNormal, FVector::UpVector))));
	if (WallAngle > MaxWallAngle)
	{
		if (bDrawDebugBox)
		{
			UE_LOG(LogTemp, Warning, TEXT("Wall toooooo angled: %.1f degrees (Max: %.1f)"), WallAngle, MaxWallAngle);
		}
		return HitInfo; // bIsValid = false
	}


	HitInfo.WallHeight = MeasureWallHeight(Hit.ImpactPoint, Hit.ImpactNormal);

	if (HitInfo.WallHeight < MinWallHeight * 0.5f || HitInfo.WallHeight > MaxWallHeight)
	{
		if (bDrawDebugBox)
		{
			UE_LOG(LogTemp, Warning, TEXT("Wall height invalid: %.0f (Min: %.0f, Max: %.0f)"),
				HitInfo.WallHeight, MinWallHeight, MaxWallHeight);
		}
		return HitInfo;
	}


	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	if (!IsSurfaceClearAboveForClimb(HitInfo.ImpactPoint, Params))
	{
		if (bDrawDebugBox)
		{
			UE_LOG(LogTemp, Warning, TEXT("Target climb location blocked"));
		}
		return HitInfo;
	}

	float DistanceScore = 1.f - (FVector::Dist(CharacterLocation, Hit.ImpactPoint) / ParkourMaxDistance);
	float AngleScore = 1.f - (WallAngle / MaxWallAngle);
	float HeightScore = FMath::Clamp((HitInfo.WallHeight - MinWallHeight) / (MaxWallHeight - MinWallHeight), 0.f, 1.f);

	HitInfo.Quality = (DistanceScore * 0.4f) + (AngleScore * 0.4f) + (HeightScore * 0.2f);
	HitInfo.bIsValid = true;

	return HitInfo;
}

float UParkourComponent::MeasureWallHeight(const FVector& ImpactPoint, const FVector& SurfaceNormal)
{
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	FVector DownStart = ImpactPoint + SurfaceNormal * -10.f;
	FVector DownEnd = DownStart + FVector(0, 0, -MaxWallHeight);

	FHitResult BottomHit;
	bool bBottomHit = GetWorld()->LineTraceSingleByChannel(BottomHit, DownStart, DownEnd, ECC_Visibility, Params);

	FVector WallBottom = BottomHit.bBlockingHit ? BottomHit.ImpactPoint : (ImpactPoint - FVector(0, 0, 10.f));

	FVector UpStart = WallBottom + SurfaceNormal * -10.f;
	FVector UpEnd = UpStart + FVector(0, 0, MaxWallHeight + 100.f);

	FHitResult TopHit;
	bool bTopHit = GetWorld()->LineTraceSingleByChannel(TopHit, UpStart, UpEnd, ECC_Visibility, Params);

	if (bTopHit)
	{
		float MeasuredHeight = TopHit.ImpactPoint.Z - WallBottom.Z;

		if (MeasuredHeight < MinWallHeight)
		{
			MeasuredHeight = MinWallHeight * 0.75f;
		}

		/*
		if (bDrawDebugBox)
		{
			DrawDebugLine(GetWorld(), WallBottom, TopHit.ImpactPoint, FColor::Magenta, false, 2.f, 0, 2.f);
		}
		*/

		return FMath::Max(0.f, MeasuredHeight);
	}

	return MinWallHeight * 0.9f;
}



// Check if the surface above the target climb location is clear for climbing
bool UParkourComponent::IsSurfaceClearAboveForClimb(const FVector& ImpactPoint, const FCollisionQueryParams& Params) const
{
	if (!OwnerCharacter) return false;

	FVector EyeLocation = OwnerCharacter->GetActorLocation() + FVector(0, 0, OwnerCharacter->BaseEyeHeight);
	FVector ClimbTargetLocation = ImpactPoint + FVector(0, 0, ClimbHeightOffset);

	if (ClimbTargetLocation.Z <= EyeLocation.Z || ClimbTargetLocation.Z > EyeLocation.Z + ClimbHeightLimit)
		return false;

	// check if the target climb location is within the allowed height limit
	FVector BoxCenter = ClimbTargetLocation + FVector(0, 0, OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FVector BoxHalf = FVector(30.f, 30.f, OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()); // character width + height

	FHitResult OccupyHit;
	bool bClimbLocationBlocked = GetWorld()->SweepSingleByChannel(
		OccupyHit,
		BoxCenter,
		BoxCenter,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeBox(BoxHalf),
		Params
	);

	// check the space above the target climb location
	FVector CheckStart = ClimbTargetLocation;
	FVector CheckEnd = CheckStart + FVector(0, 0, OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 20.f); // character height + some offset

	FHitResult Hit;
	bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, CheckStart, CheckEnd, ECC_Visibility, Params);

	FVector ForwardCheckStart = ClimbTargetLocation + OwnerCharacter->GetActorForwardVector() * 30.f; // little offset forward
	FVector ForwardCheckEnd = ForwardCheckStart + FVector(0, 0, OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	FHitResult ForwardHit;
	bool bForwardBlocked = GetWorld()->LineTraceSingleByChannel(ForwardHit, ForwardCheckStart, ForwardCheckEnd, ECC_Visibility, Params);

	/*
	if (bDrawDebugBox)
	{
		DrawDebugLine(GetWorld(), CheckStart, CheckEnd, bBlocked ? FColor::Red : FColor::Green, false, 1.f, 0, 2.f);
		if (bBlocked)
		{
			DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 12.f, FColor::Red, false, 1.f);
		}

		DrawDebugLine(GetWorld(), ForwardCheckStart, ForwardCheckEnd, bForwardBlocked ? FColor::Orange : FColor::Blue, false, 1.f, 0, 2.f);
		if (bForwardBlocked)
		{
			DrawDebugPoint(GetWorld(), ForwardHit.ImpactPoint, 10.f, FColor::Orange, false, 1.f);
		}
	}
	*/

	return !bBlocked && !bForwardBlocked && !bClimbLocationBlocked;
}

void UParkourComponent::BeginParkour(const FVector& SurfaceNormal, const FVector& HitPoint)
{
	ParkourStartLocation = OwnerCharacter->GetActorLocation();

	// Calculate the target location for climbing
	FVector AttachOffsetVec = SurfaceNormal * AttachOffset;
	ParkourTargetLocation = HitPoint + AttachOffsetVec + FVector(0, 0, ClimbHeightOffset);

	ParkourState = EParkourState::Climbing;
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	FRotator LookAtWallRotation = UKismetMathLibrary::MakeRotFromX(-SurfaceNormal);
	OwnerCharacter->SetActorRotation(LookAtWallRotation);

	// Set the character location to the start location
	OwnerCharacter->SetActorLocation(ParkourStartLocation + FVector(0, 0, -50.f));

	if (ClimbUpMontage && OwnerCharacter->GetMesh())
	{
		if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(ClimbUpMontage);
		}
	}
}

void UParkourComponent::AnimNotify_ParkourClimbEnd()
{
	if (!OwnerCharacter) return;
	if (ParkourState != EParkourState::Climbing) return;

	// if Climbing -> set the character location to the target location
	if (ParkourState == EParkourState::Climbing)
	{
		ParkourState = EParkourState::ClimbFinished; // set the state to finished
		OwnerCharacter->SetActorLocation(ParkourTargetLocation);
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
}