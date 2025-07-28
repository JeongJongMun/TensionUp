#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParkourTypes.h"
#include "Animation/AnimMontage.h"
#include "ParkourComponent.generated.h"

class ATUCharacterPlayer;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UNCLEWEB_API UParkourComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// --------------------
	// Functions
	// --------------------
	UParkourComponent();

	void Initialize(ATUCharacterPlayer* InOwner);
	void TryParkour();

	// Check if the character is currently in a parkour state
	EParkourState GetParkourState() const { return ParkourState; }

	// anim notify functions
	UFUNCTION(BlueprintCallable)
	void AnimNotify_ParkourClimbEnd();

private:
	void BeginParkour(const FVector& SurfaceNormal, const FVector& HitPoint);
	bool IsSurfaceClearAboveForClimb(const FVector& ImpactPoint, const FCollisionQueryParams& Params) const;



	// --------------------
	// Variables
	// --------------------
	UPROPERTY()
	ATUCharacterPlayer* OwnerCharacter;

	EParkourState ParkourState = EParkourState::None;
	FVector ParkourStartLocation;
	FVector ParkourTargetLocation;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	FVector BoxTraceHalfSize = FVector(30.f, 50.f, 50.f);

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float AttachOffset = 20.f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ClimbHeightOffset = 120.f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ClimbHeightLimit = 100.f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ParkourMaxDistance = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	bool bDrawDebugBox = false;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	UAnimMontage* ClimbUpMontage;
};