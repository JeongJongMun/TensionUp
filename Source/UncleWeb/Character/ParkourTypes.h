#pragma once

#include "CoreMinimal.h"
#include "ParkourTypes.generated.h"

UENUM(BlueprintType)
enum class EParkourState : uint8
{
	None        UMETA(DisplayName = "None"),
	Climbing    UMETA(DisplayName = "Climbing"),
	ClimbFinished UMETA(DisplayName = "Climb Finished")
};
