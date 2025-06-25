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
	UParkourComponent();

	void Initialize(ATUCharacterPlayer* InOwner);
	void TryParkour();
	void TickParkour(float DeltaTime);

	EParkourState GetParkourState() const { return ParkourState; }

private:
	UPROPERTY()
	ATUCharacterPlayer* OwnerCharacter;

	EParkourState ParkourState = EParkourState::None;
	FVector ParkourStartLocation;
	FVector ParkourTargetLocation;
	float ClimbInterpAlpha = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ParkourMaxDistance = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ParkourMaxHeight = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Config|Parkour")
	float ClimbSpeed = 1.0f;

	void BeginClimb(const FVector& SurfaceNormal, const FVector& HitPoint);
};
