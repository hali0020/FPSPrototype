#include "FPSPickup.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "FPSCharacter.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

AFPSPickup::AFPSPickup()
{
    PrimaryActorTick.bCanEverTick = true;

    PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
    SetRootComponent(PickupSphere);
    PickupSphere->InitSphereRadius(75.0f);
    PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupSphere->SetCollisionObjectType(ECC_WorldDynamic);
    PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    PickupSphere->SetGenerateOverlapEvents(true);

    VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
    VisualRoot->SetupAttachment(PickupSphere);

    CaseBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaseBase"));
    CaseLid = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaseLid"));
    CrossLong = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrossLong"));
    CrossShort = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrossShort"));
    AmmoRoundA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoRoundA"));
    AmmoRoundB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoRoundB"));
    AmmoRoundC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoRoundC"));

    UStaticMeshComponent* VisualMeshes[] = {
        CaseBase.Get(), CaseLid.Get(), CrossLong.Get(), CrossShort.Get(),
        AmmoRoundA.Get(), AmmoRoundB.Get(), AmmoRoundC.Get()};
    for (UStaticMeshComponent* Mesh : VisualMeshes)
    {
        Mesh->SetupAttachment(VisualRoot);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->SetGenerateOverlapEvents(false);
        Mesh->SetCanEverAffectNavigation(false);
    }

    AccentLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("AccentLight"));
    AccentLight->SetupAttachment(VisualRoot);
    AccentLight->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    AccentLight->SetIntensity(900.0f);
    AccentLight->SetAttenuationRadius(190.0f);
    AccentLight->SetCastShadows(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CaseMeshAsset(
        TEXT("/Game/LevelPrototyping/Meshes/SM_ChamferCube.SM_ChamferCube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(
        TEXT("/Game/LevelPrototyping/Meshes/SM_Cylinder.SM_Cylinder"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ColorMaterialAsset(
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    static ConstructorHelpers::FObjectFinder<USoundBase> AmmoPickupSoundAsset(
        TEXT("/Game/Pickups/Audio/Generated/SFX_Pickup_Ammo_01.SFX_Pickup_Ammo_01"));
    static ConstructorHelpers::FObjectFinder<USoundBase> HealthPickupSoundAsset(
        TEXT("/Game/Pickups/Audio/Generated/SFX_Pickup_Health_01.SFX_Pickup_Health_01"));
    static ConstructorHelpers::FObjectFinder<USoundBase> SupplyPickupSoundAsset(
        TEXT("/Game/Pickups/Audio/Generated/SFX_Pickup_Supply_01.SFX_Pickup_Supply_01"));

    if (CaseMeshAsset.Succeeded())
    {
        CaseBase->SetStaticMesh(CaseMeshAsset.Object);
        CaseLid->SetStaticMesh(CaseMeshAsset.Object);
        CrossLong->SetStaticMesh(CaseMeshAsset.Object);
        CrossShort->SetStaticMesh(CaseMeshAsset.Object);
    }
    if (CylinderMeshAsset.Succeeded())
    {
        AmmoRoundA->SetStaticMesh(CylinderMeshAsset.Object);
        AmmoRoundB->SetStaticMesh(CylinderMeshAsset.Object);
        AmmoRoundC->SetStaticMesh(CylinderMeshAsset.Object);
    }
    if (ColorMaterialAsset.Succeeded())
    {
        ColorMaterial = ColorMaterialAsset.Object;
        for (UStaticMeshComponent* Mesh : VisualMeshes)
        {
            Mesh->SetMaterial(0, ColorMaterial);
        }
    }
    if (AmmoPickupSoundAsset.Succeeded()) AmmoPickupSound = AmmoPickupSoundAsset.Object;
    if (HealthPickupSoundAsset.Succeeded()) HealthPickupSound = HealthPickupSoundAsset.Object;
    if (SupplyPickupSoundAsset.Succeeded()) SupplyPickupSound = SupplyPickupSoundAsset.Object;

    CaseBase->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    CaseBase->SetRelativeScale3D(FVector(0.58f, 0.40f, 0.28f));
    CaseLid->SetRelativeLocation(FVector(0.0f, 0.0f, 17.0f));
    CaseLid->SetRelativeScale3D(FVector(0.54f, 0.36f, 0.08f));

    CrossLong->SetRelativeLocation(FVector(0.0f, 0.0f, 23.5f));
    CrossLong->SetRelativeScale3D(FVector(0.20f, 0.06f, 0.025f));
    CrossShort->SetRelativeLocation(FVector(0.0f, 0.0f, 23.5f));
    CrossShort->SetRelativeScale3D(FVector(0.06f, 0.20f, 0.025f));

    AmmoRoundA->SetRelativeLocation(FVector(-17.0f, 0.0f, 33.0f));
    AmmoRoundB->SetRelativeLocation(FVector(0.0f, 0.0f, 33.0f));
    AmmoRoundC->SetRelativeLocation(FVector(17.0f, 0.0f, 33.0f));
    AmmoRoundA->SetRelativeScale3D(FVector(0.055f, 0.055f, 0.22f));
    AmmoRoundB->SetRelativeScale3D(FVector(0.055f, 0.055f, 0.22f));
    AmmoRoundC->SetRelativeScale3D(FVector(0.055f, 0.055f, 0.22f));

    PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AFPSPickup::HandleOverlap);
}

void AFPSPickup::InitializePickup(
    EFPSPickupType InType, int32 InAmmoAmount, float InHealAmount, float InLifetimeSeconds)
{
    PickupType = InType;
    AmmoAmount = FMath::Max(0, InAmmoAmount);
    HealAmount = FMath::Max(0.0f, InHealAmount);
    LifetimeSeconds = FMath::Max(0.0f, InLifetimeSeconds);
}

void AFPSPickup::BeginPlay()
{
    Super::BeginPlay();
    RestLocation = GetActorLocation();
    BobPhase = FMath::FRandRange(0.0f, 2.0f * PI);
    ApplyVisualStyle();
    if (LifetimeSeconds > 0.0f) SetLifeSpan(LifetimeSeconds);
}

void AFPSPickup::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bConsumed || !GetWorld()) return;

    const float BobOffset = FMath::Sin(GetWorld()->GetTimeSeconds() * BobSpeed + BobPhase) * BobAmplitude;
    SetActorLocation(RestLocation + FVector(0.0f, 0.0f, BobOffset));
    AddActorLocalRotation(FRotator(0.0f, SpinSpeed * DeltaSeconds, 0.0f));
}

void AFPSPickup::HandleOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    (void)OverlappedComponent;
    (void)OtherComponent;
    (void)OtherBodyIndex;
    (void)bFromSweep;
    (void)SweepResult;

    if (bConsumed) return;
    AFPSCharacter* Character = Cast<AFPSCharacter>(OtherActor);
    if (!Character || Character->IsDead()) return;

    int32 AddedAmmo = 0;
    float RestoredHealth = 0.0f;
    if (PickupType == EFPSPickupType::Ammo || PickupType == EFPSPickupType::Supply)
    {
        AddedAmmo = Character->AddReserveAmmo(AmmoAmount);
    }
    if (PickupType == EFPSPickupType::Health || PickupType == EFPSPickupType::Supply)
    {
        if (UHealthComponent* Health = Character->GetHealthComponent())
        {
            RestoredHealth = Health->Heal(HealAmount);
        }
    }
    if (AddedAmmo <= 0 && RestoredHealth <= 0.0f) return;

    bConsumed = true;
    PickupSphere->SetGenerateOverlapEvents(false);
    PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FString Message;
    if (AddedAmmo > 0 && RestoredHealth > 0.0f)
    {
        Message = FString::Printf(
            TEXT("SUPPLY  +%d HP  +%d AMMO"), FMath::RoundToInt(RestoredHealth), AddedAmmo);
    }
    else if (AddedAmmo > 0)
    {
        Message = FString::Printf(TEXT("AMMO +%d"), AddedAmmo);
    }
    else
    {
        Message = FString::Printf(TEXT("HEALTH +%d"), FMath::RoundToInt(RestoredHealth));
    }
    Character->ShowPickupMessage(Message);

    if (USoundBase* SelectedPickupSound = GetPickupSound())
    {
        const float Volume = PickupType == EFPSPickupType::Health ? 0.52f : 0.62f;
        UGameplayStatics::PlaySound2D(this, SelectedPickupSound, Volume);
    }
    Destroy();
}

