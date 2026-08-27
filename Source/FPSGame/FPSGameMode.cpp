#include "FPSGameMode.h"
#include "FPSCharacter.h"
#include "FPSEnemy.h"
#include "FPSHUD.h"
#include "FPSPickup.h"
#include "FPSPlayerController.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AFPSGameMode::AFPSGameMode()
{
    DefaultPawnClass = AFPSCharacter::StaticClass();
    HUDClass = AFPSHUD::StaticClass();
    PlayerControllerClass = AFPSPlayerController::StaticClass();

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded()) ArenaCube = CubeMesh.Object;
}

void AFPSGameMode::InitGame(
    const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    bWaitingForStart = !UGameplayStatics::HasOption(Options, TEXT("AutoStart"));
}

void AFPSGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    if (!NewPlayer) return;
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);
    if (!NewPlayer->GetPawn())
    {
        const FTransform SpawnTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 110.0f));
        RestartPlayerAtTransform(NewPlayer, SpawnTransform);
    }
}

void AFPSGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (!GetWorld()) return;

    if (UGameplayStatics::GetCurrentLevelName(this, true) == TEXT("Entry"))
    {
        BuildArena();
    }
    BindPlayerHealth();
}

void AFPSGameMode::BuildArena()
{
    SpawnArenaBlock(FVector(0.0f, 0.0f, -50.0f), FVector(40.0f, 40.0f, 1.0f));
    SpawnArenaBlock(FVector(2000.0f, 0.0f, 250.0f), FVector(1.0f, 40.0f, 6.0f));
    SpawnArenaBlock(FVector(-2000.0f, 0.0f, 250.0f), FVector(1.0f, 40.0f, 6.0f));
    SpawnArenaBlock(FVector(0.0f, 2000.0f, 250.0f), FVector(40.0f, 1.0f, 6.0f));
    SpawnArenaBlock(FVector(0.0f, -2000.0f, 250.0f), FVector(40.0f, 1.0f, 6.0f));

    ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(
        FVector::ZeroVector, FRotator(-55.0f, -35.0f, 0.0f));
    if (Sun)
    {
        Sun->GetLightComponent()->SetMobility(EComponentMobility::Movable);
        Sun->GetLightComponent()->SetIntensity(8.0f);
        Sun->GetLightComponent()->SetLightColor(FLinearColor(1.0f, 0.88f, 0.72f));
    }

    APointLight* FillLight = GetWorld()->SpawnActor<APointLight>(
        FVector(0.0f, 0.0f, 1200.0f), FRotator::ZeroRotator);
    if (FillLight)
    {
        if (UPointLightComponent* PointComponent = Cast<UPointLightComponent>(FillLight->GetLightComponent()))
        {
            PointComponent->SetMobility(EComponentMobility::Movable);
            PointComponent->SetIntensity(80000.0f);
            PointComponent->SetAttenuationRadius(5000.0f);
        }
    }
}

void AFPSGameMode::SpawnArenaBlock(const FVector& Location, const FVector& Scale) const
{
    if (!ArenaCube || !GetWorld()) return;
    AStaticMeshActor* Block = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (!Block) return;

    UStaticMeshComponent* Mesh = Block->GetStaticMeshComponent();
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetStaticMesh(ArenaCube);
    Mesh->SetWorldScale3D(Scale);
    Mesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AFPSGameMode::BindPlayerHealth()
{
    AFPSCharacter* Player = Cast<AFPSCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player)
    {
        GetWorldTimerManager().SetTimer(BindPlayerTimer, this, &AFPSGameMode::BindPlayerHealth, 0.1f, false);
        return;
    }

    Player->GetHealthComponent()->OnDeath.AddUniqueDynamic(this, &AFPSGameMode::HandlePlayerDied);
    if (!bWaitingForStart && !bMatchStarted)
    {
        bMatchStarted = true;
        StartNextWave();
    }
}

