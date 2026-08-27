#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FPSHUD.generated.h"

UCLASS()
class FPSGAME_API AFPSHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

private:
    void DrawDamageFeedback(class AFPSCharacter* Character);
    void DrawCrosshair(const FVector2D& Center, class AFPSCharacter* Character);
    void DrawOpticReticle(const FVector2D& Center, class AFPSCharacter* Character);
    void DrawHitMarker(const FVector2D& Center, bool bKilledTarget);
    void DrawStatus(class AFPSCharacter* Character);
    void DrawPickupMessage(class AFPSCharacter* Character);
    void DrawMatchStatus(class AFPSGameMode* GameMode);
    void DrawDeathScreen(class AFPSCharacter* Character, class AFPSGameMode* GameMode);
};
