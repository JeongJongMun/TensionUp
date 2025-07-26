// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Teleporter.generated.h"

UCLASS()
class UNCLEWEB_API ATeleporter : public AActor
{
	GENERATED_BODY()
	
public:	
	ATeleporter();

protected:
	virtual void BeginPlay() override;
	FVector FindRestartLocation() const;
	
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBoxComponent> BoxCollision;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> TeleporterMesh;

	FString RestartLocationTag = TEXT("Restart");
	FVector RestartLocation;
};