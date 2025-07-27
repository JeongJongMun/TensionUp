#include "SettingWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UncleWeb/Character/TUCharacterPlayer.h"
#include "UncleWeb/Framework/SettingSaveGame.h"

USettingWidget::USettingWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void USettingWidget::NativeConstruct()
{
    Super::NativeConstruct();
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        CachedPC = PC;
    }

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
        ApplyButton->OnClicked.AddDynamic(this, &USettingWidget::ToggleWidget);
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

	if (TopPanel)
	{
		TopPanelSlot = Cast<UCanvasPanelSlot>(TopPanel->Slot);
	}
	if (BottomPanel)
	{
		BottomPanelSlot = Cast<UCanvasPanelSlot>(BottomPanel->Slot);
	}

    LeftGears.Empty();
    LeftGears.Add(LeftGear_0);
    
    RightGears.Empty();
    RightGears.Add(RightGear_0);
    RightGears.Add(RightGear_1);
}

void USettingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsAnimating)
	{
		UpdateSlideAnimation(InDeltaTime);
	}

    UpdateGearRotation(InDeltaTime);
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

void USettingWidget::ToggleWidget()
{
	if (bIsAnimating)
		return;

	SoundManager->PlaySound2D("SW_GearRotate", 1.0f, 1.0f, true);
	SoundManager->PlaySound2D("SW_Sys_Button");
	if (!bIsOn)
	{
		SetVisibility(ESlateVisibility::Visible);
		ShowWidget();
		StartSlideInAnimation();
		bIsOn = true;
	}
	else
	{
		StartSlideOutAnimation();
		bIsOn = false;
	}
}

void USettingWidget::ShowWidget()
{
    if (!CachedPC)
        return;

    CachedPC->SetInputMode(FInputModeUIOnly());
    CachedPC->SetShowMouseCursor(true);

    SetVisibility(ESlateVisibility::Visible);
    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void USettingWidget::HideWidget()
{
    USettingSaveGame* SaveGameInstance = Cast<USettingSaveGame>(UGameplayStatics::CreateSaveGameObject(USettingSaveGame::StaticClass()));

    if (!SaveGameInstance || !CachedPC)
        return;
    
    SaveGameInstance->BgmVolume = BgmValue;
    SaveGameInstance->SfxVolume = SfxValue;
    SaveGameInstance->MouseSensitivity = MouseSensitivityValue;
    
    UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameSlotName, SaveGameSlotIndex);
    
    CachedPC->SetInputMode(FInputModeGameOnly());
    CachedPC->SetShowMouseCursor(false);
    SetVisibility(ESlateVisibility::Hidden);
    UGameplayStatics::SetGamePaused(GetWorld(), false);
}

void USettingWidget::OnExitButtonClicked()
{
    UGameplayStatics::OpenLevel(this, FName(*MainMenuLevelName));
}

void USettingWidget::UpdateGearRotation(const float DeltaTime)
{
    for (auto& LeftGear : LeftGears)
    {
        if (!LeftGear) continue;
        
        FWidgetTransform CurrentTransform = LeftGear->GetRenderTransform();
        float NewRotation = CurrentTransform.Angle + (90.0f * DeltaTime);
        
        if (FMath::Abs(NewRotation) >= 360.0f)
        {
            NewRotation = FMath::Fmod(NewRotation, 360.0f);
        }
        
        CurrentTransform.Angle = NewRotation;
        LeftGear->SetRenderTransform(CurrentTransform);
    }
    
    for (auto& RightGear : RightGears)
    {
        if (!RightGear) continue;
        
        FWidgetTransform CurrentTransform = RightGear->GetRenderTransform();
        float NewRotation = CurrentTransform.Angle + (-90.0f * DeltaTime);
        
        if (FMath::Abs(NewRotation) >= 360.0f)
        {
            NewRotation = FMath::Fmod(NewRotation, 360.0f);
        }
        
        CurrentTransform.Angle = NewRotation;
        RightGear->SetRenderTransform(CurrentTransform);
    }
}

void USettingWidget::StartSlideInAnimation()
{
	if (!TopPanelSlot || !BottomPanelSlot)
		return;
	
	bIsAnimating = true;
	bIsSlideIn = true;
	CurrentAnimationTime = 0.0f;
    
	StartOffset = SlideDistance;
	TargetOffset = 0.0f;

	TopPanelSlot->SetOffsets(FMargin(0.0f, -StartOffset, 0.0f, StartOffset));
	BottomPanelSlot->SetOffsets(FMargin(0.0f, StartOffset, 0.0f, -StartOffset));
}

void USettingWidget::StartSlideOutAnimation()
{
	if (!TopPanelSlot || !BottomPanelSlot)
		return;
	
	bIsAnimating = true;
	bIsSlideIn = false;
	CurrentAnimationTime = 0.0f;
    
	StartOffset = TopPanelSlot->GetPosition().Y;
	TargetOffset = SlideDistance;
}

void USettingWidget::UpdateSlideAnimation(const float DeltaTime)
{
	CurrentAnimationTime += DeltaTime;
	
	const float Progress = FMath::Clamp(CurrentAnimationTime / AnimationDuration, 0.0f, 1.0f);
	const float EasedProgress = UKismetMathLibrary::Ease(0.0f, 1.0f, Progress, EasingFunction);
    
	const float CurrentOffset = FMath::Lerp(StartOffset, TargetOffset, EasedProgress);
    
	if (TopPanelSlot)
	{
		TopPanelSlot->SetOffsets(FMargin(0.0f, -CurrentOffset, 0.0f, CurrentOffset));
	}
    
	if (BottomPanelSlot)
	{
		BottomPanelSlot->SetOffsets(FMargin(0.0f, CurrentOffset, 0.0f, -CurrentOffset));
	}
    
	if (Progress >= 1.0f)
	{
		bIsAnimating = false;

		bIsSlideIn ? OnSlideInComplete() : OnSlideOutComplete();
	}
}

void USettingWidget::OnSlideInComplete()
{
	if (TopPanelSlot && BottomPanelSlot)
	{
		TopPanelSlot->SetOffsets(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
		BottomPanelSlot->SetOffsets(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
	}
	SoundManager->StopSound("SW_GearRotate");
}

void USettingWidget::OnSlideOutComplete()
{
	SetVisibility(ESlateVisibility::Hidden);
    
	if (TopPanelSlot && BottomPanelSlot)
	{
		TopPanelSlot->SetOffsets(FMargin(0.0f, -SlideDistance, 0.0f, SlideDistance));
		BottomPanelSlot->SetOffsets(FMargin(0.0f, SlideDistance, 0.0f, -SlideDistance));
	}
	SoundManager->StopSound("SW_GearRotate");
	HideWidget();
}