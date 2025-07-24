#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "UncleWeb/Data/SoundLibrary.h"
#include "SoundManager.generated.h"

UENUM()
enum class ESoundType : uint8
{
    BGM     UMETA(DisplayName = "Background Music"),
    SFX     UMETA(DisplayName = "Sound Effect"),
};

UCLASS()
class UNCLEWEB_API USoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void PlaySoundAtLocation(FName SoundName, const FVector& Location, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, bool bLoop = false);
    void PlaySound2D(FName SoundName, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, bool bLoop = false);
    void StopSound(FName SoundName);

    void PlayBGM(USoundBase* Sound, bool bLoop = true, float FadeInDuration = 0.0f);
    void StopBGM(float FadeOutDuration = 0.0f);
    void PauseBGM();
    void ResumeBGM();

    void SetBGMVolume(float InVolume);
    float GetBGMVolume() const { return BGMVolume; }
    void SetSFXVolume(float InVolume);
    float GetSFXVolume() const { return SFXVolume; }

    void StopAllSounds();
    void StopAllSFX();

private:
    void PlaySoundAtLocation(USoundBase* Sound, const FVector& Location, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, bool bLoop = false);
    void PlaySound2D(USoundBase* Sound, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, bool bLoop = false);
    
    float GetVolumeForType(ESoundType SoundType, float VolumeMultiplier) const;
    void CleanupFinishedAudioComponents();
    void FadeBGM(float TargetVolume, float Duration);
    void OnBGMFadeComplete();
    

private:
    UPROPERTY()
    TObjectPtr<USoundLibrary> SoundDataAsset;
    
    UPROPERTY()
    float BGMVolume = 0.5f;
    
    UPROPERTY()
    float SFXVolume = 0.5f;
    
    UPROPERTY()
    TObjectPtr<UAudioComponent> CurrentBGMAudioComponent;
    
    UPROPERTY()
    TSet<TObjectPtr<USoundBase>> CachedSounds;
    
    UPROPERTY()
    TArray<TObjectPtr<UAudioComponent>> ActiveSFXComponents;

    FTimerHandle BGMFadeTimerHandle;
};