#include "SoundManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void USoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
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

void USoundManager::PlaySoundAtLocation(USoundBase* Sound, const FVector& Location, const ESoundType SoundType, const float VolumeMultiplier, const float PitchMultiplier, const bool bLoop)
{
    if (GetWorld()->IsNetMode(NM_DedicatedServer) || !Sound)
        return;

    if (!CachedSounds.Contains(Sound))
    {
        PreloadSound(Sound);
    }

    const float FinalVolume = GetVolumeForType(SoundType, VolumeMultiplier);
    
    if (bLoop || SoundType == ESoundType::BGM)
    {
        const TObjectPtr<UAudioComponent> AudioComp = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, Location, FRotator::ZeroRotator, FinalVolume, PitchMultiplier);
        
        if (AudioComp)
        {
            AudioComp->bAutoDestroy = !bLoop;
            
            if (SoundType == ESoundType::SFX)
            {
                ActiveSFXComponents.Add(AudioComp);
            }
        }
    }
    else
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, FinalVolume, PitchMultiplier);
    }
}

void USoundManager::PlaySound2D(USoundBase* Sound, const ESoundType SoundType, const float VolumeMultiplier, const float PitchMultiplier, const bool bLoop)
{
    if (GetWorld()->IsNetMode(NM_DedicatedServer) || !Sound)
        return;

    if (!CachedSounds.Contains(Sound))
    {
        PreloadSound(Sound);
    }

    const float FinalVolume = GetVolumeForType(SoundType, VolumeMultiplier);
    
    if (bLoop || SoundType == ESoundType::BGM)
    {
        const TObjectPtr<UAudioComponent> AudioComp = UGameplayStatics::SpawnSound2D(GetWorld(), Sound, FinalVolume, PitchMultiplier, 0.0f, nullptr, true, bLoop);
        
        if (AudioComp)
        {
            if (SoundType == ESoundType::SFX)
            {
                ActiveSFXComponents.Add(AudioComp);
            }
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

    PreloadSound(Sound);

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

void USoundManager::PreloadSound(USoundBase* Sound)
{
    if (Sound && !CachedSounds.Contains(Sound))
    {
        Sound->AddToRoot();
        CachedSounds.Add(Sound);
    }
}

void USoundManager::PreloadSounds(const TArray<USoundBase*>& Sounds)
{
    for (USoundBase* Sound : Sounds)
    {
        PreloadSound(Sound);
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
    
    // 활성 SFX 컴포넌트들의 볼륨 업데이트
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

    // 기존 타이머 정리
    if (GetWorld()->GetTimerManager().IsTimerActive(BGMFadeTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(BGMFadeTimerHandle);
    }

    const float CurrentVolume = CurrentBGMAudioComponent->VolumeMultiplier;
    float VolumeStep = (TargetVolume - CurrentVolume) / (Duration / 0.1f); // 0.1초마다 업데이트

    // 람다로 페이드 처리
    GetWorld()->GetTimerManager().SetTimer(BGMFadeTimerHandle, [this, TargetVolume, VolumeStep]()
    {
        if (!CurrentBGMAudioComponent || !IsValid(CurrentBGMAudioComponent))
        {
            OnBGMFadeComplete();
            return;
        }

        const float NewVolume = CurrentBGMAudioComponent->VolumeMultiplier + VolumeStep;
        
        // 목표 볼륨에 도달했는지 체크
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
    
    // 볼륨이 0이면 BGM 정지
    if (CurrentBGMAudioComponent && IsValid(CurrentBGMAudioComponent) && 
        CurrentBGMAudioComponent->VolumeMultiplier <= 0.0f)
    {
        CurrentBGMAudioComponent->Stop();
        CurrentBGMAudioComponent = nullptr;
    }
}