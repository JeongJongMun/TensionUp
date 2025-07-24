#pragma once

#include "UObject/Object.h"
#include "USoundCollectUtil.generated.h"

UCLASS(BlueprintType)
class USoundCollectUtil : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(CallInEditor, BlueprintCallable)
	void CollectAllSounds();
};
