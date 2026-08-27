#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSMenuWidget.generated.h"

class FReply;
class SWidget;

UENUM()
enum class EFPSMenuScreen : uint8
{
    None,
    Main,
    Pause,
    Death
};

UCLASS()
class FPSGAME_API UFPSMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void Configure(EFPSMenuScreen InScreen, int32 InScore = 0);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    FReply HandlePrimaryClicked();
    FReply HandleSecondaryClicked();
    FReply HandleMainMenuClicked();
    FReply HandleQuitClicked();

    EFPSMenuScreen Screen = EFPSMenuScreen::Main;
    int32 Score = 0;
};
