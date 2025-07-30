#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParkourTypes.h"
#include "Animation/AnimMontage.h"
#include "ParkourComponent.generated.h"

class ATUCharacterPlayer;

// Enum to represent the state of parkour
USTRUCT()
struct FParkourHitInfo
{
	GENERATED_BODY()

	FVector ImpactPoint = FVector::ZeroVector;
	FVector SurfaceNormal = FVector::ZeroVector;
	float WallHeight = 0.f;
	float Quality = 0.f; // 0-1, he higher the better
	bool bIsValid = false;

	FParkourHitInfo() {}
};

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

	bool FindBestClimbableWall(FParkourHitInfo& OutBestHit);
	FParkourHitInfo EvaluateWallHit(const FHitResult& Hit, const FVector& CharacterLocation);
	float MeasureWallHeight(const FVector& ImpactPoint, const FVector& SurfaceNormal);

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

	UPROPERTY()
	bool bDrawDebugBox = false;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	UAnimMontage* ClimbUpMontage;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour", meta = (ClampMin = "0", ClampMax = "90"))
	float MaxWallAngle = 90.f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float MinWallHeight = 1.f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float MaxWallHeight = 200.f;
};