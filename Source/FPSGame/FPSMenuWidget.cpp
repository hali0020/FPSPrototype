#include "FPSMenuWidget.h"
#include "FPSPlayerController.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void UFPSMenuWidget::Configure(EFPSMenuScreen InScreen, int32 InScore)
{
    Screen = InScreen;
    Score = InScore;
}

TSharedRef<SWidget> UFPSMenuWidget::RebuildWidget()
{
    FString Title;
    FString Subtitle;
    FString PrimaryLabel;

    switch (Screen)
    {
    case EFPSMenuScreen::Pause:
        Title = TEXT("GAME PAUSED");
        Subtitle = TEXT("THE BATTLE IS FROZEN");
        PrimaryLabel = TEXT("RESUME");
        break;
    case EFPSMenuScreen::Death:
        Title = TEXT("YOU DIED");
        Subtitle = FString::Printf(TEXT("FINAL SCORE  %06d"), Score);
        PrimaryLabel = TEXT("RESTART");
        break;
    case EFPSMenuScreen::Main:
    default:
        Title = TEXT("FPS PROTOTYPE");
        Subtitle = TEXT("SURVIVE THE WAVES");
        PrimaryLabel = TEXT("START GAME");
        break;
    }

    const FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 44.0f);
    const FSlateFontInfo SubtitleFont = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 15.0f);
    const FSlateFontInfo ButtonFont = FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 18.0f);
    const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));

    auto MakeButton = [ButtonFont](const FString& Label, const FOnClicked& OnClicked)
        -> TSharedRef<SWidget>
    {
        return SNew(SBox)
            .WidthOverride(340.0f)
            .HeightOverride(56.0f)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                .ContentPadding(FMargin(18.0f, 10.0f))
                .ButtonColorAndOpacity(FLinearColor(0.06f, 0.18f, 0.25f, 1.0f))
                .OnClicked(OnClicked)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(Label))
                    .Font(ButtonFont)
                    .ColorAndOpacity(FLinearColor(0.92f, 0.97f, 1.0f, 1.0f))
                    .Justification(ETextJustify::Center)
                ]
            ];
    };

    TSharedRef<SVerticalBox> MenuBox = SNew(SVerticalBox);
    MenuBox->AddSlot()
        .AutoHeight()
        .HAlign(HAlign_Center)
        .Padding(0.0f, 0.0f, 0.0f, 8.0f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Title))
            .Font(TitleFont)
            .ColorAndOpacity(FLinearColor(0.92f, 0.97f, 1.0f, 1.0f))
            .Justification(ETextJustify::Center)
        ];

    MenuBox->AddSlot()
        .AutoHeight()
        .HAlign(HAlign_Center)
        .Padding(0.0f, 0.0f, 0.0f, 30.0f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Subtitle))
            .Font(SubtitleFont)
            .ColorAndOpacity(FLinearColor(0.28f, 0.82f, 1.0f, 1.0f))
            .Justification(ETextJustify::Center)
        ];

    MenuBox->AddSlot()
        .AutoHeight()
        .HAlign(HAlign_Center)
        .Padding(0.0f, 6.0f)
        [
            MakeButton(PrimaryLabel,
                FOnClicked::CreateUObject(this, &UFPSMenuWidget::HandlePrimaryClicked))
        ];

    if (Screen == EFPSMenuScreen::Pause)
    {
        MenuBox->AddSlot()
            .AutoHeight()
            .HAlign(HAlign_Center)
            .Padding(0.0f, 6.0f)
            [
                MakeButton(TEXT("RESTART"),
                    FOnClicked::CreateUObject(this, &UFPSMenuWidget::HandleSecondaryClicked))
            ];
    }

    if (Screen == EFPSMenuScreen::Pause || Screen == EFPSMenuScreen::Death)
    {
        MenuBox->AddSlot()
            .AutoHeight()
            .HAlign(HAlign_Center)
            .Padding(0.0f, 6.0f)
            [
                MakeButton(TEXT("MAIN MENU"),
                    FOnClicked::CreateUObject(this, &UFPSMenuWidget::HandleMainMenuClicked))
            ];
    }

    MenuBox->AddSlot()
        .AutoHeight()
        .HAlign(HAlign_Center)
        .Padding(0.0f, 6.0f)
        [
            MakeButton(TEXT("EXIT GAME"),
                FOnClicked::CreateUObject(this, &UFPSMenuWidget::HandleQuitClicked))
        ];

    MenuBox->AddSlot()
        .AutoHeight()
        .HAlign(HAlign_Center)
        .Padding(0.0f, 24.0f, 0.0f, 0.0f)
        [
            SNew(STextBlock)
            .Text(Screen == EFPSMenuScreen::Pause
                ? FText::FromString(TEXT("ESC  RESUME"))
                : FText::FromString(TEXT("ENTER  CONFIRM")))
            .Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 12.0f))
            .ColorAndOpacity(FLinearColor(0.48f, 0.56f, 0.62f, 1.0f))
        ];

    return SNew(SOverlay)
        + SOverlay::Slot()
        [
            SNew(SBorder)
            .Padding(0.0f)
            .BorderImage(WhiteBrush)
            .BorderBackgroundColor(FLinearColor(0.004f, 0.012f, 0.02f, 0.94f))
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SBorder)
            .Padding(FMargin(58.0f, 44.0f))
            .BorderImage(WhiteBrush)
            .BorderBackgroundColor(FLinearColor(0.015f, 0.045f, 0.065f, 0.98f))
            [
                MenuBox
            ]
        ];
}

FReply UFPSMenuWidget::HandlePrimaryClicked()
{
    if (AFPSPlayerController* Controller = GetOwningPlayer<AFPSPlayerController>())
    {
        switch (Screen)
        {
        case EFPSMenuScreen::Pause:
            Controller->ResumeGame();
            break;
        case EFPSMenuScreen::Death:
            Controller->RestartGame();
            break;
        case EFPSMenuScreen::Main:
        default:
            Controller->StartGameFromMenu();
            break;
        }
    }
    return FReply::Handled();
}

FReply UFPSMenuWidget::HandleSecondaryClicked()
{
    if (AFPSPlayerController* Controller = GetOwningPlayer<AFPSPlayerController>())
    {
        Controller->RestartGame();
    }
    return FReply::Handled();
}

FReply UFPSMenuWidget::HandleMainMenuClicked()
{
    if (AFPSPlayerController* Controller = GetOwningPlayer<AFPSPlayerController>())
    {
        Controller->ReturnToMainMenu();
    }
    return FReply::Handled();
}

FReply UFPSMenuWidget::HandleQuitClicked()
{
    if (AFPSPlayerController* Controller = GetOwningPlayer<AFPSPlayerController>())
    {
        Controller->QuitToDesktop();
    }
    return FReply::Handled();
}
