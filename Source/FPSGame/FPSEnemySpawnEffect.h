#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSEnemySpawnEffect.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Lightweight native arrival marker shown when an enemy enters a wave.
 *
 * The beam and ring are a fixed set of components. Materials are initialized
 * once in BeginPlay; Tick only updates those existing objects, avoiding
 * Niagara, dynamic lights and per-frame allocations when a whole wave spawns.
 */
UCLASS(NotBlueprintable)
class FPSGAME_API AFPSEnemySpawnEffect : public AActor
{
    GENERATED_BODY()

public:
    AFPSEnemySpawnEffect();
    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Spawn Effect")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category="Spawn Effect")
    TObjectPtr<UStaticMeshComponent> ArrivalBeam;

    UPROPERTY(VisibleAnywhere, Category="Spawn Effect")
    TArray<TObjectPtr<UStaticMeshComponent>> RingSegments;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> ParticleBaseMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> BeamMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> RingMaterial;

    UPROPERTY(EditDefaultsOnly, Category="Spawn Effect", meta=(ClampMin="0.1"))
    float EffectLifetime = 0.72f;

    float ElapsedTime = 0.0f;
};
