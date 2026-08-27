#include "FPSCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "FPSImpactEffect.h"
#include "FPSFirstPersonAnimInstance.h"
#include "FPSGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HealthComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "UObject/ConstructorHelpers.h"

AFPSCharacter::AFPSCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetMesh()->SetRelativeLocation(FVector(-20.0f, 0.0f, -96.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    GetMesh()->SetOwnerNoSee(true);
    GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
    GetMesh()->VisibilityBasedAnimTickOption =
        EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
    FirstPersonMesh->SetupAttachment(GetMesh());
    FirstPersonMesh->SetOnlyOwnerSee(true);
    FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
    FirstPersonMesh->VisibilityBasedAnimTickOption =
        EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    FirstPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FirstPersonMesh->SetCastShadow(false);

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    // Keep view motion on the stable world-pose head. Never drive this camera from
    // a third-person full-body ADS pose: that lets the shoulders and rifle enter it.
    FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head"));
    FirstPersonCamera->SetRelativeLocationAndRotation(
        HipCameraRelativeLocation, FRotator(0.0f, 90.0f, -90.0f));
    FirstPersonCamera->bUsePawnControlRotation = true;
    FirstPersonCamera->bEnableFirstPersonFieldOfView = true;
    FirstPersonCamera->bEnableFirstPersonScale = true;
    FirstPersonCamera->FirstPersonFieldOfView = HipFirstPersonFOV;
    FirstPersonCamera->FirstPersonScale = 0.6f;

    WeaponFeedbackAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("WeaponFeedbackAudio"));
    WeaponFeedbackAudio->SetupAttachment(FirstPersonCamera);
    WeaponFeedbackAudio->bAutoActivate = false;
    WeaponFeedbackAudio->bAllowSpatialization = false;
    WeaponFeedbackAudio->SetUISound(true);

    HitConfirmAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("HitConfirmAudio"));
    HitConfirmAudio->SetupAttachment(FirstPersonCamera);
    HitConfirmAudio->bAutoActivate = false;
    HitConfirmAudio->bAllowSpatialization = false;
    HitConfirmAudio->SetUISound(true);

    PlayerVoiceAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("PlayerVoiceAudio"));
    PlayerVoiceAudio->SetupAttachment(FirstPersonCamera);
    PlayerVoiceAudio->bAutoActivate = false;
    PlayerVoiceAudio->bAllowSpatialization = false;
    PlayerVoiceAudio->SetUISound(true);
    PlayerVoiceAudio->SetVolumeMultiplier(0.9f);

    WeaponAimRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponAimRoot"));
    WeaponAimRoot->SetupAttachment(FirstPersonMesh, TEXT("HandGrip_R"));

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(WeaponAimRoot);
    WeaponMesh->SetOnlyOwnerSee(true);
    WeaponMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetCastShadow(false);

    // Eight voices cover the full 0.82 s report at the 0.12 s fire interval,
    // so automatic fire never restarts and chops off the previous shot's tail.
    for (int32 VoiceIndex = 0; VoiceIndex < 8; ++VoiceIndex)
    {
        UAudioComponent* FireVoice = CreateDefaultSubobject<UAudioComponent>(
            *FString::Printf(TEXT("WeaponFireAudio_%02d"), VoiceIndex));
        FireVoice->SetupAttachment(WeaponMesh, TEXT("Muzzle"));
        FireVoice->bAutoActivate = false;
        FireVoice->bOverrideAttenuation = true;
        FireVoice->AttenuationOverrides.bAttenuate = true;
        FireVoice->AttenuationOverrides.bSpatialize = true;
        FireVoice->AttenuationOverrides.DistanceAlgorithm = EAttenuationDistanceModel::NaturalSound;
        FireVoice->AttenuationOverrides.AttenuationShape = EAttenuationShape::Sphere;
        FireVoice->AttenuationOverrides.AttenuationShapeExtents = FVector(180.0f, 0.0f, 0.0f);
        FireVoice->AttenuationOverrides.FalloffDistance = 4200.0f;
        FireVoice->SetVolumeMultiplier(0.82f);
        WeaponFireAudioPool.Add(FireVoice);
    }

    auto CreateOpticFramePart = [this](
        const TCHAR* Name, const FVector& Location, const FVector& Scale)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        // This rifle has no optic socket, so mount the code-built sight directly
        // in the rifle's local space above the receiver rail. Unlike the old
        // camera-fixed frame, it now follows the weapon and is visible from hip.
        Part->SetupAttachment(WeaponMesh);
        Part->SetRelativeLocation(Location);
        Part->SetRelativeScale3D(Scale);
        Part->SetOnlyOwnerSee(true);
        Part->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetCastShadow(false);
        Part->SetVisibility(true);
        OpticFrameMeshes.Add(Part);
    };
    // SKM_Rifle points down +Y, is about 6 cm wide, and its upper receiver is
    // around Z=11..14. Keep the housing compact so it reads like a holographic
    // sight instead of covering the whole screen during ADS.
    constexpr float OpticRailY = 28.0f;
    constexpr float OpticWindowCenterZ = 21.2f;
    constexpr float OpticMountCenterZ = 15.2f;
    OpticAimPoint = CreateDefaultSubobject<USceneComponent>(TEXT("OpticAimPoint"));
    OpticAimPoint->SetupAttachment(WeaponMesh);
    OpticAimPoint->SetRelativeLocation(FVector(0.0f, OpticRailY, OpticWindowCenterZ));
    CreateOpticFramePart(TEXT("OpticFrameTop"),
        FVector(0.0f, OpticRailY, OpticWindowCenterZ + 2.8f), FVector(0.085f, 0.012f, 0.006f));
    CreateOpticFramePart(TEXT("OpticFrameBottom"),
        FVector(0.0f, OpticRailY, OpticWindowCenterZ - 2.8f), FVector(0.085f, 0.012f, 0.006f));
    CreateOpticFramePart(TEXT("OpticFrameLeft"),
        FVector(-4.0f, OpticRailY, OpticWindowCenterZ), FVector(0.006f, 0.012f, 0.056f));
    CreateOpticFramePart(TEXT("OpticFrameRight"),
        FVector(4.0f, OpticRailY, OpticWindowCenterZ), FVector(0.006f, 0.012f, 0.056f));
    CreateOpticFramePart(TEXT("OpticMountBase"),
        FVector(0.0f, OpticRailY - 0.8f, OpticMountCenterZ), FVector(0.055f, 0.035f, 0.008f));
    // Two slim risers keep the raised sight window physically connected to the
    // receiver while letting the rifle itself remain lower in the player's view.
    CreateOpticFramePart(TEXT("OpticMountLeft"),
        FVector(-3.0f, OpticRailY, 17.4f), FVector(0.008f, 0.018f, 0.022f));
    CreateOpticFramePart(TEXT("OpticMountRight"),
        FVector(3.0f, OpticRailY, 17.4f), FVector(0.008f, 0.018f, 0.022f));

    MuzzleFlashMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MuzzleFlashMesh"));
    MuzzleFlashMesh->SetupAttachment(WeaponMesh, TEXT("Muzzle"));
    MuzzleFlashMesh->SetRelativeLocation(FVector(7.0f, 0.0f, 0.0f));
    MuzzleFlashMesh->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
    MuzzleFlashMesh->SetRelativeScale3D(FVector(0.035f, 0.035f, 0.12f));
    MuzzleFlashMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MuzzleFlashMesh->SetOnlyOwnerSee(true);
    MuzzleFlashMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
    MuzzleFlashMesh->SetCastShadow(false);
    MuzzleFlashMesh->SetVisibility(false);

    MuzzleFlash = CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleFlash"));
    MuzzleFlash->SetupAttachment(WeaponMesh, TEXT("Muzzle"));
    MuzzleFlash->SetRelativeLocation(FVector(8.0f, 0.0f, 0.0f));
    MuzzleFlash->SetLightColor(FLinearColor(1.0f, 0.35f, 0.04f));
    MuzzleFlash->SetAttenuationRadius(250.0f);
    MuzzleFlash->SetIntensity(0.0f);
    MuzzleFlash->SetCastShadows(false);

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMesh(
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
    static ConstructorHelpers::FClassFinder<UAnimInstance> RifleAnimBlueprint(
        TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Rifle"));
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> RifleMesh(
        TEXT("/Game/Weapons/Rifle/Meshes/SKM_Rifle.SKM_Rifle"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> FlashCone(
        TEXT("/Engine/BasicShapes/Cone.Cone"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> OpticCube(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> OpticHousingMaterial(
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    static ConstructorHelpers::FObjectFinder<UAnimMontage> FireMontageAsset(
        TEXT("/Game/Variant_Shooter/Anims/FP_Rifle_Shoot_Montage.FP_Rifle_Shoot_Montage"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> ReloadAsset(
        TEXT("/Game/Characters/Mannequins/Anims/Rifle/MM_Rifle_Reload.MM_Rifle_Reload"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> DryFireAsset(
        TEXT("/Game/Characters/Mannequins/Anims/Rifle/MM_Rifle_DryFire.MM_Rifle_DryFire"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> SprintAsset(
        TEXT("/Game/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Fwd.MF_Rifle_Jog_Fwd"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> DeathAsset(
        TEXT("/Game/Characters/Mannequins/Anims/Death/MM_Death_Front_01.MM_Death_Front_01"));
    static ConstructorHelpers::FObjectFinder<USoundBase> FireSoundAsset(
        TEXT("/Game/Weapons/Rifle/Audio/Generated/SFX_Rifle_Shot_01.SFX_Rifle_Shot_01"));
    static ConstructorHelpers::FObjectFinder<USoundBase> EmptySoundAsset(
        TEXT("/Game/InterfaceAndItemSounds/WAV/Click_03_wav.Click_03_wav"));
    static ConstructorHelpers::FObjectFinder<USoundBase> ReloadSoundAsset(
        TEXT("/Game/InterfaceAndItemSounds/WAV/Flick_Switch_01_wav.Flick_Switch_01_wav"));
    static ConstructorHelpers::FObjectFinder<USoundBase> HitConfirmSoundAsset(
        TEXT("/Game/Weapons/Rifle/Audio/Generated/SFX_HitConfirm_01.SFX_HitConfirm_01"));
    static ConstructorHelpers::FObjectFinder<USoundBase> PlayerHurtSoundAsset(
        TEXT("/Game/HumanVocalizations/HumanMaleD/Wavs/voice_male_d_hurt_pain_low_02.voice_male_d_hurt_pain_low_02"));
    static ConstructorHelpers::FObjectFinder<USoundBase> PlayerDeathSoundAsset(
        TEXT("/Game/HumanVocalizations/HumanMaleD/Wavs/voice_male_d_death_05.voice_male_d_death_05"));

    if (MannyMesh.Succeeded())
    {
        GetMesh()->SetSkeletalMeshAsset(MannyMesh.Object);
        FirstPersonMesh->SetSkeletalMeshAsset(MannyMesh.Object);
    }
    if (RifleAnimBlueprint.Succeeded()) GetMesh()->SetAnimInstanceClass(RifleAnimBlueprint.Class);
    FirstPersonMesh->SetAnimInstanceClass(UFPSFirstPersonAnimInstance::StaticClass());
    if (RifleMesh.Succeeded()) WeaponMesh->SetSkeletalMeshAsset(RifleMesh.Object);
    if (FlashCone.Succeeded()) MuzzleFlashMesh->SetStaticMesh(FlashCone.Object);
    if (OpticCube.Succeeded())
    {
        for (UStaticMeshComponent* Part : OpticFrameMeshes)
        {
            Part->SetStaticMesh(OpticCube.Object);
            if (OpticHousingMaterial.Succeeded())
            {
                Part->SetMaterial(0, OpticHousingMaterial.Object);
            }
        }
    }
    if (FireMontageAsset.Succeeded()) FireMontage = FireMontageAsset.Object;
    if (ReloadAsset.Succeeded()) ReloadAnimation = ReloadAsset.Object;
    if (DryFireAsset.Succeeded()) DryFireAnimation = DryFireAsset.Object;
    if (SprintAsset.Succeeded()) SprintAnimation = SprintAsset.Object;
    if (DeathAsset.Succeeded()) DeathAnimation = DeathAsset.Object;
    if (FireSoundAsset.Succeeded()) FireSound = FireSoundAsset.Object;
    if (EmptySoundAsset.Succeeded()) EmptySound = EmptySoundAsset.Object;
    if (ReloadSoundAsset.Succeeded()) ReloadSound = ReloadSoundAsset.Object;
    if (HitConfirmSoundAsset.Succeeded()) HitConfirmSound = HitConfirmSoundAsset.Object;
    if (PlayerHurtSoundAsset.Succeeded()) PlayerHurtSound = PlayerHurtSoundAsset.Object;
    if (PlayerDeathSoundAsset.Succeeded()) PlayerDeathSound = PlayerDeathSoundAsset.Object;

    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AFPSCharacter::BeginPlay()
{
    Super::BeginPlay();
#if !UE_BUILD_SHIPPING
    // Offline visual regression hook used by the automated screenshot pass.
    bIsAiming = FParse::Param(FCommandLine::Get(), TEXT("FPSCaptureAim"));
    if (bIsAiming && FParse::Param(FCommandLine::Get(), TEXT("FPSCaptureScreenshot")))
    {
        FTimerHandle CaptureTimer;
        GetWorldTimerManager().SetTimer(
            CaptureTimer,
            FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                UE_LOG(LogTemp, Display, TEXT("FPS regression screenshot requested after ADS settle."));
                FScreenshotRequest::RequestScreenshot(true, true);
            }),
            3.0f, false);
    }
#endif
    NormalWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
    UpdateMovementSpeed();
    AmmoInMagazine = MagazineSize;
    ReserveAmmo = FMath::Clamp(ReserveAmmo, 0, MaxReserveAmmo);
    if (ReloadAnimation) ReloadDuration = ReloadAnimation->GetPlayLength();
    FirstPersonMesh->SetHiddenInGame(false);
    FirstPersonMesh->SetVisibility(true);
    WeaponMesh->SetHiddenInGame(false);
    WeaponMesh->SetVisibility(true);
    if (FirstPersonMesh->GetBoneIndex(TEXT("head")) != INDEX_NONE)
    {
        FirstPersonMesh->HideBoneByName(TEXT("head"), PBO_None);
    }
    FirstPersonCamera->SetFieldOfView(HipFOV);
    FirstPersonCamera->FirstPersonFieldOfView = HipFirstPersonFOV;
    LastKnownHealth = HealthComponent->GetHealth();
    HealthComponent->OnHealthChanged.AddDynamic(this, &AFPSCharacter::HandleHealthChanged);
    HealthComponent->OnDeath.AddDynamic(this, &AFPSCharacter::HandleDeath);
    if (UMaterialInstanceDynamic* FlashMaterial = MuzzleFlashMesh->CreateAndSetMaterialInstanceDynamic(0))
    {
        FlashMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.12f, 0.01f));
    }
    if (OpticFrameMeshes.Num() > 0 && OpticFrameMeshes[0])
    {
        if (UMaterialInterface* BaseMaterial = OpticFrameMeshes[0]->GetMaterial(0))
        {
            UMaterialInstanceDynamic* OpticMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            OpticMaterial->SetVectorParameterValue(
                TEXT("Color"), FLinearColor(0.004f, 0.006f, 0.009f, 1.0f));
            OpticMaterial->SetVectorParameterValue(
                TEXT("BaseColor"), FLinearColor(0.004f, 0.006f, 0.009f, 1.0f));
            OpticMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.46f);
            OpticMaterial->SetScalarParameterValue(TEXT("Metallic"), 0.55f);
            for (UStaticMeshComponent* Part : OpticFrameMeshes)
            {
                if (Part) Part->SetMaterial(0, OpticMaterial);
            }
        }
    }
    SetOpticVisible(true);
}

void AFPSCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    RefreshSprintState();
    if (!bIsDead)
    {
        UpdateAim(DeltaSeconds);
        UpdateWeaponAimAlignment(DeltaSeconds);
        UpdateShotBloom(DeltaSeconds);
    }
}

void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::StartFire);
    PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPSCharacter::StopFire);
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFPSCharacter::StartReload);
    PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AFPSCharacter::StartAim);
    PlayerInputComponent->BindAction("Aim", IE_Released, this, &AFPSCharacter::StopAim);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AFPSCharacter::StartSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AFPSCharacter::StopSprint);
}

void AFPSCharacter::CancelTransientInput()
{
    ForwardInputValue = 0.0f;
    bSprintHeld = false;
    bIsAiming = false;
    StopFire();
    SetSprinting(false);
    StopJumping();
}

void AFPSCharacter::MoveForward(float Value)
{
    ForwardInputValue = Value;
    RefreshSprintState();
    if (!bIsDead) AddMovementInput(GetActorForwardVector(), Value);
}

void AFPSCharacter::MoveRight(float Value) { if (!bIsDead) AddMovementInput(GetActorRightVector(), Value); }

void AFPSCharacter::StartFire()
{
    RefreshSprintState();
    if (bIsDead || bIsSprinting)
    {
        bWantsToFire = false;
        UpdateMovementSpeed();
        return;
    }
    bWantsToFire = true;
    UpdateMovementSpeed();
    if (bIsReloading) return;
    FireShot();
    if (!bIsReloading && AmmoInMagazine > 0)
    {
        GetWorldTimerManager().SetTimer(FireTimer, this, &AFPSCharacter::FireShot, FireInterval, true);
    }
}

void AFPSCharacter::StopFire()
{
    bWantsToFire = false;
    GetWorldTimerManager().ClearTimer(FireTimer);
    UpdateMovementSpeed();
}

void AFPSCharacter::FireShot()
{
    RefreshSprintState();
    if (bIsDead || bIsReloading || bIsSprinting)
    {
        GetWorldTimerManager().ClearTimer(FireTimer);
        return;
    }
    if (AmmoInMagazine <= 0)
    {
        GetWorldTimerManager().ClearTimer(FireTimer);
        if (ReserveAmmo > 0)
        {
            StartReload();
        }
        else
        {
            PlayDryFireFeedback();
            bWantsToFire = false;
            UpdateMovementSpeed();
        }
        return;
    }

    --AmmoInMagazine;
    PlayFireAnimation();
    MuzzleFlashMesh->SetVisibility(true);
    MuzzleFlash->SetIntensity(7000.0f);
    GetWorldTimerManager().SetTimer(
        MuzzleFlashTimer, this, &AFPSCharacter::DisableMuzzleFlash, 0.04f, false);
    // HUD aim and ballistics both use the controller's final rendered view.
    // The view-model may sway cosmetically, but it never drags the stable aim
    // center away from the middle of the screen.
    FVector Start = FirstPersonCamera->GetComponentLocation();
    FRotator ViewRotation = FirstPersonCamera->GetComponentRotation();
    if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        PlayerController->GetPlayerViewPoint(Start, ViewRotation);
    }
    const FVector BaseShotDirection = ViewRotation.Vector();
    const float SpreadRadians = FMath::DegreesToRadians(GetCurrentWeaponSpreadDegrees());
    const FVector ShotDirection = FMath::VRandCone(BaseShotDirection, SpreadRadians);
    const FVector End = Start + ShotDirection * WeaponRange;
    CurrentShotBloomDegrees = FMath::Min(
        MaxShotBloomDegrees, CurrentShotBloomDegrees + ShotBloomPerShot);
    LastShotTime = GetWorld()->GetTimeSeconds();
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponTrace), true, this);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        bool bHitDamageableTarget = false;
        if (AActor* HitActor = Hit.GetActor())
        {
            if (UHealthComponent* TargetHealth = HitActor->FindComponentByClass<UHealthComponent>())
            {
                bHitDamageableTarget = true;
                const bool bWasAlive = !TargetHealth->IsDead();
                TargetHealth->ApplyDamage(Damage);
                if (bWasAlive)
                {
                    RegisterHit(TargetHealth->IsDead());
                }
            }
        }

        const FTransform ImpactTransform(
            Hit.ImpactNormal.Rotation(), Hit.ImpactPoint + Hit.ImpactNormal * 2.0f);
        AFPSImpactEffect* Impact = GetWorld()->SpawnActorDeferred<AFPSImpactEffect>(
            AFPSImpactEffect::StaticClass(), ImpactTransform, this, this,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (Impact)
        {
            Impact->InitializeImpact(bHitDamageableTarget);
            Impact->FinishSpawning(ImpactTransform);
        }
    }
    if (AmmoInMagazine <= 0)
    {
        if (ReserveAmmo > 0)
        {
            StartReload();
        }
        else
        {
            // The final shot has no future timer callback when the magazine and
            // reserve both reach zero. Explicitly release the firing slowdown.
            GetWorldTimerManager().ClearTimer(FireTimer);
            bWantsToFire = false;
            UpdateMovementSpeed();
        }
    }
}

void AFPSCharacter::StartReload()
{
    if (bIsDead || bIsReloading || AmmoInMagazine >= MagazineSize || ReserveAmmo <= 0) return;
    GetWorldTimerManager().ClearTimer(FireTimer);
    bIsReloading = true;
    bIsAiming = false;
    SetSprinting(false);
    UpdateMovementSpeed();
    PlayReloadAnimation();
    PlayLocalFeedback(WeaponFeedbackAudio, ReloadSound, 0.62f, 0.94f);
    GetWorldTimerManager().SetTimer(ReloadTimer, this, &AFPSCharacter::FinishReload, ReloadDuration, false);
}

void AFPSCharacter::FinishReload()
{
    if (bIsDead || !bIsReloading) return;
    const int32 Needed = MagazineSize - AmmoInMagazine;
    const int32 Loaded = FMath::Min(Needed, ReserveAmmo);
    AmmoInMagazine += Loaded;
    ReserveAmmo -= Loaded;
    bIsReloading = false;
    UpdateMovementSpeed();
    RefreshSprintState();
    PlayLocalFeedback(WeaponFeedbackAudio, ReloadSound, 0.52f, 1.12f);
    if (bWantsToFire && AmmoInMagazine > 0 && !bIsDead && !bIsSprinting)
    {
        FireShot();
        if (AmmoInMagazine > 0)
        {
            GetWorldTimerManager().SetTimer(FireTimer, this, &AFPSCharacter::FireShot, FireInterval, true);
        }
    }
}

void AFPSCharacter::StartAim()
{
    if (bIsDead || bIsReloading) return;
    bIsAiming = true;
    SetSprinting(false);
}

void AFPSCharacter::StopAim()
{
    bIsAiming = false;
    RefreshSprintState();
}

void AFPSCharacter::UpdateAim(float DeltaSeconds)
{
    const float TargetFOV = bIsAiming ? AimFOV : (bIsSprinting ? SprintFOV : HipFOV);
    const float TargetFirstPersonFOV = bIsAiming ? AimFirstPersonFOV : HipFirstPersonFOV;
    const FVector TargetCameraLocation = bIsAiming
        ? AimCameraRelativeLocation
        : HipCameraRelativeLocation;
    FirstPersonCamera->SetFieldOfView(FMath::FInterpTo(
        FirstPersonCamera->FieldOfView, TargetFOV, DeltaSeconds, AimInterpSpeed));
    FirstPersonCamera->FirstPersonFieldOfView = FMath::FInterpTo(
        FirstPersonCamera->FirstPersonFieldOfView, TargetFirstPersonFOV,
        DeltaSeconds, AimInterpSpeed);
    FirstPersonCamera->SetRelativeLocation(FMath::VInterpTo(
        FirstPersonCamera->GetRelativeLocation(), TargetCameraLocation,
        DeltaSeconds, AimInterpSpeed));
}

void AFPSCharacter::UpdateWeaponAimAlignment(float DeltaSeconds)
{
    if (!WeaponAimRoot || !FirstPersonMesh || !OpticAimPoint || !FirstPersonCamera)
    {
        return;
    }

    FVector TargetRelativeOffset = FVector::ZeroVector;
    const float AimProgress = GetAimProgress();
    if (bIsAiming && AimProgress > KINDA_SMALL_NUMBER)
    {
        FVector ViewLocation = FirstPersonCamera->GetComponentLocation();
        FRotator ViewRotation = FirstPersonCamera->GetComponentRotation();
        if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
        {
            PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
        }

        const FTransform GripTransform = FirstPersonMesh->GetSocketTransform(
            TEXT("HandGrip_R"), RTS_World);
        // Remove the previous frame's translation before calculating the new
        // correction. This prevents accumulation while the hand animation moves.
        const FVector PreviousWorldOffset =
            GripTransform.TransformVectorNoScale(CurrentWeaponAimOffset);
        const FVector UnalignedOpticLocation =
            OpticAimPoint->GetComponentLocation() - PreviousWorldOffset;
        const FVector ViewForward = ViewRotation.Vector();
        const FVector ToOptic = UnalignedOpticLocation - ViewLocation;
        const float ForwardDepth = FVector::DotProduct(ViewForward, ToOptic);
        if (ForwardDepth > 1.0f)
        {
            const FVector OffAxis = ToOptic - ViewForward * ForwardDepth;
            const FVector DesiredWorldOffset = -OffAxis;
            TargetRelativeOffset = GripTransform.InverseTransformVectorNoScale(
                DesiredWorldOffset).GetClampedToMaxSize(MaxWeaponAimOffset);
            TargetRelativeOffset *= AimProgress;
        }
    }

    CurrentWeaponAimOffset = FMath::VInterpTo(
        CurrentWeaponAimOffset, TargetRelativeOffset, DeltaSeconds,
        WeaponAimAlignmentSpeed);
    WeaponAimRoot->SetRelativeLocation(CurrentWeaponAimOffset);
}

void AFPSCharacter::StartSprint()
{
    if (bIsDead) return;
    bSprintHeld = true;
    RefreshSprintState();
}

void AFPSCharacter::StopSprint()
{
    bSprintHeld = false;
    SetSprinting(false);
}

void AFPSCharacter::RefreshSprintState()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    const bool bShouldSprint = Movement
        && bSprintHeld
        && ForwardInputValue > SprintForwardThreshold
        && !bIsDead
        && !bIsReloading
        && !bIsAiming
        && Movement->IsMovingOnGround();
    SetSprinting(bShouldSprint);
}

