// Fill out your copyright notice in the Description page of Project Settings.

#include "TUDynamicCamera.h"
#include "GameFramework/Character.h"

UTUDynamicCamera::UTUDynamicCamera()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UTUDynamicCamera::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* OwnerCharacter = CastChecked<ACharacter>(GetOwner());
	MovementComponent = OwnerCharacter->GetCharacterMovement();
}

void UTUDynamicCamera::InitializeCamera()
{
	TargetSpringArm->bUsePawnControlRotation = true;
	TargetSpringArm->SetRelativeLocation(Offset);
}

void UTUDynamicCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TargetSpringArm || !MovementComponent) return;

	float CurrentSpeed = MovementComponent->Velocity.Size();
	float Alpha = FMath::Clamp(CurrentSpeed / MaxSpeed, 0.0f, 1.0f);
	float TargetLength = FMath::Lerp(MinArmLength, MaxArmLength, Alpha);

	float NewLength = FMath::FInterpTo(TargetSpringArm->TargetArmLength, TargetLength, DeltaTime, InterpolationSpeed);
	TargetSpringArm->TargetArmLength = NewLength;
}
