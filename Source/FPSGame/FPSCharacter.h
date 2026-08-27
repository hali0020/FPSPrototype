#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSCharacter.generated.h"

class UCameraComponent;
class UAudioComponent;
class UHealthComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UAnimMontage;
class UAnimSequence;
class USoundBase;

UCLASS()
class FPSGAME_API AFPSCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AFPSCharacter();
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintPure, Category="Weapon") int32 GetAmmoInMagazine() const { return AmmoInMagazine; }
    UFUNCTION(BlueprintPure, Category="Weapon") int32 GetReserveAmmo() const { return ReserveAmmo; }
    UFUNCTION(BlueprintPure, Category="Weapon") int32 GetMagazineSize() const { return MagazineSize; }
    UFUNCTION(BlueprintPure, Category="Weapon") int32 GetMaxReserveAmmo() const { return MaxReserveAmmo; }
    UFUNCTION(BlueprintPure, Category="Weapon") bool IsReloading() const { return bIsReloading; }
    UFUNCTION(BlueprintPure, Category="Weapon") bool IsAiming() const { return bIsAiming; }
    UFUNCTION(BlueprintPure, Category="Weapon") float GetAimProgress() const;
    UFUNCTION(BlueprintPure, Category="Weapon") bool IsAimViewSettled() const;
    UFUNCTION(BlueprintPure, Category="Movement") bool IsSprinting() const { return bIsSprinting; }
    UFUNCTION(BlueprintPure, Category="Weapon") bool IsHitMarkerVisible() const;
    UFUNCTION(BlueprintPure, Category="Weapon") bool WasLastHitKill() const { return bLastHitWasKill; }
    UFUNCTION(BlueprintPure, Category="Weapon") float GetCurrentWeaponSpreadDegrees() const;
    UFUNCTION(BlueprintPure, Category="Health") UHealthComponent* GetHealthComponent() const { return HealthComponent; }
    UFUNCTION(BlueprintPure, Category="Health") bool IsDead() const { return bIsDead; }
    UFUNCTION(BlueprintPure, Category="Health") float GetDeathElapsedTime() const;
    UFUNCTION(BlueprintPure, Category="Feedback") float GetDamageFeedbackAlpha() const;

    UFUNCTION(BlueprintCallable, Category="Weapon") int32 AddReserveAmmo(int32 Amount);
    void ShowPickupMessage(const FString& Message);
    const FString& GetPickupMessage() const { return PickupMessage; }
    float GetPickupMessageAlpha() const;
    UAnimSequence* GetSprintPoseAnimation() const { return SprintAnimation; }
    float GetSprintPosePlayRate() const { return SprintAnimationPlayRate; }
    void CancelTransientInput();

protected:
    virtual void BeginPlay() override;