void AFPSCharacter::SetSprinting(bool bNewSprinting)
{
    if (bIsSprinting == bNewSprinting) return;

    bIsSprinting = bNewSprinting;
    UpdateMovementSpeed();

    if (bIsSprinting)
    {
        bIsAiming = false;
        StopFire();
    }
}

bool AFPSCharacter::IsActivelyFiring() const
{
    return bWantsToFire
        && !bIsReloading
        && AmmoInMagazine > 0
        && !bIsDead
        && !bIsSprinting;
}

void AFPSCharacter::UpdateMovementSpeed()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement || NormalWalkSpeed <= 0.0f) return;

    if (bIsSprinting)
    {
        Movement->MaxWalkSpeed = FMath::Max(SprintSpeed, NormalWalkSpeed);
        return;
    }

    Movement->MaxWalkSpeed = NormalWalkSpeed
        * (IsActivelyFiring() ? FiringMoveSpeedMultiplier : 1.0f);
}

float AFPSCharacter::GetCurrentWeaponSpreadDegrees() const
{
    const UCharacterMovementComponent* Movement = GetCharacterMovement();
    const float SpeedAlpha = Movement
        ? FMath::Clamp(Movement->Velocity.Size2D()
            / FMath::Max(NormalWalkSpeed, 1.0f), 0.0f, 1.0f)
        : 0.0f;
    const float AimProgress = GetAimProgress();
    const float MovementMultiplier = FMath::Lerp(1.0f, AimMoveSpreadMultiplier, AimProgress);
    const float BloomMultiplier = FMath::Lerp(1.0f, AimBloomMultiplier, AimProgress);
    const float AirSpread = Movement && Movement->IsFalling() ? AirSpreadDegrees : 0.0f;

    return FMath::Lerp(HipBaseSpreadDegrees, AimBaseSpreadDegrees, AimProgress)
        + MovingSpreadDegrees * SpeedAlpha * MovementMultiplier
        + AirSpread
        + CurrentShotBloomDegrees * BloomMultiplier;
}

