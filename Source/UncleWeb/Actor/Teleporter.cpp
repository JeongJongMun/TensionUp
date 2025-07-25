// Fill out your copyright notice in the Description page of Project Settings.
#include "Teleporter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UncleWeb/Character/TUCharacterPlayer.h"

ATeleporter::ATeleporter()
{
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	RootComponent = BoxCollision;
	BoxCollision->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
    
	BoxCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	BoxCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	BoxCollision->SetGenerateOverlapEvents(true);

	TeleporterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeleporterMesh"));
	TeleporterMesh->SetupAttachment(RootComponent);
	TeleporterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TeleporterMesh->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> TeleporterMeshAsset(TEXT("/Script/Engine.StaticMesh'/Game/Art/Mesh/SM_Cube.SM_Cube'"));
	if (TeleporterMeshAsset.Succeeded())
	{
		TeleporterMesh->SetStaticMesh(TeleporterMeshAsset.Object);
	}
}

void ATeleporter::BeginPlay()
{
	Super::BeginPlay();

	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ATeleporter::OnTriggerBeginOverlap);
	RestartLocation = FindRestartLocation();
}

FVector ATeleporter::FindRestartLocation() const
{
	if (!RestartLocationTag.IsEmpty())
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
        
		for (AActor* Actor : PlayerStarts)
		{
			if (APlayerStart* PS = Cast<APlayerStart>(Actor))
			{
				if (PS->PlayerStartTag.ToString() == RestartLocationTag)
				{
					return PS->GetActorLocation();
				}
			}
		}
	}
	
	if (const APlayerStart* DefaultPlayerStart = Cast<APlayerStart>(UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass())))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Using default PlayerStart location."), CURRENT_CONTEXT);
		return DefaultPlayerStart->GetActorLocation();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[%s] No PlayerStart found in the level."), CURRENT_CONTEXT);
	return FVector::ZeroVector;
}

void ATeleporter::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA<ATUCharacterPlayer>())
	{
		OtherActor->SetActorLocation(RestartLocation);
	}
}