USoundBase* AFPSPickup::GetPickupSound() const
{
    switch (PickupType)
    {
    case EFPSPickupType::Health:
        return HealthPickupSound.Get();
    case EFPSPickupType::Supply:
        return SupplyPickupSound.Get();
    case EFPSPickupType::Ammo:
    default:
        return AmmoPickupSound.Get();
    }
}

void AFPSPickup::ApplyVisualStyle()
{
    const bool bShowCross = PickupType == EFPSPickupType::Health || PickupType == EFPSPickupType::Supply;
    const bool bShowAmmo = PickupType == EFPSPickupType::Ammo || PickupType == EFPSPickupType::Supply;
    CrossLong->SetVisibility(bShowCross, true);
    CrossShort->SetVisibility(bShowCross, true);
    AmmoRoundA->SetVisibility(bShowAmmo, true);
    AmmoRoundB->SetVisibility(bShowAmmo, true);
    AmmoRoundC->SetVisibility(bShowAmmo, true);

    FLinearColor BaseColor;
    FLinearColor LidColor;
    FLinearColor LightColor;
    if (PickupType == EFPSPickupType::Health)
    {
        BaseColor = FLinearColor(0.12f, 0.15f, 0.18f);
        LidColor = FLinearColor(0.72f, 0.76f, 0.78f);
        LightColor = FLinearColor(1.0f, 0.06f, 0.04f);
        CrossLong->SetRelativeLocation(FVector(0.0f, 0.0f, 23.5f));
        CrossShort->SetRelativeLocation(FVector(0.0f, 0.0f, 23.5f));
    }
    else if (PickupType == EFPSPickupType::Supply)
    {
        BaseColor = FLinearColor(0.025f, 0.15f, 0.12f);
        LidColor = FLinearColor(0.04f, 0.42f, 0.34f);
        LightColor = FLinearColor(0.12f, 1.0f, 0.66f);
        CrossLong->SetRelativeLocation(FVector(-18.0f, 0.0f, 23.5f));
        CrossShort->SetRelativeLocation(FVector(-18.0f, 0.0f, 23.5f));
        AmmoRoundA->SetRelativeLocation(FVector(18.0f, -11.0f, 33.0f));
        AmmoRoundB->SetRelativeLocation(FVector(18.0f, 0.0f, 33.0f));
        AmmoRoundC->SetRelativeLocation(FVector(18.0f, 11.0f, 33.0f));
        SetActorScale3D(FVector(1.12f, 1.12f, 1.12f));
    }
    else
    {
        BaseColor = FLinearColor(0.025f, 0.07f, 0.11f);
        LidColor = FLinearColor(0.04f, 0.32f, 0.58f);
        LightColor = FLinearColor(0.06f, 0.55f, 1.0f);
    }

    SetMeshColor(CaseBase, BaseColor);
    SetMeshColor(CaseLid, LidColor);
    SetMeshColor(CrossLong, FLinearColor(0.95f, 0.015f, 0.01f));
    SetMeshColor(CrossShort, FLinearColor(0.95f, 0.015f, 0.01f));
    const FLinearColor BrassColor(0.92f, 0.42f, 0.055f);
    SetMeshColor(AmmoRoundA, BrassColor);
    SetMeshColor(AmmoRoundB, BrassColor);
    SetMeshColor(AmmoRoundC, BrassColor);
    AccentLight->SetLightColor(LightColor);
}

void AFPSPickup::SetMeshColor(UStaticMeshComponent* Mesh, const FLinearColor& Color) const
{
    if (!Mesh) return;
    if (UMaterialInstanceDynamic* Material = Mesh->CreateAndSetMaterialInstanceDynamic(0))
    {
        Material->SetVectorParameterValue(TEXT("Color"), Color);
    }
}
