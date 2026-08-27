#include "FPSEnemy.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "FPSDeathEffect.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AFPSEnemy::AFPSEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    VoiceAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudio"));
    VoiceAudio->SetupAttachment(GetRootComponent());
    VoiceAudio->bAutoActivate = false;
    VoiceAudio->bOverrideAttenuation = true;
    VoiceAudio->AttenuationOverrides.bAttenuate = true;
    VoiceAudio->AttenuationOverrides.bSpatialize = true;
    VoiceAudio->AttenuationOverrides.DistanceAlgorithm = EAttenuationDistanceModel::NaturalSound;
    VoiceAudio->AttenuationOverrides.AttenuationShape = EAttenuationShape::Sphere;
    VoiceAudio->AttenuationOverrides.AttenuationShapeExtents = FVector(160.0f, 0.0f, 0.0f);
    VoiceAudio->AttenuationOverrides.FalloffDistance = 2400.0f;
    VoiceAudio->SetVolumeMultiplier(0.9f);

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> QuinnMesh(
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
    static ConstructorHelpers::FClassFinder<UAnimInstance> UnarmedAnimBlueprint(
        TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
    if (QuinnMesh.Succeeded())
    {
        GetMesh()->SetSkeletalMeshAsset(QuinnMesh.Object);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        GetMesh()->SetVisibility(true);
    }
    if (UnarmedAnimBlueprint.Succeeded())
    {
        GetMesh()->SetAnimInstanceClass(UnarmedAnimBlueprint.Class);
    }

    static ConstructorHelpers::FObjectFinder<UAnimSequence> AttackAsset(
        TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01.MM_Attack_01"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> HitReactAsset(
        TEXT("/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Med_01.MM_HitReact_Front_Med_01"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> DeathAsset(
        TEXT("/Game/Characters/Mannequins/Anims/Death/MM_Death_Front_01.MM_Death_Front_01"));
    if (AttackAsset.Succeeded()) AttackAnimation = AttackAsset.Object;
    if (HitReactAsset.Succeeded()) HitReactAnimation = HitReactAsset.Object;
    if (DeathAsset.Succeeded()) DeathAnimation = DeathAsset.Object;

    const auto AddVoice = [](TArray<TObjectPtr<USoundBase>>& Target, const TCHAR* AssetPath)
    {
        ConstructorHelpers::FObjectFinder<USoundBase> VoiceAsset(AssetPath);
        if (VoiceAsset.Succeeded()) Target.Add(VoiceAsset.Object);
    };
    AddVoice(AlertSounds,
        TEXT("/Game/HumanVocalizations/HumanFemaleC/Wavs/voice_female_c_battle_shout_short_01.voice_female_c_battle_shout_short_01"));
    AddVoice(AttackSounds,
        TEXT("/Game/HumanVocalizations/HumanFemaleC/Wavs/voice_female_c_attack_01.voice_female_c_attack_01"));
    AddVoice(AttackSounds,
        TEXT("/Game/HumanVocalizations/HumanFemaleC/Wavs/voice_female_c_attack_05.voice_female_c_attack_05"));
    AddVoice(HurtSounds,
        TEXT("/Game/HumanVocalizations/HumanFemaleC/Wavs/voice_female_c_hurt_pain_12.voice_female_c_hurt_pain_12"));
    AddVoice(HurtSounds,
        TEXT("/Game/HumanVocalizations/HumanFemaleC/Wavs/voice_female_c_hurt_pain_01.voice_female_c_hurt_pain_01"));
    AddVoice(HurtSounds,
        TEXT("/Game/HumanVocalizations/HumanFemaleC/Wavs/voice_female_c_hurt_pain_06.voice_female_c_hurt_pain_06"));
    AddVoice(HurtSounds,
        TEXT("/Game/HumanVocalizations/HumanFemaleC/Wavs/voice_female_c_hurt_pain_07.voice_female_c_hurt_pain_07"));
    AddVoice(HurtSounds,
        TEXT("/Game/HumanVocalizations/HumanFemaleC/Wavs/voice_female_c_hurt_pain_02.voice_female_c_hurt_pain_02"));
    AddVoice(DeathSounds,
        TEXT("/Game/HumanVocalizations/HumanFemaleC/Wavs/voice_female_c_death_05.voice_female_c_death_05"));
    AddVoice(DeathSounds,
        TEXT("/Game/HumanVocalizations/HumanFemaleC/Wavs/voice_female_c_death_02.voice_female_c_death_02"));
    AddVoice(DeathSounds,
        TEXT("/Game/HumanVocalizations/HumanFemaleC/Wavs/voice_female_c_death_01.voice_female_c_death_01"));
    AddVoice(DeathSounds,
        TEXT("/Game/HumanVocalizations/HumanFemaleC/Wavs/voice_female_c_death_06.voice_female_c_death_06"));

    GetCharacterMovement()->MaxWalkSpeed = 300.0f;
    GetCharacterMovement()->bRunPhysicsWithNoController = true;
    GetCharacterMovement()->bUseRVOAvoidance = true;
    GetCharacterMovement()->AvoidanceConsiderationRadius = 180.0f;
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AFPSEnemy::BeginPlay()
{
    Super::BeginPlay();
    HealthComponent->OnDeath.AddDynamic(this, &AFPSEnemy::HandleDeath);
    HealthComponent->OnHealthChanged.AddDynamic(this, &AFPSEnemy::HandleHealthChanged);
    AvoidanceSign = (GetUniqueID() % 2 == 0) ? 1.0f : -1.0f;
}

void AFPSEnemy::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bIsDead) return;
    TimeUntilNextAttack = FMath::Max(0.0f, TimeUntilNextAttack - DeltaSeconds);
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return;

    const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
    const float Distance = ToPlayer.Size();
    if (Distance > DetectionRange) return;
    if (!bHasDetectedPlayer)
    {
        bHasDetectedPlayer = true;
        if (FMath::FRand() <= AlertVoiceChance)
        {
            PlayRandomVoice(AlertSounds, false);
        }
    }
    if (Distance > AttackRange)
    {
        FVector MoveDirection = ToPlayer.GetSafeNormal2D();
        FHitResult ObstacleHit;
        FCollisionQueryParams ObstacleParams(SCENE_QUERY_STAT(EnemyObstacleProbe), false, this);
        const FVector ProbeStart = GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
        const FVector ProbeEnd = ProbeStart + MoveDirection * 130.0f;
        if (GetWorld()->SweepSingleByChannel(
            ObstacleHit, ProbeStart, ProbeEnd, FQuat::Identity, ECC_Visibility,
            FCollisionShape::MakeSphere(36.0f), ObstacleParams))
        {
            if (ObstacleHit.GetActor() != Player)
            {
                const FVector SideDirection =
                    FVector::CrossProduct(FVector::UpVector, MoveDirection) * AvoidanceSign;
                MoveDirection = (SideDirection + ObstacleHit.Normal * 0.35f).GetSafeNormal2D();
            }
        }
        AddMovementInput(MoveDirection);
        SetActorRotation(FRotator(0.0f, MoveDirection.Rotation().Yaw, 0.0f));
    }
    else TryAttack();
}

void AFPSEnemy::TryAttack()
{
    if (TimeUntilNextAttack > 0.0f) return;
    if (APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
    {
        FHitResult SightHit;
        FCollisionQueryParams SightParams(SCENE_QUERY_STAT(EnemyMeleeSight), true, this);
        const bool bHit = GetWorld()->LineTraceSingleByChannel(
            SightHit, GetActorLocation(), Player->GetActorLocation(), ECC_Visibility, SightParams);
        if (!bHit || SightHit.GetActor() != Player) return;

        if (Player->FindComponentByClass<UHealthComponent>())
        {
            if (AttackAnimation)
            {
                if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
                {
                    AnimInstance->PlaySlotAnimationAsDynamicMontage(
                        AttackAnimation, TEXT("DefaultSlot"), 0.1f, 0.15f);
                }
            }
            if (FMath::FRand() <= AttackVoiceChance)
            {
                PlayRandomVoice(AttackSounds, false);
            }
            TimeUntilNextAttack = AttackCooldown;
            GetWorldTimerManager().SetTimer(
                AttackDamageTimer, this, &AFPSEnemy::ApplyMeleeDamage, 0.28f, false);
        }
    }
}

void AFPSEnemy::ApplyMeleeDamage()
{
    if (bIsDead) return;
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player || FVector::Dist2D(Player->GetActorLocation(), GetActorLocation()) > AttackRange + 35.0f)
    {
        return;
    }

    FHitResult SightHit;
    FCollisionQueryParams SightParams(SCENE_QUERY_STAT(EnemyMeleeDamageSight), true, this);
    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        SightHit, GetActorLocation(), Player->GetActorLocation(), ECC_Visibility, SightParams);
    if (bHit && SightHit.GetActor() != Player) return;

    if (UHealthComponent* PlayerHealth = Player->FindComponentByClass<UHealthComponent>())
    {
        PlayerHealth->ApplyDamage(AttackDamage);
    }
}

void AFPSEnemy::HandleHealthChanged(float Health, float MaxHealth)
{
    (void)MaxHealth;
    if (Health <= 0.0f) return;
    if (GetWorld() && GetWorld()->GetTimeSeconds() >= NextHurtVoiceTime
        && FMath::FRand() <= HurtVoiceChance)
    {
        if (PlayRandomVoice(HurtSounds, false))
        {
            NextHurtVoiceTime = GetWorld()->GetTimeSeconds() + HurtVoiceCooldown;
        }
    }
    if (!HitReactAnimation) return;
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        AnimInstance->PlaySlotAnimationAsDynamicMontage(
            HitReactAnimation, TEXT("DefaultSlot"), 0.05f, 0.1f);
    }
}

