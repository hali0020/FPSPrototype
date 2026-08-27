#include "FPSDeathEffect.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
constexpr int32 DeathBurstPartCount = 14;
constexpr int32 DeathShockwaveSegmentCount = 12;
}

AFPSDeathEffect::AFPSDeathEffect()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    bReplicates = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterial(
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    if (BaseMaterial.Succeeded())
    {
        ParticleBaseMaterial = BaseMaterial.Object;
    }

    BurstParts.Reserve(DeathBurstPartCount);
    for (int32 PartIndex = 0; PartIndex < DeathBurstPartCount; ++PartIndex)
    {
        const FName PartName(*FString::Printf(TEXT("DeathBurstPart_%02d"), PartIndex));
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(PartName);
        Part->SetupAttachment(SceneRoot);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetGenerateOverlapEvents(false);
        Part->SetCanEverAffectNavigation(false);
        Part->SetCastShadow(false);
        Part->SetReceivesDecals(false);
        Part->SetOnlyOwnerSee(false);
        Part->SetOwnerNoSee(false);
        // This is a world effect, not an owner-only weapon primitive. None keeps
        // it visible to the player who caused the death as well as other views.
        Part->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::None);
        Part->SetVisibility(false);

        if ((PartIndex % 3) == 0 && CubeMesh.Succeeded())
        {
            Part->SetStaticMesh(CubeMesh.Object);
        }
        else if (SphereMesh.Succeeded())
        {
            Part->SetStaticMesh(SphereMesh.Object);
        }
        else if (CubeMesh.Succeeded())
        {
            Part->SetStaticMesh(CubeMesh.Object);
        }

        BurstParts.Add(Part);
    }

    CoreFlash = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreFlash"));
    CoreFlash->SetupAttachment(SceneRoot);
    CoreFlash->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CoreFlash->SetGenerateOverlapEvents(false);
    CoreFlash->SetCanEverAffectNavigation(false);
    CoreFlash->SetCastShadow(false);
    CoreFlash->SetReceivesDecals(false);
    CoreFlash->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::None);
    CoreFlash->SetVisibility(false);
    if (SphereMesh.Succeeded())
    {
        CoreFlash->SetStaticMesh(SphereMesh.Object);
    }

    ShockwaveSegments.Reserve(DeathShockwaveSegmentCount);
    for (int32 SegmentIndex = 0; SegmentIndex < DeathShockwaveSegmentCount; ++SegmentIndex)
    {
        const FName SegmentName(*FString::Printf(TEXT("DeathShockwave_%02d"), SegmentIndex));
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
        ShockwaveSegments.Add(Segment);
    }

    BurstLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BurstLight"));
    BurstLight->SetupAttachment(SceneRoot);
    BurstLight->SetLightColor(FLinearColor(1.0f, 0.24f, 0.035f));
    BurstLight->SetAttenuationRadius(520.0f);
    BurstLight->SetIntensity(0.0f);
    BurstLight->SetCastShadows(false);
    BurstLight->SetVisibility(false);
}