float AFPSCharacter::GetAimProgress() const
{
    if (!FirstPersonCamera) return 0.0f;
    const float FOVRange = HipFOV - AimFOV;
    if (FOVRange <= KINDA_SMALL_NUMBER) return bIsAiming ? 1.0f : 0.0f;

    return FMath::Clamp(
        (HipFOV - FirstPersonCamera->FieldOfView) / FOVRange, 0.0f, 1.0f);
}

bool AFPSCharacter::IsAimViewSettled() const
{
    return bIsAiming && GetAimProgress() >= 0.75f;
}

void AFPSCharacter::UpdateShotBloom(float DeltaSeconds)
{
    if (CurrentShotBloomDegrees <= 0.0f || !GetWorld()) return;
    if (GetWorld()->GetTimeSeconds() - LastShotTime < SpreadRecoveryDelay) return;

    CurrentShotBloomDegrees = FMath::FInterpConstantTo(
        CurrentShotBloomDegrees, 0.0f, DeltaSeconds, SpreadRecoveryRate);
}

void AFPSCharacter::SetOpticVisible(bool bVisible)
{
    for (UStaticMeshComponent* Part : OpticFrameMeshes)
    {
        if (Part) Part->SetVisibility(bVisible);
    }
}

void AFPSCharacter::PlayFireAnimation()
{
    if (FireMontage)
    {
        if (UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance())
        {
            AnimInstance->Montage_Play(FireMontage, 1.0f);
        }
    }
    if (FireSound && !WeaponFireAudioPool.IsEmpty())
    {
        UAudioComponent* FireVoice = nullptr;
        for (int32 Offset = 0; Offset < WeaponFireAudioPool.Num(); ++Offset)
        {
            const int32 VoiceIndex = (NextFireAudioVoice + Offset) % WeaponFireAudioPool.Num();
            if (WeaponFireAudioPool[VoiceIndex] && !WeaponFireAudioPool[VoiceIndex]->IsPlaying())
            {
                FireVoice = WeaponFireAudioPool[VoiceIndex];
                NextFireAudioVoice = (VoiceIndex + 1) % WeaponFireAudioPool.Num();
                break;
            }
        }
        if (FireVoice)
        {
            FireVoice->SetSound(FireSound);
            FireVoice->SetPitchMultiplier(FMath::FRandRange(0.985f, 1.015f));
            FireVoice->Play();
        }
    }
}

