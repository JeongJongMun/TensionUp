// Fill out your copyright notice in the Description page of Project Settings.

#include "DynamicCamera.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UncleWeb/Character/TUCharacterPlayer.h"
#include "UncleWeb/Util/TUDefines.h"

UDynamicCamera::UDynamicCamera()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UDynamicCamera::BeginPlay()
{
	Super::BeginPlay();

	Owner = CastChecked<ATUCharacterPlayer>(GetOwner());
}

void UDynamicCamera::InitializeCamera()
{
	TargetSpringArm->bUsePawnControlRotation = true;
	TargetSpringArm->SetRelativeLocation(CameraOffset);
}

void UDynamicCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TargetSpringArm || !Owner->GetMovementComponent()) return;

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