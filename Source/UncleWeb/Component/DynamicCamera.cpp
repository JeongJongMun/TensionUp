// Fill out your copyright notice in the Description page of Project Settings.

#include "DynamicCamera.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "UncleWeb/Character/TUCharacterPlayer.h"

UDynamicCamera::UDynamicCamera()
{
	PrimaryComponentTick.bCanEverTick = true;

	const ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> CollectionFinder(TEXT("/Script/Engine.MaterialParameterCollection'/Game/Materials/MPC_PostProcessingRadialBlur.MPC_PostProcessingRadialBlur'"));
	if (CollectionFinder.Succeeded())
	{
		MaterialParameterCollection = CollectionFinder.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] MaterialParameterCollection not found!"), CURRENT_CONTEXT);
	}
}


void UDynamicCamera::BeginPlay()
{
	Super::BeginPlay();

	Owner = CastChecked<ATUCharacterPlayer>(GetOwner());

	if (const UWorld* World = GetWorld())
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClassWithTag(World, APostProcessVolume::StaticClass(), FName("PostProcessVolume"),FoundActors);
        
		if (FoundActors.Num() > 0)
		{
			PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
		}
	}

	MaterialParameterCollectionInstance = GetWorld()->GetParameterCollectionInstance(MaterialParameterCollection);
	if (!MaterialParameterCollectionInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] MaterialParameterCollectionInstance not found!"), CURRENT_CONTEXT);
	}
}

void UDynamicCamera::InitializeCamera() const
{
	TargetSpringArm->bUsePawnControlRotation = true;
	TargetSpringArm->SetRelativeLocation(CameraOffset);
}

void UDynamicCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TargetSpringArm || !Owner->GetMovementComponent()) return;

	
	UpdatePostProcessVolume(DeltaTime);

	const float CurrentSpeed = Owner->GetMovementComponent()->Velocity.Size();
	float TargetLength;
	if (Owner->IsCableAttached())
		TargetLength = CalculateTargetArmLength(CurrentSpeed);
	else
		TargetLength = MinSpringArmLength;

	const float NewLength = FMath::FInterpTo(TargetSpringArm->TargetArmLength, TargetLength, DeltaTime, CameraInterpolationSpeed);
	TargetSpringArm->TargetArmLength = NewLength;
}

float UDynamicCamera::CalculateTargetArmLength(const float CurrentSpeed) const
{
	if (CurrentSpeed <= SpeedThreshold_A)
		return MinSpringArmLength;
    
	if (CurrentSpeed <= SpeedThreshold_B)
	{
		const float Alpha = (CurrentSpeed - SpeedThreshold_A) / (SpeedThreshold_B - SpeedThreshold_A);
		return FMath::Lerp(MinSpringArmLength, MidSpringArmLength, Alpha);
	}
    
	if (CurrentSpeed <= SpeedThreshold_C)
		return MidSpringArmLength;
    
	if (CurrentSpeed <= SpeedThreshold_D)
	{
		const float Alpha = (CurrentSpeed - SpeedThreshold_C) / (SpeedThreshold_D - SpeedThreshold_C);
		return FMath::Lerp(MidSpringArmLength, MaxSpringArmLength, Alpha);
	}
    
	return MaxSpringArmLength;
}

