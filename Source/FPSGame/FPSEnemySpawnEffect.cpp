#include "FPSEnemySpawnEffect.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
constexpr int32 SpawnRingSegmentCount = 6;
}

AFPSEnemySpawnEffect::AFPSEnemySpawnEffect()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    bReplicates = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
        TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterial(
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    if (BaseMaterial.Succeeded())
    {
        ParticleBaseMaterial = BaseMaterial.Object;
    }

    ArrivalBeam = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrivalBeam"));
    ArrivalBeam->SetupAttachment(SceneRoot);
    ArrivalBeam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ArrivalBeam->SetGenerateOverlapEvents(false);
    ArrivalBeam->SetCanEverAffectNavigation(false);
    ArrivalBeam->SetCastShadow(false);
    ArrivalBeam->SetReceivesDecals(false);
    ArrivalBeam->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::None);
    ArrivalBeam->SetVisibility(false);
    if (CylinderMesh.Succeeded())
    {
        ArrivalBeam->SetStaticMesh(CylinderMesh.Object);
    }
    else if (CubeMesh.Succeeded())
    {
        ArrivalBeam->SetStaticMesh(CubeMesh.Object);
    }

    RingSegments.Reserve(SpawnRingSegmentCount);
    for (int32 SegmentIndex = 0; SegmentIndex < SpawnRingSegmentCount; ++SegmentIndex)
    {
        const FName SegmentName(*FString::Printf(TEXT("ArrivalRing_%02d"), SegmentIndex));
        UStaticMeshComponent* Segment = CreateDefaultSubobject<UStaticMeshComponent>(SegmentName);
        Segment->SetupAttachment(SceneRoot);
        Segment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Segment->SetGenerateOverlapEvents(false);
        Segment->SetCanEverAffectNavigation(false);
        Segment->SetCastShadow(false);
        Segment->SetReceivesDecals(false);
        Segment->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::None);
        Segment->SetVisibility(false);
        if (CubeMesh.Succeeded())
        {
            Segment->SetStaticMesh(CubeMesh.Object);
        }
        RingSegments.Add(Segment);
    }

}

void AFPSEnemySpawnEffect::BeginPlay()
{
    Super::BeginPlay();

    if (!ParticleBaseMaterial || !ArrivalBeam || RingSegments.IsEmpty()
        || EffectLifetime <= 0.0f)
    {
        Destroy();
        return;
    }

    BeamMaterial = UMaterialInstanceDynamic::Create(ParticleBaseMaterial, this);
    RingMaterial = UMaterialInstanceDynamic::Create(ParticleBaseMaterial, this);
    if (!BeamMaterial || !RingMaterial)
    {
        Destroy();
        return;
    }

    BeamMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.025f, 0.55f, 1.0f, 1.0f));
    RingMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.08f, 0.92f, 1.0f, 1.0f));
    BeamMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.22f);
    RingMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.18f);

    ArrivalBeam->SetMaterial(0, BeamMaterial);
    ArrivalBeam->SetRelativeScale3D(FVector(0.08f, 0.08f, 0.35f));
    ArrivalBeam->SetRelativeLocation(FVector(0.0f, 0.0f, 17.5f));
    ArrivalBeam->SetVisibility(true);

    for (int32 SegmentIndex = 0; SegmentIndex < RingSegments.Num(); ++SegmentIndex)
    {
        UStaticMeshComponent* Segment = RingSegments[SegmentIndex];
        if (!Segment) continue;

        const float Angle = UE_TWO_PI * static_cast<float>(SegmentIndex)
            / static_cast<float>(RingSegments.Num());
        Segment->SetMaterial(0, RingMaterial);
        Segment->SetRelativeLocation(FVector(
            FMath::Cos(Angle) * 24.0f,
            FMath::Sin(Angle) * 24.0f,
            5.0f));
        Segment->SetRelativeRotation(FRotator(
            0.0f, FMath::RadiansToDegrees(Angle) + 90.0f, 0.0f));
        Segment->SetRelativeScale3D(FVector(0.028f, 0.22f, 0.022f));
        Segment->SetVisibility(true);
    }

    ElapsedTime = 0.0f;
    SetLifeSpan(EffectLifetime + 0.15f);
}

void AFPSEnemySpawnEffect::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    ElapsedTime += DeltaSeconds;
    const float LifeAlpha = FMath::Clamp(
        ElapsedTime / FMath::Max(EffectLifetime, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
    const float RemainingAlpha = 1.0f - LifeAlpha;
    const float SmoothAlpha = LifeAlpha * LifeAlpha * (3.0f - 2.0f * LifeAlpha);
    const float EaseOutAlpha = 1.0f - FMath::Pow(RemainingAlpha, 3.0f);

    if (ArrivalBeam)
    {
        const float RiseAlpha = FMath::Clamp(LifeAlpha / 0.28f, 0.0f, 1.0f);
        const float SmoothRise = RiseAlpha * RiseAlpha * (3.0f - 2.0f * RiseAlpha);
        const float BeamHeight = FMath::Lerp(0.35f, 3.15f, SmoothRise);
        const float BeamWidth = 0.085f * FMath::Max(0.08f, RemainingAlpha);
        ArrivalBeam->SetRelativeScale3D(FVector(BeamWidth, BeamWidth, BeamHeight));
        // The Engine cylinder is 100 cm high; moving by half its scaled height
        // keeps the base of the column planted on the arrival point.
        ArrivalBeam->SetRelativeLocation(FVector(0.0f, 0.0f, BeamHeight * 50.0f));
    }

    const float RingRadius = FMath::Lerp(24.0f, 175.0f, EaseOutAlpha);
    const float RingScale = FMath::Pow(RemainingAlpha, 0.62f);
    for (int32 SegmentIndex = 0; SegmentIndex < RingSegments.Num(); ++SegmentIndex)
    {
        UStaticMeshComponent* Segment = RingSegments[SegmentIndex];
        if (!Segment) continue;

        const float Angle = UE_TWO_PI * static_cast<float>(SegmentIndex)
            / static_cast<float>(RingSegments.Num());
        Segment->SetRelativeLocation(FVector(
            FMath::Cos(Angle) * RingRadius,
            FMath::Sin(Angle) * RingRadius,
            FMath::Lerp(5.0f, 22.0f, SmoothAlpha)));
        Segment->SetRelativeRotation(FRotator(0.0f, FMath::RadiansToDegrees(Angle) + 90.0f, 0.0f));
        Segment->SetRelativeScale3D(FVector(
            0.028f * RingScale, 0.22f * RingScale, 0.022f * RingScale));
    }

    if (LifeAlpha >= 1.0f)
    {
        if (ArrivalBeam) ArrivalBeam->SetVisibility(false);
        for (UStaticMeshComponent* Segment : RingSegments)
        {
            if (Segment) Segment->SetVisibility(false);
        }
        SetActorTickEnabled(false);
        Destroy();
    }
}