void AFPSDeathEffect::BeginPlay()
{
    Super::BeginPlay();

    if (BurstParts.IsEmpty() || !ParticleBaseMaterial || EffectLifetime <= 0.0f)
    {
        Destroy();
        return;
    }

    static const FLinearColor Palette[] =
    {
        FLinearColor(0.012f, 0.018f, 0.025f, 1.0f),
        FLinearColor(1.0f, 0.08f, 0.015f, 1.0f),
        FLinearColor(1.0f, 0.52f, 0.035f, 1.0f),
        FLinearColor(0.04f, 0.48f, 1.0f, 1.0f),
        FLinearColor(1.0f, 0.86f, 0.38f, 1.0f)
    };

    PaletteMaterials.Reserve(UE_ARRAY_COUNT(Palette));
    for (const FLinearColor& Color : Palette)
    {
        UMaterialInstanceDynamic* Material =
            UMaterialInstanceDynamic::Create(ParticleBaseMaterial, this);
        if (!Material) continue;

        Material->SetVectorParameterValue(TEXT("Color"), Color);
        Material->SetScalarParameterValue(TEXT("Roughness"), 0.42f);
        PaletteMaterials.Add(Material);
    }

    if (CoreFlash && !PaletteMaterials.IsEmpty())
    {
        CoreFlash->SetMaterial(0, PaletteMaterials.Last());
        CoreFlash->SetRelativeScale3D(FVector(0.055f));
        CoreFlash->SetVisibility(true);
    }

    for (int32 SegmentIndex = 0; SegmentIndex < ShockwaveSegments.Num(); ++SegmentIndex)
    {
        UStaticMeshComponent* Segment = ShockwaveSegments[SegmentIndex];
        if (!Segment) continue;

        if (!PaletteMaterials.IsEmpty())
        {
            const int32 ColorIndex = FMath::Min(
                1 + SegmentIndex % 2, PaletteMaterials.Num() - 1);
            Segment->SetMaterial(0, PaletteMaterials[ColorIndex]);
        }
        const float Angle = UE_TWO_PI * static_cast<float>(SegmentIndex)
            / static_cast<float>(ShockwaveSegments.Num());
        Segment->SetRelativeLocation(FVector(
            FMath::Cos(Angle) * 30.0f,
            FMath::Sin(Angle) * 30.0f,
            -4.0f));
        Segment->SetRelativeRotation(FRotator(
            0.0f, FMath::RadiansToDegrees(Angle) + 90.0f, 0.0f));
        Segment->SetRelativeScale3D(FVector(0.035f, 0.24f, 0.025f));
        Segment->SetVisibility(true);
    }

    PartVelocities.SetNumUninitialized(BurstParts.Num());
    PartRotationRates.SetNumUninitialized(BurstParts.Num());
    PartInitialScales.SetNumUninitialized(BurstParts.Num());

    const uint32 LocationSeed = GetTypeHash(GetActorLocation());
    FRandomStream RandomStream(static_cast<int32>(LocationSeed ^ GetUniqueID()));
    for (int32 PartIndex = 0; PartIndex < BurstParts.Num(); ++PartIndex)
    {
        UStaticMeshComponent* Part = BurstParts[PartIndex];
        if (!Part) continue;

        const float Angle = RandomStream.FRandRange(0.0f, UE_TWO_PI);
        const FVector HorizontalDirection(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
        const FVector Direction = (
            HorizontalDirection * RandomStream.FRandRange(0.65f, 1.0f)
            + FVector::UpVector * RandomStream.FRandRange(0.25f, 0.9f)).GetSafeNormal();

        PartVelocities[PartIndex] = Direction * RandomStream.FRandRange(260.0f, 610.0f);
        PartRotationRates[PartIndex] = FRotator(
            RandomStream.FRandRange(-260.0f, 260.0f),
            RandomStream.FRandRange(-320.0f, 320.0f),
            RandomStream.FRandRange(-260.0f, 260.0f));

        const float UniformScale = RandomStream.FRandRange(0.016f, 0.044f);
        FVector InitialScale(UniformScale);
        if ((PartIndex % 3) == 0)
        {
            InitialScale *= FVector(1.5f, 0.55f, 0.55f);
        }
        PartInitialScales[PartIndex] = InitialScale;

        Part->SetRelativeLocation(FVector(
            RandomStream.FRandRange(-18.0f, 18.0f),
            RandomStream.FRandRange(-18.0f, 18.0f),
            RandomStream.FRandRange(-22.0f, 25.0f)));
        Part->SetRelativeRotation(FRotator(
            RandomStream.FRandRange(-180.0f, 180.0f),
            RandomStream.FRandRange(-180.0f, 180.0f),
            RandomStream.FRandRange(-180.0f, 180.0f)));
        Part->SetRelativeScale3D(InitialScale);
        if (!PaletteMaterials.IsEmpty())
        {
            Part->SetMaterial(0, PaletteMaterials[PartIndex % PaletteMaterials.Num()]);
        }
        Part->SetVisibility(true);
    }

    ElapsedTime = 0.0f;
    BurstLight->SetIntensity(InitialLightIntensity);
    BurstLight->SetVisibility(true);
    SetLifeSpan(EffectLifetime + 0.15f);
}

void AFPSDeathEffect::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    ElapsedTime += DeltaSeconds;
    const float LifeAlpha = FMath::Clamp(ElapsedTime / FMath::Max(EffectLifetime, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
    const float RemainingAlpha = 1.0f - LifeAlpha;
    const float ScaleAlpha = FMath::Pow(RemainingAlpha, 0.72f);
    const float DragMultiplier = FMath::Max(0.0f, 1.0f - Drag * DeltaSeconds);

    for (int32 PartIndex = 0; PartIndex < BurstParts.Num(); ++PartIndex)
    {
        UStaticMeshComponent* Part = BurstParts[PartIndex];
        if (!Part || !PartVelocities.IsValidIndex(PartIndex)
            || !PartRotationRates.IsValidIndex(PartIndex)
            || !PartInitialScales.IsValidIndex(PartIndex))
        {
            continue;
        }

        FVector& Velocity = PartVelocities[PartIndex];
        Velocity.Z -= Gravity * DeltaSeconds;
        Velocity *= DragMultiplier;
        Part->AddWorldOffset(Velocity * DeltaSeconds, false, nullptr, ETeleportType::TeleportPhysics);

        const FRotator& RotationRate = PartRotationRates[PartIndex];
        Part->AddLocalRotation(FRotator(
            RotationRate.Pitch * DeltaSeconds,
            RotationRate.Yaw * DeltaSeconds,
            RotationRate.Roll * DeltaSeconds));
        Part->SetRelativeScale3D(PartInitialScales[PartIndex] * ScaleAlpha);
    }

    if (CoreFlash)
    {
        const float CoreWindowAlpha = FMath::Clamp(LifeAlpha / 0.48f, 0.0f, 1.0f);
        const float CorePulse = FMath::Sin(PI * CoreWindowAlpha)
            * FMath::Pow(RemainingAlpha, 0.25f);
        CoreFlash->SetRelativeScale3D(FVector(
            (0.055f + 0.47f * CorePulse) * FMath::Max(0.06f, RemainingAlpha)));
    }

    const float ShockwaveEase = 1.0f - FMath::Pow(RemainingAlpha, 3.0f);
    const float ShockwaveRadius = FMath::Lerp(30.0f, 275.0f, ShockwaveEase);
    const float ShockwaveScale = FMath::Pow(RemainingAlpha, 0.58f);
    for (int32 SegmentIndex = 0; SegmentIndex < ShockwaveSegments.Num(); ++SegmentIndex)
    {
        UStaticMeshComponent* Segment = ShockwaveSegments[SegmentIndex];
        if (!Segment) continue;

        const float Angle = UE_TWO_PI * static_cast<float>(SegmentIndex)
            / static_cast<float>(ShockwaveSegments.Num());
        Segment->SetRelativeLocation(FVector(
            FMath::Cos(Angle) * ShockwaveRadius,
            FMath::Sin(Angle) * ShockwaveRadius,
            FMath::Lerp(-4.0f, 18.0f, LifeAlpha)));
        Segment->SetRelativeScale3D(FVector(
            0.035f * ShockwaveScale,
            0.30f * ShockwaveScale,
            0.026f * ShockwaveScale));
    }

    if (BurstLight)
    {
        const float LightPulse = FMath::Sin(PI * FMath::Clamp(LifeAlpha / 0.42f, 0.0f, 1.0f));
        BurstLight->SetIntensity(
            InitialLightIntensity * RemainingAlpha * RemainingAlpha + 1300.0f * LightPulse);
    }

    if (LifeAlpha >= 1.0f)
    {
        for (UStaticMeshComponent* Part : BurstParts)
        {
            if (Part) Part->SetVisibility(false);
        }
        if (CoreFlash) CoreFlash->SetVisibility(false);
        for (UStaticMeshComponent* Segment : ShockwaveSegments)
        {
            if (Segment) Segment->SetVisibility(false);
        }
        if (BurstLight)
        {
            BurstLight->SetIntensity(0.0f);
            BurstLight->SetVisibility(false);
        }
        SetActorTickEnabled(false);
        Destroy();
    }
}
