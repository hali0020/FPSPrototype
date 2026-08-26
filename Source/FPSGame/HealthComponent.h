#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHealthChangedSignature, float, Health, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDeathSignature);

UCLASS(ClassGroup=(FPS), meta=(BlueprintSpawnableComponent))
class FPSGAME_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

    UFUNCTION(BlueprintCallable, Category="Health")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category="Health")
    float Heal(float HealAmount);

    UFUNCTION(BlueprintPure, Category="Health") float GetHealth() const { return Health; }
    UFUNCTION(BlueprintPure, Category="Health") float GetMaxHealth() const { return MaxHealth; }
    UFUNCTION(BlueprintPure, Category="Health") bool IsDead() const { return Health <= 0.0f; }

    UPROPERTY(BlueprintAssignable, Category="Health") FHealthChangedSignature OnHealthChanged;
    UPROPERTY(BlueprintAssignable, Category="Health") FDeathSignature OnDeath;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health", meta=(ClampMin="1.0"))
    float MaxHealth = 100.0f;

private:
    UPROPERTY(VisibleInstanceOnly, Category="Health")
    float Health = 100.0f;
};