void AFPSGameMode::StartNextWave()
{
    if (bGameOver || !GetWorld()) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player)
    {
        GetWorldTimerManager().SetTimer(WaveTimer, this, &AFPSGameMode::StartNextWave, 0.1f, false);
        return;
    }

    ++CurrentWave;
    const int32 EnemyCount = BaseEnemiesPerWave + (CurrentWave - 1) * 2;
    EnemiesRemaining = EnemyCount;
    const FVector SpawnCenter = Player->GetActorLocation();

    for (int32 Index = 0; Index < EnemyCount; ++Index)
    {
        FVector Location;
        FindEnemySpawnLocation(SpawnCenter, Index, EnemyCount, Location);
        const float FacingYaw = (SpawnCenter - Location).Rotation().Yaw;
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        AFPSEnemy* Enemy = GetWorld()->SpawnActor<AFPSEnemy>(
            AFPSEnemy::StaticClass(), Location, FRotator(0.0f, FacingYaw, 0.0f), SpawnParameters);
        if (Enemy)
        {
            Enemy->OnEnemyDied.AddDynamic(this, &AFPSGameMode::HandleEnemyDied);
        }
        else
        {
            --EnemiesRemaining;
        }
    }

    if (EnemiesRemaining == 0)
    {
        GetWorldTimerManager().SetTimer(WaveTimer, this, &AFPSGameMode::StartNextWave, TimeBetweenWaves, false);
    }
}

