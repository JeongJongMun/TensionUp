// Fill out your copyright notice in the Description page of Project Settings.

#include "DynamicCamera.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UncleWeb/Character/TUCharacterPlayer.h"

UDynamicCamera::UDynamicCamera()
{
	PrimaryComponentTick.bCanEverTick = true;
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
}

void UDynamicCamera::InitializeCamera() const
{
	TargetSpringArm->bUsePawnControlRotation = true;
	TargetSpringArm->SetRelativeLocation(CameraOffset);
}

void UDynamicCamera::UpdatePostProcessVolume() const
{
	if (!Owner->IsCableAttached())
		return;
	
	const float CurrentSpeed = Owner->GetVelocity().Size();
	const float SpeedRatio = FMath::Clamp(CurrentSpeed / SpeedThreshold_D, 0.0f, 1.0f);
    
	PostProcessVolume->Settings.MotionBlurAmount = FMath::Lerp(MotionBlurAmountMin, MotionBlurAmountMax, SpeedRatio);
	PostProcessVolume->Settings.MotionBlurMax = FMath::Lerp(MotionBlurDistortionMin, MotionBlurDistortionMax, SpeedRatio);

	const float TargetFOV = FMath::Lerp(FOVMin, FOVMax, SpeedRatio);
        
	FollowCamera->SetFieldOfView(TargetFOV);
}

void UDynamicCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TargetSpringArm || !Owner->GetMovementComponent()) return;
	
	UpdatePostProcessVolume();

	const float CurrentSpeed = Owner->GetMovementComponent()->Velocity.Size();
	float TargetLength;
	if (Owner->IsCableAttached())
		TargetLength = CalculateTargetArmLength(CurrentSpeed);
	else
		TargetLength = MinSpringArmLength;

	const float NewLength = FMath::FInterpTo(TargetSpringArm->TargetArmLength, TargetLength, DeltaTime, CameraInterpolationSpeed);
	TargetSpringArm->TargetArmLength = NewLength;
	
	static int32 Counter = 0;
	//if (++Counter % 20 == 0)
		//UE_LOG(LogTemp, Log, TEXT("[%s] Current Speed: %f / Current Spring Arm Length: %f"), CURRENT_CONTEXT, CurrentSpeed, NewLength);
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