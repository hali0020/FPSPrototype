#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSDeathEffect.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPointLightComponent;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Lightweight native death burst used by enemies.
 *
 * The effect owns a fixed set of mesh components and initializes all runtime
 * state once in BeginPlay. Tick only moves and scales those existing parts, so
 * automatic combat cannot create per-frame particles or material instances.
 */
UCLASS(NotBlueprintable)
class FPSGAME_API AFPSDeathEffect : public AActor
{
    GENERATED_BODY()

public:
    AFPSDeathEffect();
    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Death Effect")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category="Death Effect")
    TArray<TObjectPtr<UStaticMeshComponent>> BurstParts;

    UPROPERTY(VisibleAnywhere, Category="Death Effect")
    TObjectPtr<UStaticMeshComponent> CoreFlash;

    UPROPERTY(VisibleAnywhere, Category="Death Effect")
    TArray<TObjectPtr<UStaticMeshComponent>> ShockwaveSegments;

    UPROPERTY(VisibleAnywhere, Category="Death Effect")
    TObjectPtr<UPointLightComponent> BurstLight;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> ParticleBaseMaterial;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UMaterialInstanceDynamic>> PaletteMaterials;

    UPROPERTY(EditDefaultsOnly, Category="Death Effect", meta=(ClampMin="0.1"))
    float EffectLifetime = 1.05f;

    UPROPERTY(EditDefaultsOnly, Category="Death Effect", meta=(ClampMin="0.0"))
    float Gravity = 620.0f;

    UPROPERTY(EditDefaultsOnly, Category="Death Effect", meta=(ClampMin="0.0"))
    float Drag = 1.6f;

    UPROPERTY(EditDefaultsOnly, Category="Death Effect", meta=(ClampMin="0.0"))
    float InitialLightIntensity = 5600.0f;

    TArray<FVector> PartVelocities;
    TArray<FRotator> PartRotationRates;
    TArray<FVector> PartInitialScales;
    float ElapsedTime = 0.0f;
};
