#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSEnemy.generated.h"

class UHealthComponent;
class UAnimSequence;
class UAudioComponent;
class USkeletalMesh;
class USoundBase;
class AFPSEnemy;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyDiedSignature, AFPSEnemy*, Enemy);

UCLASS()
class FPSGAME_API AFPSEnemy : public ACharacter
{
    GENERATED_BODY()

public:
    AFPSEnemy();
    virtual void Tick(float DeltaSeconds) override;
    void SetHeavyVariant(bool bHeavy);

    UPROPERTY(BlueprintAssignable, Category="Combat") FEnemyDiedSignature OnEnemyDied;

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION() void HandleDeath();
    UFUNCTION() void HandleHealthChanged(float Health, float MaxHealth);
    void ConfigureVariant();
    void TryAttack();
    void ApplyMeleeDamage();
    USoundBase* PlayRandomVoice(
        const TArray<TObjectPtr<USoundBase>>& Sounds, bool bInterruptCurrent);

    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UHealthComponent> HealthComponent;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UAudioComponent> VoiceAudio;
    UPROPERTY() TObjectPtr<USkeletalMesh> QuinnMesh;
    UPROPERTY() TObjectPtr<USkeletalMesh> MannyMesh;
    UPROPERTY() TObjectPtr<UAnimSequence> AttackAnimation;
    UPROPERTY() TObjectPtr<UAnimSequence> HitReactAnimation;
    UPROPERTY() TObjectPtr<UAnimSequence> DeathAnimation;
    UPROPERTY() TArray<TObjectPtr<UAnimSequence>> AttackAnimations;
    UPROPERTY() TArray<TObjectPtr<UAnimSequence>> HitReactAnimations;
    UPROPERTY() TArray<TObjectPtr<UAnimSequence>> DeathAnimations;
    UPROPERTY() TArray<TObjectPtr<USoundBase>> QuinnAlertSounds;
    UPROPERTY() TArray<TObjectPtr<USoundBase>> QuinnAttackSounds;
    UPROPERTY() TArray<TObjectPtr<USoundBase>> QuinnHurtSounds;
    UPROPERTY() TArray<TObjectPtr<USoundBase>> QuinnDeathSounds;
    UPROPERTY() TArray<TObjectPtr<USoundBase>> MannyHurtSounds;
    UPROPERTY() TArray<TObjectPtr<USoundBase>> MannyDeathSounds;
    UPROPERTY() TArray<TObjectPtr<USoundBase>> AlertSounds;
    UPROPERTY() TArray<TObjectPtr<USoundBase>> AttackSounds;
    UPROPERTY() TArray<TObjectPtr<USoundBase>> HurtSounds;
    UPROPERTY() TArray<TObjectPtr<USoundBase>> DeathSounds;
    UPROPERTY(EditDefaultsOnly, Category="Combat") float DetectionRange = 5000.0f;
    UPROPERTY(EditDefaultsOnly, Category="Combat") float AttackRange = 150.0f;
    UPROPERTY(EditDefaultsOnly, Category="Combat") float AttackDamage = 10.0f;
    UPROPERTY(EditDefaultsOnly, Category="Combat") float AttackCooldown = 1.0f;
    UPROPERTY(EditDefaultsOnly, Category="Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
    float AlertVoiceChance = 0.35f;
    UPROPERTY(EditDefaultsOnly, Category="Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
    float AttackVoiceChance = 0.4f;
    UPROPERTY(EditDefaultsOnly, Category="Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
    float HurtVoiceChance = 0.85f;
    UPROPERTY(EditDefaultsOnly, Category="Audio", meta=(ClampMin="0.0"))
    float HurtVoiceCooldown = 0.55f;
    float TimeUntilNextAttack = 0.0f;
    float NextHurtVoiceTime = 0.0f;
    float AvoidanceSign = 1.0f;
    bool bHasVariantOverride = false;
    bool bRequestedHeavyVariant = false;
    bool bIsHeavyVariant = false;
    bool bHasDetectedPlayer = false;
    bool bIsDead = false;
    FTimerHandle AttackDamageTimer;
};
