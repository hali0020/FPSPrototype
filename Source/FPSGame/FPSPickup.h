#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSPickup.generated.h"

class AFPSCharacter;
class UMaterialInterface;
class UPointLightComponent;
class UPrimitiveComponent;
class USceneComponent;
class USoundBase;
class USphereComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EFPSPickupType : uint8
{
    Ammo,
    Health,
    Supply
};

UCLASS()
class FPSGAME_API AFPSPickup : public AActor
{
    GENERATED_BODY()

public:
    AFPSPickup();
    virtual void Tick(float DeltaSeconds) override;

    void InitializePickup(
        EFPSPickupType InType, int32 InAmmoAmount, float InHealAmount, float InLifetimeSeconds);

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void HandleOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComponent,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    void ApplyVisualStyle();
    USoundBase* GetPickupSound() const;
    void SetMeshColor(UStaticMeshComponent* Mesh, const FLinearColor& Color) const;

    UPROPERTY(VisibleAnywhere, Category="Pickup") TObjectPtr<USphereComponent> PickupSphere;
    UPROPERTY(VisibleAnywhere, Category="Pickup") TObjectPtr<USceneComponent> VisualRoot;
    UPROPERTY(VisibleAnywhere, Category="Pickup") TObjectPtr<UStaticMeshComponent> CaseBase;
    UPROPERTY(VisibleAnywhere, Category="Pickup") TObjectPtr<UStaticMeshComponent> CaseLid;
    UPROPERTY(VisibleAnywhere, Category="Pickup") TObjectPtr<UStaticMeshComponent> CrossLong;
    UPROPERTY(VisibleAnywhere, Category="Pickup") TObjectPtr<UStaticMeshComponent> CrossShort;
    UPROPERTY(VisibleAnywhere, Category="Pickup") TObjectPtr<UStaticMeshComponent> AmmoRoundA;
    UPROPERTY(VisibleAnywhere, Category="Pickup") TObjectPtr<UStaticMeshComponent> AmmoRoundB;
    UPROPERTY(VisibleAnywhere, Category="Pickup") TObjectPtr<UStaticMeshComponent> AmmoRoundC;
    UPROPERTY(VisibleAnywhere, Category="Pickup") TObjectPtr<UPointLightComponent> AccentLight;

    UPROPERTY() TObjectPtr<UMaterialInterface> ColorMaterial;
    UPROPERTY() TObjectPtr<USoundBase> AmmoPickupSound;
    UPROPERTY() TObjectPtr<USoundBase> HealthPickupSound;
    UPROPERTY() TObjectPtr<USoundBase> SupplyPickupSound;

    UPROPERTY(EditAnywhere, Category="Pickup") EFPSPickupType PickupType = EFPSPickupType::Ammo;
    UPROPERTY(EditAnywhere, Category="Pickup", meta=(ClampMin="0")) int32 AmmoAmount = 30;
    UPROPERTY(EditAnywhere, Category="Pickup", meta=(ClampMin="0.0")) float HealAmount = 25.0f;
    UPROPERTY(EditAnywhere, Category="Pickup", meta=(ClampMin="0.0")) float LifetimeSeconds = 0.0f;
    UPROPERTY(EditDefaultsOnly, Category="Pickup") float BobAmplitude = 3.0f;
    UPROPERTY(EditDefaultsOnly, Category="Pickup") float BobSpeed = 2.2f;
    UPROPERTY(EditDefaultsOnly, Category="Pickup") float SpinSpeed = 28.0f;

    FVector RestLocation = FVector::ZeroVector;
    float BobPhase = 0.0f;
    bool bConsumed = false;
};
