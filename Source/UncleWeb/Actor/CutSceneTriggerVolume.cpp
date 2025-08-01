#include "CutSceneTriggerVolume.h"
#include "Kismet/GameplayStatics.h"
#include "UncleWeb/UI/UIManager.h"
#include <UncleWeb/Character/TUCharacterPlayer.h>

ACutSceneTriggerVolume::ACutSceneTriggerVolume()
{
    OnActorBeginOverlap.AddDynamic(this, &ACutSceneTriggerVolume::OnTriggerEnter);
}

void ACutSceneTriggerVolume::BeginPlay()
{
    Super::BeginPlay();
}

void ACutSceneTriggerVolume::OnTriggerEnter(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!OtherActor->IsA<ATUCharacterPlayer>()) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;

    AUIManager* UIManager = Cast<AUIManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AUIManager::StaticClass()));
    if (!UIManager || !CutSceneTexture) return;

    UIManager->ShowCutSceneImage(CutSceneTexture, CutSceneDuration);

    FTimerHandle QuitHandle;
    GetWorld()->GetTimerManager().SetTimer(QuitHandle, []()
        {
            UE_LOG(LogTemp, Warning, TEXT("[Trigger] Quitting game now"));
            UKismetSystemLibrary::QuitGame(GWorld, nullptr, EQuitPreference::Quit, false);
        }, CutSceneDuration, false);

    Destroy();
}