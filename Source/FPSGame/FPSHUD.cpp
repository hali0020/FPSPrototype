#include "FPSHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "FPSCharacter.h"
#include "FPSGameMode.h"
#include "HealthComponent.h"

void AFPSHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;
    AFPSCharacter* Character = Cast<AFPSCharacter>(GetOwningPawn());
    AFPSGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFPSGameMode>() : nullptr;
    if (Character && !Character->IsDead())
    {
        DrawDamageFeedback(Character);
        DrawCrosshair(FVector2D(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f), Character);
        DrawStatus(Character);
        DrawPickupMessage(Character);
    }
    DrawMatchStatus(GameMode);
    if (Character && Character->IsDead())
    {
        DrawDeathScreen(Character, GameMode);
    }
}

void AFPSHUD::DrawDamageFeedback(AFPSCharacter* Character)
{
    if (!Character || !Canvas) return;
    const float Alpha = Character->GetDamageFeedbackAlpha();
    if (Alpha <= 0.0f) return;

    const float Strength = Alpha * Alpha;
    const float Border = FMath::Lerp(24.0f, 72.0f, Strength);
    const FLinearColor BorderColor(0.72f, 0.0f, 0.0f, 0.52f * Strength);
    DrawRect(FLinearColor(0.35f, 0.0f, 0.0f, 0.09f * Strength),
        0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY);
    DrawRect(BorderColor, 0.0f, 0.0f, Canvas->ClipX, Border);
    DrawRect(BorderColor, 0.0f, Canvas->ClipY - Border, Canvas->ClipX, Border);
    DrawRect(BorderColor, 0.0f, Border, Border, Canvas->ClipY - Border * 2.0f);
    DrawRect(BorderColor, Canvas->ClipX - Border, Border, Border, Canvas->ClipY - Border * 2.0f);
}

void AFPSHUD::DrawCrosshair(const FVector2D& Center, AFPSCharacter* Character)
{
    const float Gap = 5.0f;
    const float Length = 9.0f;
    DrawLine(Center.X - Gap - Length, Center.Y, Center.X - Gap, Center.Y, FLinearColor::White, 2.0f);
    DrawLine(Center.X + Gap, Center.Y, Center.X + Gap + Length, Center.Y, FLinearColor::White, 2.0f);
    DrawLine(Center.X, Center.Y - Gap - Length, Center.X, Center.Y - Gap, FLinearColor::White, 2.0f);
    DrawLine(Center.X, Center.Y + Gap, Center.X, Center.Y + Gap + Length, FLinearColor::White, 2.0f);
    if (Character && Character->IsHitMarkerVisible())
    {
        DrawHitMarker(Center, Character->WasLastHitKill());
    }
}

void AFPSHUD::DrawHitMarker(const FVector2D& Center, bool bKilledTarget)
{
    const float Gap = 7.0f;
    const float Length = bKilledTarget ? 12.0f : 9.0f;
    const FLinearColor Color = bKilledTarget
        ? FLinearColor(1.0f, 0.05f, 0.02f)
        : FLinearColor(0.95f, 0.95f, 0.95f);
    const float Thickness = bKilledTarget ? 3.5f : 2.5f;

    DrawLine(Center.X - Gap - Length, Center.Y - Gap - Length,
        Center.X - Gap, Center.Y - Gap, Color, Thickness);
    DrawLine(Center.X + Gap, Center.Y + Gap,
        Center.X + Gap + Length, Center.Y + Gap + Length, Color, Thickness);
    DrawLine(Center.X + Gap, Center.Y - Gap,
        Center.X + Gap + Length, Center.Y - Gap - Length, Color, Thickness);
    DrawLine(Center.X - Gap - Length, Center.Y + Gap + Length,
        Center.X - Gap, Center.Y + Gap, Color, Thickness);
}

void AFPSHUD::DrawStatus(AFPSCharacter* Character)
{
    if (!Character || !GEngine) return;
    UHealthComponent* Health = Character->GetHealthComponent();
    const float HealthPercent = Health ? Health->GetHealth() / Health->GetMaxHealth() : 0.0f;
    const float X = 40.0f;
    const float Y = Canvas->ClipY - 80.0f;
    DrawRect(FLinearColor(0.05f, 0.05f, 0.05f, 0.8f), X, Y, 220.0f, 20.0f);
    DrawRect(FLinearColor::Red, X + 2.0f, Y + 2.0f, 216.0f * HealthPercent, 16.0f);
    DrawText(FString::Printf(TEXT("HP %.0f"), Health ? Health->GetHealth() : 0.0f), FLinearColor::White, X, Y - 24.0f, GEngine->GetSmallFont());

    const FString Ammo = Character->IsReloading()
        ? TEXT("RELOADING...")
        : FString::Printf(TEXT("%d / %d"), Character->GetAmmoInMagazine(), Character->GetReserveAmmo());
    DrawText(Ammo, FLinearColor::White, Canvas->ClipX - 190.0f, Canvas->ClipY - 80.0f, GEngine->GetLargeFont());
}