void AFPSCharacter::PlayReloadAnimation()
{
    if (!ReloadAnimation) return;
    if (UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance())
    {
        AnimInstance->PlaySlotAnimationAsDynamicMontage(
            ReloadAnimation, TEXT("Arms"), 0.08f, 0.15f, 1.0f);
    }
}

void AFPSCharacter::PlayDryFireFeedback()
{
    PlayLocalFeedback(
        WeaponFeedbackAudio, EmptySound, 0.62f, FMath::FRandRange(0.96f, 1.04f));
    if (DryFireAnimation)
    {
        if (UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance())
        {
            AnimInstance->PlaySlotAnimationAsDynamicMontage(
                DryFireAnimation, TEXT("Arms"), 0.03f, 0.08f, 1.0f);
        }
    }
}

void AFPSCharacter::PlayLocalFeedback(
    UAudioComponent* AudioComponent, USoundBase* Sound, float Volume, float Pitch)
{
    if (!AudioComponent || !Sound || bIsDead) return;
    if (AudioComponent->IsPlaying()) AudioComponent->Stop();
    AudioComponent->SetSound(Sound);
    AudioComponent->SetVolumeMultiplier(Volume);
    AudioComponent->SetPitchMultiplier(Pitch);
    AudioComponent->Play();
}

