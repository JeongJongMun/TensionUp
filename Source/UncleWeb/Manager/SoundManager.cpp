#include "SoundManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UncleWeb/Data/SoundLibrary.h"
#include "UncleWeb/Util/TUDefines.h"

void USoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    USoundLibrary* Library = LoadObject<USoundLibrary>(nullptr, TEXT("/Script/UncleWeb.SoundLibrary'/Game/Data/DA_SoundLibrary.DA_SoundLibrary'"));
    if (!Library)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] SoundLibrary not found!"), CURRENT_CONTEXT);
        return;
    }
    SoundDataAsset = Library;

    CurrentBGMAudioComponent = nullptr;
}

void USoundManager::Deinitialize()
{
    StopAllSounds();
    
    if (CurrentBGMAudioComponent && IsValid(CurrentBGMAudioComponent))
    {
        CurrentBGMAudioComponent->Stop();
        CurrentBGMAudioComponent = nullptr;
    }
    
    CachedSounds.Empty();
    ActiveSFXComponents.Empty();
    
    Super::Deinitialize();
}

void USoundManager::PlaySoundAtLocation(FName SoundName, const FVector& Location, float VolumeMultiplier, float PitchMultiplier, bool bLoop)
{
    if (USoundBase* Sound = SoundDataAsset->SFXMap.FindRef(SoundName))
    {
        PlaySoundAtLocation(Sound, Location, VolumeMultiplier, PitchMultiplier, bLoop);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SFX sound '%s' not found!"), *GetName(), *SoundName.ToString());
    }
}

void USoundManager::PlaySound2D(FName SoundName, float VolumeMultiplier, float PitchMultiplier,bool bLoop)
{
    if (USoundBase* Sound = SoundDataAsset->SFXMap.FindRef(SoundName))
    {
        PlaySound2D(Sound, VolumeMultiplier, PitchMultiplier, bLoop);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SFX sound '%s' not found!"), *GetName(), *SoundName.ToString());
    }
}

void USoundManager::StopSound(const FName SoundName)
{
    if (const TObjectPtr<USoundBase> Sound = SoundDataAsset->SFXMap.FindRef(SoundName))
    {
        for (UAudioComponent* AudioComp : ActiveSFXComponents)
        {
            if (IsValid(AudioComp) && AudioComp->Sound == Sound)
            {
                AudioComp->Stop();
                ActiveSFXComponents.Remove(AudioComp);
                break;
            }
        }
    }
}

void USoundManager::PlaySoundAtLocation(USoundBase* Sound, const FVector& Location, const float VolumeMultiplier, const float PitchMultiplier, const bool bLoop)
{
    if (GetWorld()->IsNetMode(NM_DedicatedServer) || !Sound)
        return;


    const float FinalVolume = SFXVolume * VolumeMultiplier;
    
    if (bLoop)
    {
        const TObjectPtr<UAudioComponent> AudioComp = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, Location, FRotator::ZeroRotator, FinalVolume, PitchMultiplier);
        
        if (AudioComp)
        {
            AudioComp->bAutoDestroy = !bLoop;
            ActiveSFXComponents.Add(AudioComp);
        }
    }
    else
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, FinalVolume, PitchMultiplier);
    }
}

void USoundManager::PlaySound2D(USoundBase* Sound, const float VolumeMultiplier, const float PitchMultiplier, const bool bLoop)
{
    if (GetWorld()->IsNetMode(NM_DedicatedServer) || !Sound)
        return;

    const float FinalVolume = SFXVolume * VolumeMultiplier;
    
    if (bLoop)
    {
        const TObjectPtr<UAudioComponent> AudioComp = UGameplayStatics::SpawnSound2D(GetWorld(), Sound, FinalVolume, PitchMultiplier, 0.0f, nullptr, true, bLoop);
        if (AudioComp)
        {
            ActiveSFXComponents.Add(AudioComp);
        }
    }
    else
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Sound, FinalVolume, PitchMultiplier);
    }
}

void USoundManager::PlayBGM(USoundBase* Sound, const bool bLoop, const float FadeInDuration)
{
    if (!Sound || GetWorld()->IsNetMode(NM_DedicatedServer))
        return;

    if (CurrentBGMAudioComponent && IsValid(CurrentBGMAudioComponent))
    {
        if (FadeInDuration > 0.0f)
        {
            StopBGM(FadeInDuration * 0.5f);
        }
        else
        {
            CurrentBGMAudioComponent->Stop();
            CurrentBGMAudioComponent = nullptr;
        }
    }

    CurrentBGMAudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), Sound, BGMVolume, 1.0f, 0.0f, nullptr, true, bLoop);

    if (CurrentBGMAudioComponent && FadeInDuration > 0.0f)
    {
        CurrentBGMAudioComponent->SetVolumeMultiplier(0.0f);
        FadeBGM(BGMVolume, FadeInDuration);
    }
}

