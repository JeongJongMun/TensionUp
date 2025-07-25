// Fill out your copyright notice in the Description page of Project Settings.

#include "SteamCharger.h"
#include "UncleWeb/Character/TUCharacterPlayer.h"

ASteamCharger::ASteamCharger()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	RootComponent = SphereCollision;
	SphereCollision->SetSphereRadius(100.0f);

	SphereCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	SphereCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SphereCollision->SetGenerateOverlapEvents(true);

	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	SphereMesh->SetupAttachment(RootComponent);
	SphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereMesh->SetRelativeLocation(FVector::ZeroVector);
	SphereMesh->SetRelativeRotation(FRotator::ZeroRotator);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Script/Engine.StaticMesh'/Game/Art/Mesh/SM_Sphere.SM_Sphere'"));
	if (SphereMeshAsset.Succeeded())
	{
		SphereMesh->SetStaticMesh(SphereMeshAsset.Object);
	}
}

void ASteamCharger::BeginPlay()
{
	Super::BeginPlay();

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ASteamCharger::OnOverlapBegin);
}

void ASteamCharger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == ESteamChargerState::Disabled)
	{
		ElapsedTime += DeltaTime;
		if (ElapsedTime >= RegenerationTime)
		{
			ChangeState(ESteamChargerState::Enabled);
			ElapsedTime = 0.0f;
		}
	}
}

void ASteamCharger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA<ATUCharacterPlayer>())
	{
		ChangeState(ESteamChargerState::Disabled);
	}
}

void ASteamCharger::ChangeState(const ESteamChargerState NewState)
{
	if (CurrentState == NewState)
		return;
	
	CurrentState = NewState;
	switch (NewState)
	{
		case ESteamChargerState::Disabled:
			OnDisabled();
			break;
		
		case ESteamChargerState::Enabled:
			OnEnabled();
			break;
		
		default:
			break;
	}
}

void ASteamCharger::OnDisabled()
{
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereMesh->SetVisibility(false);
}

void ASteamCharger::OnEnabled()
{
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SphereMesh->SetVisibility(true);
}