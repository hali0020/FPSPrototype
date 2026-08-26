#include "FPSImpactEffect.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AFPSImpactEffect::AFPSImpactEffect()
{
    PrimaryActorTick.bCanEverTick = true;
    SetActorEnableCollision(false);
    SetCanBeDamaged(false);

    VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
    SetRootComponent(VisualRoot);

    ImpactCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ImpactCore"));
    SparkA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SparkA"));
    SparkB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SparkB"));
    SparkC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SparkC"));

    UStaticMeshComponent* VisualMeshes[] = {
        ImpactCore.Get(), SparkA.Get(), SparkB.Get(), SparkC.Get()};
    for (UStaticMeshComponent* Mesh : VisualMeshes)
    {
        Mesh->SetupAttachment(VisualRoot);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->SetGenerateOverlapEvents(false);
        Mesh->SetCanEverAffectNavigation(false);
        Mesh->SetCastShadow(false);
    }

    ImpactLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ImpactLight"));
    ImpactLight->SetupAttachment(VisualRoot);
    ImpactLight->SetRelativeLocation(FVector(5.0f, 0.0f, 0.0f));
    ImpactLight->SetIntensity(InitialLightIntensity);
    ImpactLight->SetAttenuationRadius(175.0f);
    ImpactLight->SetCastShadows(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ColorMaterialAsset(
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    if (SphereAsset.Succeeded()) ImpactCore->SetStaticMesh(SphereAsset.Object);
    if (CubeAsset.Succeeded())
    {
        SparkA->SetStaticMesh(CubeAsset.Object);
        SparkB->SetStaticMesh(CubeAsset.Object);
        SparkC->SetStaticMesh(CubeAsset.Object);
    }
    if (ColorMaterialAsset.Succeeded())
    {
        ColorMaterial = ColorMaterialAsset.Object;
        for (UStaticMeshComponent* Mesh : VisualMeshes)
        {
            Mesh->SetMaterial(0, ColorMaterial);
        }
    }

    ImpactCore->SetRelativeLocation(FVector(2.0f, 0.0f, 0.0f));
    ImpactCore->SetRelativeScale3D(FVector(0.055f, 0.055f, 0.055f));

    SparkA->SetRelativeLocation(FVector(2.0f, 0.0f, 0.0f));
    SparkB->SetRelativeLocation(FVector(2.0f, 0.0f, 0.0f));
    SparkC->SetRelativeLocation(FVector(2.0f, 0.0f, 0.0f));
    SparkA->SetRelativeScale3D(FVector(0.014f, 0.085f, 0.014f));
    SparkB->SetRelativeScale3D(FVector(0.014f, 0.085f, 0.014f));
    SparkC->SetRelativeScale3D(FVector(0.014f, 0.085f, 0.014f));
    SparkA->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    SparkB->SetRelativeRotation(FRotator(0.0f, 0.0f, 60.0f));
    SparkC->SetRelativeRotation(FRotator(0.0f, 0.0f, 120.0f));
}

void AFPSImpactEffect::InitializeImpact(bool bInCharacterImpact)
{
    bCharacterImpact = bInCharacterImpact;
}

void AFPSImpactEffect::BeginPlay()
{
    Super::BeginPlay();
    SetLifeSpan(FMath::Max(ImpactDuration, 0.05f) + 0.10f);
    const FLinearColor ImpactColor = bCharacterImpact
        ? FLinearColor(1.0f, 0.015f, 0.005f)
        : FLinearColor(1.0f, 0.30f, 0.025f);
    SetMeshColor(ImpactCore, ImpactColor);
    SetMeshColor(SparkA, ImpactColor);
    SetMeshColor(SparkB, ImpactColor);
    SetMeshColor(SparkC, ImpactColor);
    ImpactLight->SetLightColor(ImpactColor);
    ImpactLight->SetIntensity(InitialLightIntensity);
}

void AFPSImpactEffect::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    Age += DeltaSeconds;
    const float Progress = FMath::Clamp(
        Age / FMath::Max(ImpactDuration, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
    const float Remaining = 1.0f - Progress;
    const float CoreScale = 0.055f * (1.0f + Progress * 1.4f) * Remaining;
    const float SparkScale = (0.8f + Progress * 1.6f) * Remaining;

    ImpactCore->SetRelativeScale3D(FVector(CoreScale, CoreScale, CoreScale));
    SparkA->SetRelativeScale3D(FVector(0.014f, 0.085f, 0.014f) * SparkScale);
    SparkB->SetRelativeScale3D(FVector(0.014f, 0.085f, 0.014f) * SparkScale);
    SparkC->SetRelativeScale3D(FVector(0.014f, 0.085f, 0.014f) * SparkScale);
    VisualRoot->AddLocalRotation(FRotator(0.0f, 0.0f, 420.0f * DeltaSeconds));
    ImpactLight->SetIntensity(InitialLightIntensity * Remaining * Remaining);

    if (Progress >= 1.0f) Destroy();
}

void AFPSImpactEffect::SetMeshColor(UStaticMeshComponent* Mesh, const FLinearColor& Color) const
{
    if (!Mesh) return;
    if (UMaterialInstanceDynamic* Material = Mesh->CreateAndSetMaterialInstanceDynamic(0))
    {
        Material->SetVectorParameterValue(TEXT("Color"), Color);
    }
}
