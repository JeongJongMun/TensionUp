#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParkourTypes.h"
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
	void TickParkour(float DeltaTime);

	EParkourState GetParkourState() const { return ParkourState; }

private:
	void BeginClimb(const FVector& SurfaceNormal, const FVector& HitPoint);

	// --------------------
	// Variables
	// --------------------
	UPROPERTY()
	ATUCharacterPlayer* OwnerCharacter;

	EParkourState ParkourState = EParkourState::None;
	FVector ParkourStartLocation;
	FVector ParkourTargetLocation;
	float ClimbInterpAlpha = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	FVector BoxTraceHalfSize = FVector(30.f, 50.f, 40.f);

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float AttachOffset = 30.f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ClimbHeightOffset = 120.f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ClimbHeightLimit = 100.f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ParkourMaxDistance = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ClimbSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	bool bDrawDebugBox = true;
};
