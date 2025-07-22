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
	if (Owner->IsCableAttached())
	{
		const float CurrentSpeed = Owner->GetVelocity().Size();
		const float SpeedRatio = FMath::Clamp(CurrentSpeed / SpeedThreshold_D, 0.0f, 1.0f);
    
		PostProcessVolume->Settings.MotionBlurAmount = FMath::Lerp(MotionBlurAmountMin, MotionBlurAmountMax, SpeedRatio);
		PostProcessVolume->Settings.MotionBlurMax = FMath::Lerp(MotionBlurDistortionMin, MotionBlurDistortionMax, SpeedRatio);

		const float TargetFOV = FMath::Lerp(FOVMin, FOVMax, SpeedRatio);
        
		FollowCamera->SetFieldOfView(TargetFOV);

		const float RadialBlurIntensityValue = FMath::Lerp(0.0f, RadialBlurIntensityMax, SpeedRatio);
		const float RadialBlurRadiusValue = FMath::Lerp(0.0f, RadialBlurRadiusMax, SpeedRatio);
		
		MaterialParameterCollectionInstance->SetScalarParameterValue(RadialBlurIntensityName, RadialBlurIntensityValue);
		MaterialParameterCollectionInstance->SetScalarParameterValue(RadialBlurRadiusName, RadialBlurRadiusValue);
	}
	else
	{
		const float CurrentMotionBlurAmount = PostProcessVolume->Settings.MotionBlurAmount;
		const float CurrentMotionBlurMax = PostProcessVolume->Settings.MotionBlurMax;
		const float CurrentFOV = FollowCamera->FieldOfView;

		PostProcessVolume->Settings.MotionBlurAmount = FMath::FInterpTo(CurrentMotionBlurAmount, 0.0f, DeltaTime, PostProcessingResetInterpolationSpeed);
		PostProcessVolume->Settings.MotionBlurMax = FMath::FInterpTo(CurrentMotionBlurMax, 0.0f, DeltaTime, PostProcessingResetInterpolationSpeed);
		FollowCamera->SetFieldOfView(FMath::FInterpTo(CurrentFOV, 90.0f, DeltaTime, PostProcessingResetInterpolationSpeed));
	
		float CurrentRadialBlurIntensity;
		MaterialParameterCollectionInstance->GetScalarParameterValue(RadialBlurIntensityName, CurrentRadialBlurIntensity);
		float CurrentRadialBlurRadius;
		MaterialParameterCollectionInstance->GetScalarParameterValue(RadialBlurRadiusName, CurrentRadialBlurRadius);
	
		const float RadialBlurIntensityValue = FMath::FInterpTo(CurrentRadialBlurIntensity, 0.0f, DeltaTime, PostProcessingResetInterpolationSpeed);
		const float RadialBlurRadiusValue = FMath::FInterpTo(CurrentRadialBlurRadius, 0.0f, DeltaTime, PostProcessingResetInterpolationSpeed);
		MaterialParameterCollectionInstance->SetScalarParameterValue(RadialBlurIntensityName, RadialBlurIntensityValue);
		MaterialParameterCollectionInstance->SetScalarParameterValue(RadialBlurRadiusName, RadialBlurRadiusValue);
	}
}