void AFPSHUD::DrawPickupMessage(AFPSCharacter* Character)
{
    if (!Character || !Canvas || !GEngine) return;
    const float Alpha = Character->GetPickupMessageAlpha();
    if (Alpha <= 0.0f) return;

    const FString& Message = Character->GetPickupMessage();
    UFont* Font = GEngine->GetSmallFont();
    constexpr float Scale = 1.15f;
    float TextWidth = 0.0f;
    float TextHeight = 0.0f;
    Canvas->StrLen(Font, Message, TextWidth, TextHeight);
    TextWidth *= Scale;
    TextHeight *= Scale;

    const float X = Canvas->ClipX * 0.5f - TextWidth * 0.5f;
    const float Y = Canvas->ClipY * 0.70f;
    DrawRect(FLinearColor(0.01f, 0.025f, 0.03f, 0.72f * Alpha),
        X - 14.0f, Y - 7.0f, TextWidth + 28.0f, TextHeight + 14.0f);
    DrawText(Message, FLinearColor(0.35f, 1.0f, 0.68f, Alpha), X, Y, Font, Scale);
}

void AFPSHUD::DrawMatchStatus(AFPSGameMode* GameMode)
{
    if (!GameMode || !GEngine) return;

    DrawText(FString::Printf(TEXT("WAVE %d"), GameMode->GetCurrentWave()),
        FLinearColor::White, 40.0f, 35.0f, GEngine->GetLargeFont());
    DrawText(FString::Printf(TEXT("ENEMIES %d"), GameMode->GetEnemiesRemaining()),
        FLinearColor::White, 40.0f, 65.0f, GEngine->GetSmallFont());
    DrawText(FString::Printf(TEXT("SCORE %06d"), GameMode->GetScore()),
        FLinearColor(1.0f, 0.82f, 0.1f), 40.0f, 90.0f, GEngine->GetSmallFont());

}

void AFPSHUD::DrawDeathScreen(AFPSCharacter* Character, AFPSGameMode* GameMode)
{
    if (!Character || !GEngine || !Canvas) return;

    const float Elapsed = Character->GetDeathElapsedTime();
    const float FadeLinear = FMath::Clamp((Elapsed - 0.35f) / 1.0f, 0.0f, 1.0f);
    const float Fade = FadeLinear * FadeLinear * (3.0f - 2.0f * FadeLinear);
    const float RedFlash = FMath::Clamp(1.0f - Elapsed / 0.7f, 0.0f, 1.0f);

    DrawRect(FLinearColor(0.24f, 0.0f, 0.0f, 0.16f + RedFlash * 0.24f),
        0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY);

    const float Border = FMath::Lerp(70.0f, 28.0f, Fade);
    const FLinearColor BorderColor(0.55f, 0.0f, 0.0f, 0.30f + RedFlash * 0.35f);
    DrawRect(BorderColor, 0.0f, 0.0f, Canvas->ClipX, Border);
    DrawRect(BorderColor, 0.0f, Canvas->ClipY - Border, Canvas->ClipX, Border);
    DrawRect(BorderColor, 0.0f, Border, Border, Canvas->ClipY - Border * 2.0f);
    DrawRect(BorderColor, Canvas->ClipX - Border, Border, Border, Canvas->ClipY - Border * 2.0f);

    DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, Fade * 0.78f),
        0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY);

    if (Fade <= 0.05f) return;

    const float CenterX = Canvas->ClipX * 0.5f;
    const float CenterY = Canvas->ClipY * 0.5f;
    const float TextAlpha = FMath::Clamp((Fade - 0.1f) / 0.9f, 0.0f, 1.0f);

    auto DrawCentered = [this, CenterX](
        const FString& Text, UFont* Font, float Y, float Scale, const FLinearColor& Color)
    {
        float Width = 0.0f;
        float Height = 0.0f;
        Canvas->StrLen(Font, Text, Width, Height);
        DrawText(Text, Color, CenterX - Width * Scale * 0.5f, Y, Font, Scale);
    };

    DrawCentered(TEXT("YOU DIED"), GEngine->GetLargeFont(), CenterY - 90.0f, 1.8f,
        FLinearColor(0.95f, 0.03f, 0.02f, TextAlpha));

    const FString ScoreText = FString::Printf(
        TEXT("FINAL SCORE: %d"), GameMode ? GameMode->GetScore() : 0);
    DrawCentered(ScoreText, GEngine->GetSmallFont(), CenterY + 5.0f, 1.2f,
        FLinearColor(1.0f, 1.0f, 1.0f, TextAlpha));

    if (Elapsed > 1.0f)
    {
        const float Pulse = 0.70f + 0.30f * FMath::Sin(Elapsed * 4.0f);
        DrawCentered(TEXT("PRESS ENTER TO RESTART"), GEngine->GetSmallFont(),
            CenterY + 55.0f, 1.0f, FLinearColor(1.0f, 1.0f, 1.0f, TextAlpha * Pulse));
    }
}