private:
    void MoveForward(float Value);
    void MoveRight(float Value);
    void StartFire();
    void StopFire();
    void FireShot();
    void StartReload();
    void FinishReload();
    void StartAim();
    void StopAim();
    void UpdateAim(float DeltaSeconds);
    void StartSprint();
    void StopSprint();
    void RefreshSprintState();
    void SetSprinting(bool bNewSprinting);
    bool IsActivelyFiring() const;
    void UpdateMovementSpeed();
    void UpdateShotBloom(float DeltaSeconds);
    void SetOpticVisible(bool bVisible);
    void PlayFireAnimation();
    void PlayReloadAnimation();
    void PlayDryFireFeedback();
    void PlayLocalFeedback(
        UAudioComponent* AudioComponent, USoundBase* Sound, float Volume, float Pitch);
    bool PlayPlayerVoice(USoundBase* Sound, bool bInterruptCurrent);
    void RegisterHit(bool bKilledTarget);
    void DisableMuzzleFlash();
    UFUNCTION() void HandleHealthChanged(float Health, float MaxHealth);
    UFUNCTION() void HandleDeath();

    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UCameraComponent> FirstPersonCamera;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UHealthComponent> HealthComponent;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<USkeletalMeshComponent> WeaponMesh;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UStaticMeshComponent> MuzzleFlashMesh;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UPointLightComponent> MuzzleFlash;
    UPROPERTY(VisibleAnywhere, Category="Components")
    TArray<TObjectPtr<UAudioComponent>> WeaponFireAudioPool;
    UPROPERTY(VisibleAnywhere, Category="Components")
    TArray<TObjectPtr<UStaticMeshComponent>> OpticFrameMeshes;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UAudioComponent> WeaponFeedbackAudio;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UAudioComponent> HitConfirmAudio;
    UPROPERTY(VisibleAnywhere, Category="Components") TObjectPtr<UAudioComponent> PlayerVoiceAudio;

    UPROPERTY() TObjectPtr<UAnimMontage> FireMontage;
    UPROPERTY() TObjectPtr<UAnimSequence> ReloadAnimation;
    UPROPERTY() TObjectPtr<UAnimSequence> DryFireAnimation;
    UPROPERTY() TObjectPtr<UAnimSequence> SprintAnimation;
    UPROPERTY() TObjectPtr<UAnimSequence> DeathAnimation;
    UPROPERTY() TObjectPtr<USoundBase> FireSound;
    UPROPERTY() TObjectPtr<USoundBase> EmptySound;
    UPROPERTY() TObjectPtr<USoundBase> ReloadSound;
    UPROPERTY() TObjectPtr<USoundBase> HitConfirmSound;
    UPROPERTY() TObjectPtr<USoundBase> PlayerHurtSound;
    UPROPERTY() TObjectPtr<USoundBase> PlayerDeathSound;

    UPROPERTY(EditDefaultsOnly, Category="Weapon") float Damage = 25.0f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon") float FireInterval = 0.12f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon") float WeaponRange = 15000.0f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon") int32 MagazineSize = 30;
    UPROPERTY(EditDefaultsOnly, Category="Weapon", meta=(ClampMin="0")) int32 MaxReserveAmmo = 180;
    UPROPERTY(EditDefaultsOnly, Category="Weapon") float ReloadDuration = 2.2f;
    UPROPERTY(EditDefaultsOnly, Category="Feedback") float HitMarkerDuration = 0.13f;
    UPROPERTY(EditDefaultsOnly, Category="Feedback") float KillMarkerDuration = 0.28f;
    UPROPERTY(EditDefaultsOnly, Category="Feedback", meta=(ClampMin="0.01"))
    float DamageFeedbackDuration = 0.38f;
    UPROPERTY(EditDefaultsOnly, Category="Feedback", meta=(ClampMin="0.0"))
    float HurtSoundCooldown = 0.45f;
    UPROPERTY(EditDefaultsOnly, Category="Feedback", meta=(ClampMin="0.1"))
    float PickupMessageDuration = 1.4f;
    UPROPERTY(EditDefaultsOnly, Category="Aim") float HipFOV = 90.0f;
    UPROPERTY(EditDefaultsOnly, Category="Aim") float AimFOV = 60.0f;
    UPROPERTY(EditDefaultsOnly, Category="Aim") float HipFirstPersonFOV = 88.0f;
    UPROPERTY(EditDefaultsOnly, Category="Aim") float AimFirstPersonFOV = 82.0f;
    UPROPERTY(EditDefaultsOnly, Category="Aim")
    FVector HipCameraRelativeLocation = FVector(13.5f, 5.89f, -2.0f);
    UPROPERTY(EditDefaultsOnly, Category="Aim")
    FVector AimCameraRelativeLocation = FVector(13.0f, 8.0f, 5.5f);
    UPROPERTY(EditDefaultsOnly, Category="Movement|Sprint") float SprintFOV = 96.0f;
    UPROPERTY(EditDefaultsOnly, Category="Aim") float AimInterpSpeed = 12.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Sprint", meta=(ClampMin="0.0"))
    float SprintSpeed = 900.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Sprint", meta=(ClampMin="0.0", ClampMax="1.0"))
    float SprintForwardThreshold = 0.2f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Sprint", meta=(ClampMin="0.1"))
    float SprintAnimationPlayRate = 1.15f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Combat", meta=(ClampMin="0.1", ClampMax="1.0"))
    float FiringMoveSpeedMultiplier = 0.65f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread", meta=(ClampMin="0.0"))
    float HipBaseSpreadDegrees = 0.35f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread", meta=(ClampMin="0.0"))
    float AimBaseSpreadDegrees = 0.08f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread", meta=(ClampMin="0.0"))
    float MovingSpreadDegrees = 1.15f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread", meta=(ClampMin="0.0"))
    float AirSpreadDegrees = 2.0f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread", meta=(ClampMin="0.0", ClampMax="1.0"))
    float AimMoveSpreadMultiplier = 0.35f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread", meta=(ClampMin="0.0"))
    float ShotBloomPerShot = 0.18f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread", meta=(ClampMin="0.0"))
    float MaxShotBloomDegrees = 1.35f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread", meta=(ClampMin="0.0", ClampMax="1.0"))
    float AimBloomMultiplier = 0.45f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread", meta=(ClampMin="0.0"))
    float SpreadRecoveryDelay = 0.18f;
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread", meta=(ClampMin="0.0"))
    float SpreadRecoveryRate = 2.6f;

    int32 AmmoInMagazine = 30;
    int32 ReserveAmmo = 90;
    bool bIsReloading = false;
    bool bIsAiming = false;
    bool bIsDead = false;
    bool bWantsToFire = false;
    bool bSprintHeld = false;
    bool bIsSprinting = false;
    bool bLastHitWasKill = false;
    float ForwardInputValue = 0.0f;
    float NormalWalkSpeed = 0.0f;
    float HitMarkerEndTime = 0.0f;
    float DamageFeedbackEndTime = 0.0f;
    float NextHurtSoundTime = 0.0f;
    float LastKnownHealth = 0.0f;
    float DeathStartTime = 0.0f;
    float PickupMessageEndTime = 0.0f;
    float CurrentShotBloomDegrees = 0.0f;
    float LastShotTime = -BIG_NUMBER;
    int32 NextFireAudioVoice = 0;
    FString PickupMessage;
    FTimerHandle FireTimer;
    FTimerHandle ReloadTimer;
    FTimerHandle MuzzleFlashTimer;
};