bool AFPSCharacter::PlayPlayerVoice(USoundBase* Sound, bool bInterruptCurrent)
{
    if (!PlayerVoiceAudio || !Sound) return false;
    if (PlayerVoiceAudio->IsPlaying())
    {
        if (!bInterruptCurrent) return false;
        PlayerVoiceAudio->Stop();
    }
    PlayerVoiceAudio->SetSound(Sound);
    PlayerVoiceAudio->SetPitchMultiplier(FMath::FRandRange(0.98f, 1.02f));
    PlayerVoiceAudio->Play();
    return true;
}

void AFPSCharacter::RegisterHit(bool bKilledTarget)
{
    if (!GetWorld()) return;
    bLastHitWasKill = bKilledTarget;
    HitMarkerEndTime = GetWorld()->GetTimeSeconds()
        + (bKilledTarget ? KillMarkerDuration : HitMarkerDuration);
    PlayLocalFeedback(
        HitConfirmAudio, HitConfirmSound, bKilledTarget ? 0.72f : 0.42f,
        bKilledTarget ? 0.82f : 1.10f);
}

bool AFPSCharacter::IsHitMarkerVisible() const
{
    return GetWorld() && GetWorld()->GetTimeSeconds() < HitMarkerEndTime;
}

int32 AFPSCharacter::AddReserveAmmo(int32 Amount)
{
    if (Amount <= 0 || bIsDead) return 0;

    const int32 AvailableSpace = FMath::Max(0, MaxReserveAmmo - ReserveAmmo);
    const int32 ActualAmount = FMath::Min(Amount, AvailableSpace);
    ReserveAmmo += ActualAmount;
    return ActualAmount;
}

