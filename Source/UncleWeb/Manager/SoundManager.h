#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
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

    void PlayBGM(FName SoundName, bool bLoop = true);
    void StopBGM();
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
    void PlayBGM(USoundBase* Sound, bool bLoop = true);

    float GetVolumeForType(ESoundType SoundType, float VolumeMultiplier) const;
    void CleanupFinishedAudioComponents();
    

private:
    UPROPERTY()
    TObjectPtr<USoundLibrary> SoundDataAsset;
    
    UPROPERTY()
    float BGMVolume = 0.5f;
    
    UPROPERTY()
    float SFXVolume = 0.5f;
    
    UPROPERTY()
    TObjectPtr<class UAudioComponent> CurrentBGMAudioComponent;
    
    UPROPERTY()
    TSet<TObjectPtr<USoundBase>> CachedSounds;
    
    UPROPERTY()
    TArray<TObjectPtr<class UAudioComponent>> ActiveSFXComponents;

    FTimerHandle BGMFadeTimerHandle;
};