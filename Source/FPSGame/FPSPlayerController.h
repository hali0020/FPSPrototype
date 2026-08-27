#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSMenuWidget.h"
#include "FPSPlayerController.generated.h"

UCLASS()
class FPSGAME_API AFPSPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AFPSPlayerController();

    virtual void SetupInputComponent() override;

    void StartGameFromMenu();
    void ResumeGame();
    void RestartGame();
    void ReturnToMainMenu();
    void QuitToDesktop();
    void ShowDeathMenu(int32 FinalScore);

    bool IsPauseMenuVisible() const { return ActiveScreen == EFPSMenuScreen::Pause; }

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

private:
    void TogglePauseMenu();
    void ConfirmMenuAction();
    void ShowMenu(EFPSMenuScreen Screen, int32 FinalScore = 0);
    void CloseMenuForGameplay();
    void PrepareForTravel();
    void DisablePawnGameplayInput();

    UPROPERTY() TObjectPtr<UFPSMenuWidget> ActiveMenu;
    EFPSMenuScreen ActiveScreen = EFPSMenuScreen::None;
};