bool AFPSGameMode::FindEnemySpawnLocation(
    const FVector& SpawnCenter, int32 EnemyIndex, int32 EnemyCount, FVector& OutLocation) const
{
    if (!GetWorld() || EnemyCount <= 0) return false;

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemySpawn), false);
    if (APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
    {
        QueryParams.AddIgnoredActor(Player);
    }

    constexpr int32 MaxAttempts = 12;
    for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
    {
        const float BaseAngle = 2.0f * PI * static_cast<float>(EnemyIndex) / EnemyCount;
        const float Angle = BaseAngle + CurrentWave * 0.35f + Attempt * 0.73f;
        const float RadiusAlpha = static_cast<float>((EnemyIndex * 7 + Attempt * 3) % 11) / 10.0f;
        const float Radius = FMath::Lerp(MinEnemySpawnRadius, MaxEnemySpawnRadius, RadiusAlpha);
        const FVector Candidate = SpawnCenter + FVector(
            FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);

        FHitResult GroundHit;
        const FVector TraceStart = Candidate + FVector(0.0f, 0.0f, 1000.0f);
        const FVector TraceEnd = Candidate - FVector(0.0f, 0.0f, 1800.0f);
        if (!GetWorld()->LineTraceSingleByChannel(
            GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
        {
            continue;
        }
        if (GroundHit.ImpactNormal.Z < 0.65f) continue;

        const FVector GroundLocation = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 92.0f);
        const bool bBlocked = GetWorld()->OverlapBlockingTestByChannel(
            GroundLocation, FQuat::Identity, ECC_Pawn,
            FCollisionShape::MakeCapsule(45.0f, 88.0f), QueryParams);
        if (!bBlocked)
        {
            OutLocation = GroundLocation;
            return true;
        }
    }

    const float FallbackAngle = 2.0f * PI * static_cast<float>(EnemyIndex) / EnemyCount;
    OutLocation = SpawnCenter + FVector(
        FMath::Cos(FallbackAngle) * MinEnemySpawnRadius,
        FMath::Sin(FallbackAngle) * MinEnemySpawnRadius,
        100.0f);
    return false;
}

void AFPSGameMode::HandleEnemyDied(AFPSEnemy* Enemy)
{
    if (bGameOver || !Enemy) return;
    Score += 100;
    EnemiesRemaining = FMath::Max(0, EnemiesRemaining - 1);
    TrySpawnEnemyDrop(Enemy, EnemiesRemaining == 0);
    if (EnemiesRemaining == 0)
    {
        GetWorldTimerManager().SetTimer(WaveTimer, this, &AFPSGameMode::StartNextWave, TimeBetweenWaves, false);
    }
}

void AFPSGameMode::TrySpawnEnemyDrop(AFPSEnemy* Enemy, bool bWaveCleared)
{
    if (!Enemy || !GetWorld()) return;

    AFPSCharacter* Player = Cast<AFPSCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player || Player->IsDead()) return;

    UHealthComponent* Health = Player->GetHealthComponent();
    const float HealthNeed = Health && Health->GetMaxHealth() > 0.0f
        ? 1.0f - Health->GetHealth() / Health->GetMaxHealth()
        : 0.0f;
    const float AmmoNeed = Player->GetMaxReserveAmmo() > 0
        ? 1.0f - static_cast<float>(Player->GetReserveAmmo()) / Player->GetMaxReserveAmmo()
        : 0.0f;

    EFPSPickupType PickupType = EFPSPickupType::Supply;
    if (!bWaveCleared)
    {
        if (FMath::FRand() > EnemyPickupDropChance) return;

        const float ClampedHealthNeed = FMath::Clamp(HealthNeed, 0.0f, 1.0f);
        const float ClampedAmmoNeed = FMath::Clamp(AmmoNeed, 0.0f, 1.0f);
        const float TotalNeed = ClampedHealthNeed + ClampedAmmoNeed;
        if (TotalNeed <= KINDA_SMALL_NUMBER) return;

        PickupType = FMath::FRandRange(0.0f, TotalNeed) < ClampedHealthNeed
            ? EFPSPickupType::Health
            : EFPSPickupType::Ammo;
    }

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PickupGround), false);
    QueryParams.AddIgnoredActor(Enemy);
    QueryParams.AddIgnoredActor(Player);
    FCollisionObjectQueryParams GroundObjectTypes;
    GroundObjectTypes.AddObjectTypesToQuery(ECC_WorldStatic);
    FHitResult GroundHit;
    const FVector TraceStart = Enemy->GetActorLocation() + FVector(0.0f, 0.0f, 120.0f);
    const FVector TraceEnd = Enemy->GetActorLocation() - FVector(0.0f, 0.0f, 450.0f);
    FVector SpawnLocation = Enemy->GetActorLocation() - FVector(0.0f, 0.0f, 72.0f);
    if (GetWorld()->LineTraceSingleByObjectType(
        GroundHit, TraceStart, TraceEnd, GroundObjectTypes, QueryParams))
    {
        SpawnLocation = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 18.0f);
    }

    const FTransform SpawnTransform(
        FRotator(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f), SpawnLocation);
    AFPSPickup* Pickup = GetWorld()->SpawnActorDeferred<AFPSPickup>(
        AFPSPickup::StaticClass(), SpawnTransform, nullptr, nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!Pickup) return;

    const int32 AmmoAmount = PickupType == EFPSPickupType::Supply
        ? WaveSupplyAmmoAmount
        : (PickupType == EFPSPickupType::Ammo ? AmmoPickupAmount : 0);
    const float HealAmount = PickupType == EFPSPickupType::Supply
        ? WaveSupplyHealthAmount
        : (PickupType == EFPSPickupType::Health ? HealthPickupAmount : 0.0f);
    const float Lifetime = PickupType == EFPSPickupType::Supply
        ? WaveSupplyLifetime
        : EnemyPickupLifetime;
    Pickup->InitializePickup(PickupType, AmmoAmount, HealAmount, Lifetime);
    UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
}

void AFPSGameMode::HandlePlayerDied()
{
    bGameOver = true;
    GetWorldTimerManager().ClearTimer(WaveTimer);
    GetWorldTimerManager().ClearTimer(BindPlayerTimer);
    GetWorldTimerManager().SetTimer(
        DeathMenuTimer, this, &AFPSGameMode::ShowDeathMenu, DeathMenuDelay, false);
}

void AFPSGameMode::ShowDeathMenu()
{
    if (AFPSPlayerController* PlayerController =
        Cast<AFPSPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
    {
        PlayerController->ShowDeathMenu(Score);
    }
}

void AFPSGameMode::RequestStartGame()
{
    if (bGameOver || !bWaitingForStart) return;

    bWaitingForStart = false;
    BindPlayerHealth();
}

void AFPSGameMode::RequestRestart()
{
    if (GetWorld())
    {
        UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/FirstPerson/Lvl_FirstPerson")), true,
            TEXT("game=/Script/FPSGame.FPSGameMode?AutoStart=1"));
    }
}

void AFPSGameMode::RequestMainMenu()
{
    if (GetWorld())
    {
        UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/FirstPerson/Lvl_FirstPerson")), true,
            TEXT("game=/Script/FPSGame.FPSGameMode"));
    }
}