void AFPSCharacter::ShowPickupMessage(const FString& Message)
{
    if (Message.IsEmpty() || !GetWorld()) return;

    PickupMessage = Message;
    PickupMessageEndTime = GetWorld()->GetTimeSeconds() + PickupMessageDuration;
}

float AFPSCharacter::GetPickupMessageAlpha() const
{
    if (!GetWorld() || PickupMessage.IsEmpty()) return 0.0f;

    const float Remaining = PickupMessageEndTime - GetWorld()->GetTimeSeconds();
    if (Remaining <= 0.0f) return 0.0f;
    return FMath::Clamp(Remaining / 0.35f, 0.0f, 1.0f);
}

float AFPSCharacter::GetDamageFeedbackAlpha() const
{
    if (!GetWorld() || bIsDead) return 0.0f;
    return FMath::Clamp(
        (DamageFeedbackEndTime - GetWorld()->GetTimeSeconds())
            / FMath::Max(DamageFeedbackDuration, KINDA_SMALL_NUMBER),
        0.0f, 1.0f);
}

float AFPSCharacter::GetDeathElapsedTime() const
{
    return bIsDead && GetWorld()
        ? FMath::Max(0.0f, GetWorld()->GetTimeSeconds() - DeathStartTime)
        : 0.0f;
}

