#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;
}

void UHealthComponent::ApplyDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f || IsDead()) return;

    Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(Health, MaxHealth);
    if (IsDead()) OnDeath.Broadcast();
}

void UHealthComponent::SetMaxHealth(float NewMaxHealth, bool bRefill)
{
    MaxHealth = FMath::Max(1.0f, NewMaxHealth);
    Health = bRefill ? MaxHealth : FMath::Clamp(Health, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(Health, MaxHealth);
}

float UHealthComponent::Heal(float HealAmount)
{
    if (HealAmount <= 0.0f || IsDead() || Health >= MaxHealth) return 0.0f;

    const float PreviousHealth = Health;
    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);
    const float ActualHeal = Health - PreviousHealth;
    if (ActualHeal > 0.0f)
    {
        OnHealthChanged.Broadcast(Health, MaxHealth);
    }
    return ActualHeal;
}
