#include "SettingWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "UncleWeb/Character/TUCharacterPlayer.h"
#include "UncleWeb/Framework/SettingSaveGame.h"

USettingWidget::USettingWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void USettingWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (const UWorld* World = GetWorld())
    {
        if (const UGameInstance* GI = World->GetGameInstance())
        {
            SoundManager = GI->GetSubsystem<USoundManager>();
        }
    }

    if (UGameplayStatics::DoesSaveGameExist(SaveGameSlotName, SaveGameSlotIndex))
    {
        USettingSaveGame* LoadedGame = Cast<USettingSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveGameSlotName, SaveGameSlotIndex));

        if (LoadedGame)
        {
            BgmValue = LoadedGame->BgmVolume;
            SfxValue = LoadedGame->SfxVolume;
            MouseSensitivityValue = LoadedGame->MouseSensitivity;
        }
    }
    
    if (ApplyButton)
    {
        ApplyButton->OnClicked.AddDynamic(this, &USettingWidget::OnApplyButtonClicked);
    }
    if (ExitButton)
    {
        ExitButton->OnClicked.AddDynamic(this, &USettingWidget::OnExitButtonClicked);
    }

    InitBgmSlider();
    InitSfxSlider();
    InitMouseSensitivitySlider();
    UpdateSliderText(BgmText, BgmValue);
    UpdateSliderText(SfxText, SfxValue);
    UpdateSliderText(MouseSensitivityText, MouseSensitivityValue);
}

void USettingWidget::UpdateSliderText(const TObjectPtr<UTextBlock>& TextBlock, const float Value)
{
    if (TextBlock)
    {
        FNumberFormattingOptions NumberFormatOptions;
        NumberFormatOptions.MinimumFractionalDigits = 2;
        NumberFormatOptions.MaximumFractionalDigits = 2;
        
        TextBlock->SetText(FText::AsNumber(Value, &NumberFormatOptions));
    }
}

void USettingWidget::InitBgmSlider()
{
    if (BgmSlider)
    {
        BgmSlider->OnValueChanged.AddDynamic(this, &USettingWidget::OnBgmVolumeChanged);
        BgmSlider->SetValue(BgmValue);
    }
}

void USettingWidget::InitSfxSlider()
{
    if (SfxSlider)
    {
        SfxSlider->OnValueChanged.AddDynamic(this, &USettingWidget::OnSfxVolumeChanged);
        SfxSlider->SetValue(SfxValue);
    }
}

void USettingWidget::InitMouseSensitivitySlider()
{
    if (MouseSensitivitySlider)
    {
        MouseSensitivitySlider->OnValueChanged.AddDynamic(this, &USettingWidget::OnMouseSensitivityChanged);
        MouseSensitivitySlider->SetValue(MouseSensitivityValue);
    }
}

void USettingWidget::OnBgmVolumeChanged(const float Value)
{
    BgmValue = Value;
    UpdateSliderText(BgmText, BgmValue);

    SoundManager->SetBGMVolume(BgmValue);
}

void USettingWidget::OnSfxVolumeChanged(const float Value)
{
    SfxValue = Value;
    UpdateSliderText(SfxText, SfxValue);

    SoundManager->SetSFXVolume(SfxValue);
}

void USettingWidget::OnMouseSensitivityChanged(const float Value)
{
    MouseSensitivityValue = Value;
    UpdateSliderText(MouseSensitivityText, MouseSensitivityValue);

    if (const APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (ATUCharacterPlayer* MyCharacter = Cast<ATUCharacterPlayer>(PC->GetPawn()))
        {
            MyCharacter->SetMouseSensitivity(Value);
        }
    }
}

void USettingWidget::OnApplyButtonClicked()
{
    USettingSaveGame* SaveGameInstance = Cast<USettingSaveGame>(UGameplayStatics::CreateSaveGameObject(USettingSaveGame::StaticClass()));

    if (SaveGameInstance)
    {
        SaveGameInstance->BgmVolume = BgmValue;
        SaveGameInstance->SfxVolume = SfxValue;
        SaveGameInstance->MouseSensitivity = MouseSensitivityValue;
        
        UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameSlotName, SaveGameSlotIndex);
        
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            PC->SetInputMode(FInputModeGameOnly());
            PC->SetShowMouseCursor(false);
        }
        SetVisibility(ESlateVisibility::Hidden);
        UGameplayStatics::SetGamePaused(GetWorld(), false);
    }
}

void USettingWidget::OnExitButtonClicked()
{
    UGameplayStatics::OpenLevel(this, FName(*MainMenuLevelName));
}
