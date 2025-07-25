// SoundLibrary.h
#pragma once

#include "Engine/DataAsset.h"
#include "Sound/SoundBase.h"
#include "SoundLibrary.generated.h"

UCLASS()
class USoundLibrary : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TMap<FName, TObjectPtr<USoundBase>> SFXMap;

	UPROPERTY(EditAnywhere)
	TMap<FName, TObjectPtr<USoundBase>> BGMMap;
};
