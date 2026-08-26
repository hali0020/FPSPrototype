#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSImpactEffect.generated.h"

class UMaterialInterface;
class UPointLightComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class FPSGAME_API AFPSImpactEffect : public AActor
{
    GENERATED_BODY()

public:
    AFPSImpactEffect();
    virtual void Tick(float DeltaSeconds) override;

    void InitializeImpact(bool bInCharacterImpact);

protected:
    virtual void BeginPlay() override;

private:
    void SetMeshColor(UStaticMeshComponent* Mesh, const FLinearColor& Color) const;

    UPROPERTY(VisibleAnywhere, Category="Impact") TObjectPtr<USceneComponent> VisualRoot;
    UPROPERTY(VisibleAnywhere, Category="Impact") TObjectPtr<UStaticMeshComponent> ImpactCore;
    UPROPERTY(VisibleAnywhere, Category="Impact") TObjectPtr<UStaticMeshComponent> SparkA;
    UPROPERTY(VisibleAnywhere, Category="Impact") TObjectPtr<UStaticMeshComponent> SparkB;
    UPROPERTY(VisibleAnywhere, Category="Impact") TObjectPtr<UStaticMeshComponent> SparkC;
    UPROPERTY(VisibleAnywhere, Category="Impact") TObjectPtr<UPointLightComponent> ImpactLight;
    UPROPERTY() TObjectPtr<UMaterialInterface> ColorMaterial;

    UPROPERTY(EditDefaultsOnly, Category="Impact", meta=(ClampMin="0.05"))
    float ImpactDuration = 0.18f;
    UPROPERTY(EditDefaultsOnly, Category="Impact", meta=(ClampMin="0.0"))
    float InitialLightIntensity = 2200.0f;

    float Age = 0.0f;
    bool bCharacterImpact = false;
};