void USoundManager::StopBGM(const float FadeOutDuration)
{
    if (!CurrentBGMAudioComponent || !IsValid(CurrentBGMAudioComponent))
        return;

    if (FadeOutDuration > 0.0f)
    {
        FadeBGM(0.0f, FadeOutDuration);
    }
    else
    {
        CurrentBGMAudioComponent->Stop();
        CurrentBGMAudioComponent = nullptr;
    }
}

void USoundManager::PauseBGM()
{
    if (CurrentBGMAudioComponent && IsValid(CurrentBGMAudioComponent))
    {
        CurrentBGMAudioComponent->SetPaused(true);
    }
}

void USoundManager::ResumeBGM()
{
    if (CurrentBGMAudioComponent && IsValid(CurrentBGMAudioComponent))
    {
        CurrentBGMAudioComponent->SetPaused(false);
    }
}

void USoundManager::SetBGMVolume(const float InVolume)
{
    BGMVolume = FMath::Clamp(InVolume, 0.0f, 1.0f);
    
    if (CurrentBGMAudioComponent && IsValid(CurrentBGMAudioComponent))
    {
        CurrentBGMAudioComponent->SetVolumeMultiplier(BGMVolume);
    }
}

void USoundManager::SetSFXVolume(const float InVolume)
{
    SFXVolume = FMath::Clamp(InVolume, 0.0f, 1.0f);
    
    CleanupFinishedAudioComponents();
    for (UAudioComponent* AudioComp : ActiveSFXComponents)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->SetVolumeMultiplier(SFXVolume);
        }
    }
}

void USoundManager::StopAllSounds()
{
    StopBGM();
    StopAllSFX();
}

void USoundManager::StopAllSFX()
{
    for (UAudioComponent* AudioComp : ActiveSFXComponents)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
    }
    ActiveSFXComponents.Empty();
}

float USoundManager::GetVolumeForType(const ESoundType SoundType, const float VolumeMultiplier) const
{
    switch (SoundType)
    {
        case ESoundType::BGM:
            return BGMVolume * VolumeMultiplier;
        case ESoundType::SFX:
            return SFXVolume * VolumeMultiplier;
        default:
            return VolumeMultiplier;
    }
}

void USoundManager::CleanupFinishedAudioComponents()
{
    ActiveSFXComponents.RemoveAll([](const UAudioComponent* AudioComp)
    {
        return !IsValid(AudioComp) || !AudioComp->IsPlaying();
    });
}

void USoundManager::FadeBGM(float TargetVolume, const float Duration)
{
    if (!CurrentBGMAudioComponent || !IsValid(CurrentBGMAudioComponent))
        return;

    if (GetWorld()->GetTimerManager().IsTimerActive(BGMFadeTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(BGMFadeTimerHandle);
    }

    const float CurrentVolume = CurrentBGMAudioComponent->VolumeMultiplier;
    float VolumeStep = (TargetVolume - CurrentVolume) / (Duration / 0.1f); // 0.1초마다 업데이트

    GetWorld()->GetTimerManager().SetTimer(BGMFadeTimerHandle, [this, TargetVolume, VolumeStep]()
    {
        if (!CurrentBGMAudioComponent || !IsValid(CurrentBGMAudioComponent))
        {
            OnBGMFadeComplete();
            return;
        }

        const float NewVolume = CurrentBGMAudioComponent->VolumeMultiplier + VolumeStep;
        
        const bool bReachedTarget = (VolumeStep > 0.0f) ? (NewVolume >= TargetVolume) : (NewVolume <= TargetVolume);
        
        if (bReachedTarget)
        {
            CurrentBGMAudioComponent->SetVolumeMultiplier(TargetVolume);
            OnBGMFadeComplete();
        }
        else
        {
            CurrentBGMAudioComponent->SetVolumeMultiplier(NewVolume);
        }
    }, 0.1f, true);
}

void USoundManager::OnBGMFadeComplete()
{
    GetWorld()->GetTimerManager().ClearTimer(BGMFadeTimerHandle);
    
    if (CurrentBGMAudioComponent && IsValid(CurrentBGMAudioComponent) && 
        CurrentBGMAudioComponent->VolumeMultiplier <= 0.0f)
    {
        CurrentBGMAudioComponent->Stop();
        CurrentBGMAudioComponent = nullptr;
    }
}