#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
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

    // 기본 사운드 재생 함수들 (타입 지정)
    void PlaySoundAtLocation(USoundBase* Sound, const FVector& Location, ESoundType SoundType, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, bool bLoop = false);
    void PlaySound2D(USoundBase* Sound, ESoundType SoundType, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, bool bLoop = false);

    // BGM 전용 함수들
    void PlayBGM(USoundBase* Sound, bool bLoop = true, float FadeInDuration = 0.0f);
    void StopBGM(float FadeOutDuration = 0.0f);
    void PauseBGM();
    void ResumeBGM();

    // 사운드 캐싱 함수들
    void PreloadSound(USoundBase* Sound);
    void PreloadSounds(const TArray<USoundBase*>& Sounds);

    // 볼륨 조절 함수들
    void SetBGMVolume(float InVolume);
    float GetBGMVolume() const { return BGMVolume; }
    void SetSFXVolume(float InVolume);
    float GetSFXVolume() const { return SFXVolume; }

    // 사운드 정지 함수들
    void StopAllSounds();
    void StopAllSFX();

private:
    // 볼륨 설정
    UPROPERTY()
    float BGMVolume = 0.5f;
    
    UPROPERTY()
    float SFXVolume = 0.5f;
    
    // BGM 관리용 오디오 컴포넌트
    UPROPERTY()
    TObjectPtr<UAudioComponent> CurrentBGMAudioComponent;
    
    // 사운드 캐시
    UPROPERTY()
    TSet<TObjectPtr<USoundBase>> CachedSounds;
    
    // 활성 SFX 오디오 컴포넌트들 (정지용)
    UPROPERTY()
    TArray<TObjectPtr<UAudioComponent>> ActiveSFXComponents;

    float GetVolumeForType(ESoundType SoundType, float VolumeMultiplier) const;
    void CleanupFinishedAudioComponents();
    
    FTimerHandle BGMFadeTimerHandle;
    void FadeBGM(float TargetVolume, float Duration);
    void OnBGMFadeComplete();
};