void AFPSEnemy::HandleDeath()
{
    if (bIsDead) return;
    bIsDead = true;
    GetWorldTimerManager().ClearTimer(AttackDamageTimer);
    SetActorTickEnabled(false);
    GetCharacterMovement()->DisableMovement();
    SetActorEnableCollision(false);
    if (UWorld* World = GetWorld())
    {
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        World->SpawnActor<AFPSDeathEffect>(
            AFPSDeathEffect::StaticClass(),
            GetActorLocation() + FVector(0.0f, 0.0f, 42.0f),
            FRotator::ZeroRotator,
            SpawnParameters);
    }
    float CorpseLifetime = 2.5f;
    if (USoundBase* DeathSound = PlayRandomVoice(DeathSounds, true))
    {
        CorpseLifetime = FMath::Max(CorpseLifetime, DeathSound->GetDuration() + 0.25f);
    }
    if (DeathAnimation)
    {
        GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        GetMesh()->PlayAnimation(DeathAnimation, false);
        CorpseLifetime = FMath::Max(CorpseLifetime, DeathAnimation->GetPlayLength() + 0.35f);
    }
    OnEnemyDied.Broadcast(this);
    SetLifeSpan(CorpseLifetime);
}

USoundBase* AFPSEnemy::PlayRandomVoice(
    const TArray<TObjectPtr<USoundBase>>& Sounds, bool bInterruptCurrent)
{
    if (!VoiceAudio || Sounds.IsEmpty()) return nullptr;
    if (VoiceAudio->IsPlaying())
    {
        if (!bInterruptCurrent) return nullptr;
        VoiceAudio->Stop();
    }

    const int32 SoundIndex = FMath::RandRange(0, Sounds.Num() - 1);
    USoundBase* SelectedSound = Sounds[SoundIndex];
    if (!SelectedSound) return nullptr;

    VoiceAudio->SetSound(SelectedSound);
    VoiceAudio->SetPitchMultiplier(FMath::FRandRange(0.97f, 1.03f));
    VoiceAudio->Play();
    return SelectedSound;
}
