#include "FPSPlayerController.h"
#include "FPSCharacter.h"
#include "FPSGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPSMenu, Log, All);

AFPSPlayerController::AFPSPlayerController()
{
    bShouldPerformFullTickWhenPaused = true;
}

void AFPSPlayerController::BeginPlay()
{
    Super::BeginPlay();

    AFPSGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFPSGameMode>() : nullptr;
    if (GameMode && GameMode->IsWaitingForStart())
    {
        ShowMenu(EFPSMenuScreen::Main);
        UE_LOG(LogFPSMenu, Display, TEXT("Main menu ready; gameplay has not started."));
    }
    else
    {
        CloseMenuForGameplay();
    }
}

void AFPSPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if (ActiveScreen != EFPSMenuScreen::None)
    {
        DisablePawnGameplayInput();
    }
}

void AFPSPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (!InputComponent) return;

    FInputActionBinding& PauseBinding = InputComponent->BindAction(
        TEXT("PauseMenu"), IE_Pressed, this, &AFPSPlayerController::TogglePauseMenu);
    PauseBinding.bExecuteWhenPaused = true;

    FInputActionBinding& ConfirmBinding = InputComponent->BindAction(
        TEXT("Restart"), IE_Pressed, this, &AFPSPlayerController::ConfirmMenuAction);
    ConfirmBinding.bExecuteWhenPaused = true;
}

void AFPSPlayerController::StartGameFromMenu()
{
    if (ActiveScreen != EFPSMenuScreen::Main) return;

    if (AFPSGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFPSGameMode>() : nullptr)
    {
        GameMode->RequestStartGame();
    }
    CloseMenuForGameplay();
    UE_LOG(LogFPSMenu, Display, TEXT("Main menu closed; gameplay started."));
}

void AFPSPlayerController::ResumeGame()
{
    if (ActiveScreen != EFPSMenuScreen::Pause) return;
    CloseMenuForGameplay();
    UE_LOG(LogFPSMenu, Display, TEXT("Pause menu closed; gameplay resumed."));
}

void AFPSPlayerController::RestartGame()
{
    PrepareForTravel();
    if (AFPSGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFPSGameMode>() : nullptr)
    {
        GameMode->RequestRestart();
    }
}

void AFPSPlayerController::ReturnToMainMenu()
{
    PrepareForTravel();
    if (AFPSGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFPSGameMode>() : nullptr)
    {
        GameMode->RequestMainMenu();
    }
}

void AFPSPlayerController::QuitToDesktop()
{
    PrepareForTravel();
    UE_LOG(LogFPSMenu, Display, TEXT("Exit Game requested."));
    UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void AFPSPlayerController::ShowDeathMenu(int32 FinalScore)
{
    AFPSGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFPSGameMode>() : nullptr;
    if (!GameMode || !GameMode->IsGameOver()) return;

    ShowMenu(EFPSMenuScreen::Death, FinalScore);
    UE_LOG(LogFPSMenu, Display, TEXT("Death menu shown. Final score: %d"), FinalScore);
}

void AFPSPlayerController::TogglePauseMenu()
{
    AFPSGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFPSGameMode>() : nullptr;
    if (!GameMode || GameMode->IsWaitingForStart() || GameMode->IsGameOver()) return;

    if (ActiveScreen == EFPSMenuScreen::Pause)
    {
        ResumeGame();
    }
    else if (ActiveScreen == EFPSMenuScreen::None)
    {
        ShowMenu(EFPSMenuScreen::Pause);
        UE_LOG(LogFPSMenu, Display, TEXT("Gameplay paused by PauseMenu input."));
    }
}

void AFPSPlayerController::ConfirmMenuAction()
{
    switch (ActiveScreen)
    {
    case EFPSMenuScreen::Main:
        StartGameFromMenu();
        break;
    case EFPSMenuScreen::Pause:
        ResumeGame();
        break;
    case EFPSMenuScreen::Death:
        RestartGame();
        break;
    case EFPSMenuScreen::None:
    default:
        if (AFPSGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFPSGameMode>() : nullptr)
        {
            if (GameMode->IsGameOver()) RestartGame();
        }
        break;
    }
}

void AFPSPlayerController::ShowMenu(EFPSMenuScreen Screen, int32 FinalScore)
{
    if (Screen == EFPSMenuScreen::None) return;

    if (ActiveMenu)
    {
        ActiveMenu->RemoveFromParent();
        ActiveMenu = nullptr;
    }

    ActiveScreen = Screen;
    ActiveMenu = CreateWidget<UFPSMenuWidget>(this, UFPSMenuWidget::StaticClass());
    if (!ActiveMenu)
    {
        UE_LOG(LogFPSMenu, Error, TEXT("Failed to create the native menu widget."));
        ActiveScreen = EFPSMenuScreen::None;
        return;
    }

    ActiveMenu->Configure(Screen, FinalScore);
    ActiveMenu->AddToViewport(100);
    DisablePawnGameplayInput();

    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    FInputModeGameAndUI InputMode;
    InputMode.SetWidgetToFocus(ActiveMenu->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);
    FlushPressedKeys();
    SetPause(true);
}

void AFPSPlayerController::CloseMenuForGameplay()
{
    if (ActiveMenu)
    {
        ActiveMenu->RemoveFromParent();
        ActiveMenu = nullptr;
    }
    ActiveScreen = EFPSMenuScreen::None;

    FInputModeGameOnly InputMode;
    InputMode.SetConsumeCaptureMouseDown(true);
    SetInputMode(InputMode);
    FlushPressedKeys();
    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;
    if (APawn* ControlledPawn = GetPawn())
    {
        ControlledPawn->EnableInput(this);
    }
    SetPause(false);
}

void AFPSPlayerController::PrepareForTravel()
{
    if (ActiveMenu)
    {
        ActiveMenu->RemoveFromParent();
        ActiveMenu = nullptr;
    }
    ActiveScreen = EFPSMenuScreen::None;

    SetInputMode(FInputModeGameOnly());
    FlushPressedKeys();
    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;
    SetPause(false);
}

void AFPSPlayerController::DisablePawnGameplayInput()
{
    if (APawn* ControlledPawn = GetPawn())
    {
        if (AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(ControlledPawn))
        {
            FPSCharacter->CancelTransientInput();
        }
        ControlledPawn->DisableInput(this);
    }
}