void UDynamicCamera::UpdatePostProcessVolume(const float DeltaTime) const
{
	if (!PostProcessVolume || !MaterialParameterCollectionInstance || !FollowCamera)
		return;

	const bool bCable = Owner->IsCableAttached();
	const bool bDash = Owner->IsDashing();

	// dash
	if (bDash && !bCable)
	{
		// maybe creat variable for the dash
		PostProcessVolume->Settings.MotionBlurAmount = 0.5f;
		PostProcessVolume->Settings.MotionBlurMax = 2.5f;

		const float CurrentFOV = FollowCamera->FieldOfView;
		const float TargetFOV = 120.0f;
		const float InterpolatedFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 5.0f);
		FollowCamera->SetFieldOfView(InterpolatedFOV);

		MaterialParameterCollectionInstance->SetScalarParameterValue(RadialBlurIntensityName, 0.5f);
		MaterialParameterCollectionInstance->SetScalarParameterValue(RadialBlurRadiusName, 0.5f);

		return;
	}

	// cable
	if (bCable)
	{
		const float CurrentSpeed = Owner->GetVelocity().Size();
		const float SpeedRatio = FMath::Clamp(CurrentSpeed / SpeedThreshold_D * 1.5f, 0.0f, 1.0f);

		const float CurrentMotionBlurAmount = PostProcessVolume->Settings.MotionBlurAmount;
		const float CurrentMotionBlurMax = PostProcessVolume->Settings.MotionBlurMax;
		const float TargetMotionBlurAmount = FMath::Lerp(0.0f, MotionBlurAmountMax, SpeedRatio);
		const float TargetMotionBlurDistortion = FMath::Lerp(0.0f, MotionBlurDistortionMax, SpeedRatio);
		PostProcessVolume->Settings.MotionBlurAmount = FMath::FInterpTo(CurrentMotionBlurAmount, TargetMotionBlurAmount, DeltaTime, PostProcessingInterpolationSpeed);
		PostProcessVolume->Settings.MotionBlurMax = FMath::FInterpTo(CurrentMotionBlurMax, TargetMotionBlurDistortion, DeltaTime, PostProcessingInterpolationSpeed);

		const float CurrentFOV = FollowCamera->FieldOfView;
		const float TargetFOV = FMath::Lerp(FOVMin, FOVMax, SpeedRatio);
		const float InterpolatedFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, PostProcessingInterpolationSpeed);
		FollowCamera->SetFieldOfView(InterpolatedFOV);

		float CurrentRadialBlurIntensity, CurrentRadialBlurRadius;
		MaterialParameterCollectionInstance->GetScalarParameterValue(RadialBlurIntensityName, CurrentRadialBlurIntensity);
		MaterialParameterCollectionInstance->GetScalarParameterValue(RadialBlurRadiusName, CurrentRadialBlurRadius);
		const float TargetRadialBlurIntensity = FMath::Lerp(0.0f, RadialBlurIntensityMax, SpeedRatio);
		const float TargetRadialBlurRadius = FMath::Lerp(0.0f, RadialBlurRadiusMax, SpeedRatio);
		const float InterpolatedRadialBlurIntensity = FMath::FInterpTo(CurrentRadialBlurIntensity, TargetRadialBlurIntensity, DeltaTime, PostProcessingInterpolationSpeed);
		const float InterpolatedRadialBlurRadius = FMath::FInterpTo(CurrentRadialBlurRadius, TargetRadialBlurRadius, DeltaTime, PostProcessingInterpolationSpeed);
		MaterialParameterCollectionInstance->SetScalarParameterValue(RadialBlurIntensityName, InterpolatedRadialBlurIntensity);
		MaterialParameterCollectionInstance->SetScalarParameterValue(RadialBlurRadiusName, InterpolatedRadialBlurRadius);

		return;
	}

	PostProcessVolume->Settings.MotionBlurAmount = FMath::FInterpTo(PostProcessVolume->Settings.MotionBlurAmount, 0.0f, DeltaTime, PostProcessingInterpolationSpeed);
	PostProcessVolume->Settings.MotionBlurMax = FMath::FInterpTo(PostProcessVolume->Settings.MotionBlurMax, 0.0f, DeltaTime, PostProcessingInterpolationSpeed);

	const float CurrentFOV = FollowCamera->FieldOfView;
	FollowCamera->SetFieldOfView(FMath::FInterpTo(CurrentFOV, 90.0f, DeltaTime, PostProcessingInterpolationSpeed));

	float CurrentRadialBlurIntensity, CurrentRadialBlurRadius;
	MaterialParameterCollectionInstance->GetScalarParameterValue(RadialBlurIntensityName, CurrentRadialBlurIntensity);
	MaterialParameterCollectionInstance->GetScalarParameterValue(RadialBlurRadiusName, CurrentRadialBlurRadius);

	const float RadialBlurIntensityValue = FMath::FInterpTo(CurrentRadialBlurIntensity, 0.0f, DeltaTime, PostProcessingInterpolationSpeed);
	const float RadialBlurRadiusValue = FMath::FInterpTo(CurrentRadialBlurRadius, 0.0f, DeltaTime, PostProcessingInterpolationSpeed);
	MaterialParameterCollectionInstance->SetScalarParameterValue(RadialBlurIntensityName, RadialBlurIntensityValue);
	MaterialParameterCollectionInstance->SetScalarParameterValue(RadialBlurRadiusName, RadialBlurRadiusValue);
}
