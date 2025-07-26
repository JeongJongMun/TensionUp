// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "UncleWeb/Util/TUDefines.h"
#include "SteamCharger.generated.h"

UCLASS()
class UNCLEWEB_API ASteamCharger : public AActor
{
	GENERATED_BODY()
	
public:	
	ASteamCharger();
	virtual void Tick(float DeltaTime) override;
	void ChangeState(ESteamChargerState NewState);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void OnDisabled();
	void OnEnabled();

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> SphereMesh;

private:
	UPROPERTY(EditAnywhere, Category = "Config|SteamCharger")
	float RegenerationTime = 8.0f;
	
	ESteamChargerState CurrentState = ESteamChargerState::None;
	float ElapsedTime = 0.0f;

	UPROPERTY()
	TObjectPtr<class USoundManager> SoundManager;
};