void AFPSCharacter::DisableMuzzleFlash()
{
    MuzzleFlashMesh->SetVisibility(false);
    MuzzleFlash->SetIntensity(0.0f);
}

void AFPSCharacter::HandleHealthChanged(float Health, float MaxHealth)
{
    (void)MaxHealth;
    const bool bTookDamage = Health < LastKnownHealth;
    LastKnownHealth = Health;
    if (!bTookDamage || bIsDead) return;

    if (GetWorld())
    {
        const float CurrentTime = GetWorld()->GetTimeSeconds();
        DamageFeedbackEndTime = CurrentTime + DamageFeedbackDuration;
        if (Health > 0.0f && CurrentTime >= NextHurtSoundTime)
        {
            if (PlayPlayerVoice(PlayerHurtSound, false))
            {
                NextHurtSoundTime = CurrentTime + HurtSoundCooldown;
            }
        }
    }
}

void AFPSCharacter::HandleDeath()
{
    if (bIsDead) return;
    bIsDead = true;
    bSprintHeld = false;
    SetSprinting(false);
    DeathStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    bWantsToFire = false;
    bIsAiming = false;
    bIsReloading = false;
    CurrentShotBloomDegrees = 0.0f;
    LastShotTime = -BIG_NUMBER;
    SetOpticVisible(false);
    StopFire();
    FirstPersonCamera->SetFieldOfView(HipFOV);
    FirstPersonCamera->FirstPersonFieldOfView = HipFirstPersonFOV;
    FirstPersonCamera->SetRelativeLocation(HipCameraRelativeLocation);
    for (UAudioComponent* FireVoice : WeaponFireAudioPool)
    {
        if (FireVoice) FireVoice->Stop();
    }
    if (WeaponFeedbackAudio) WeaponFeedbackAudio->Stop();
    if (HitConfirmAudio) HitConfirmAudio->Stop();
    PlayPlayerVoice(PlayerDeathSound, true);
    GetWorldTimerManager().ClearTimer(ReloadTimer);
    GetWorldTimerManager().ClearTimer(MuzzleFlashTimer);
    if (UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance())
    {
        AnimInstance->Montage_Stop(0.1f);
    }

    AController* OwningController = GetController();
    const FRotator DeathViewRotation = OwningController
        ? OwningController->GetControlRotation()
        : FirstPersonCamera->GetComponentRotation();
    FirstPersonCamera->bUsePawnControlRotation = false;
    FirstPersonCamera->SetWorldRotation(DeathViewRotation);
    bUseControllerRotationYaw = false;

    if (APlayerController* PlayerController = Cast<APlayerController>(OwningController))
    {
        PlayerController->SetIgnoreMoveInput(true);
        PlayerController->SetIgnoreLookInput(true);
    }

    if (DeathAnimation)
    {
        GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        GetMesh()->PlayAnimation(DeathAnimation, false);
    }

    DisableMuzzleFlash();
    StopJumping();
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
}
