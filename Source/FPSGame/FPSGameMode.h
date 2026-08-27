#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameMode.generated.h"

UCLASS()
class FPSGAME_API AFPSGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AFPSGameMode();

    virtual void InitGame(
        const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

    UFUNCTION(BlueprintPure, Category="Match") int32 GetCurrentWave() const { return CurrentWave; }
    UFUNCTION(BlueprintPure, Category="Match") int32 GetEnemiesRemaining() const { return EnemiesRemaining; }
    UFUNCTION(BlueprintPure, Category="Match") int32 GetScore() const { return Score; }
    UFUNCTION(BlueprintPure, Category="Match") bool IsGameOver() const { return bGameOver; }
    UFUNCTION(BlueprintPure, Category="Match") bool IsWaitingForStart() const { return bWaitingForStart; }

    void RequestStartGame();
    void RequestRestart();
    void RequestMainMenu();

protected:
    virtual void BeginPlay() override;

private:
    void BuildArena();
    void SpawnArenaBlock(const FVector& Location, const FVector& Scale) const;
    void StartNextWave();
    void BindPlayerHealth();
    void ShowDeathMenu();
    void TrySpawnEnemyDrop(class AFPSEnemy* Enemy, bool bWaveCleared);
    bool FindEnemySpawnLocation(
        const FVector& SpawnCenter, int32 EnemyIndex, int32 EnemyCount, FVector& OutLocation) const;

    UFUNCTION() void HandleEnemyDied(class AFPSEnemy* Enemy);
    UFUNCTION() void HandlePlayerDied();

    UPROPERTY() TObjectPtr<class UStaticMesh> ArenaCube;
    UPROPERTY(EditDefaultsOnly, Category="Match") int32 BaseEnemiesPerWave = 3;
    UPROPERTY(EditDefaultsOnly, Category="Match") float TimeBetweenWaves = 2.5f;
    UPROPERTY(EditDefaultsOnly, Category="Match", meta=(ClampMin="0.0"))
    float DeathMenuDelay = 1.15f;
    UPROPERTY(EditDefaultsOnly, Category="Match") float MinEnemySpawnRadius = 800.0f;
    UPROPERTY(EditDefaultsOnly, Category="Match") float MaxEnemySpawnRadius = 1350.0f;
    UPROPERTY(EditDefaultsOnly, Category="Pickups", meta=(ClampMin="0.0", ClampMax="1.0"))
    float EnemyPickupDropChance = 0.42f;
    UPROPERTY(EditDefaultsOnly, Category="Pickups", meta=(ClampMin="0"))
    int32 AmmoPickupAmount = 30;
    UPROPERTY(EditDefaultsOnly, Category="Pickups", meta=(ClampMin="0.0"))
    float HealthPickupAmount = 25.0f;
    UPROPERTY(EditDefaultsOnly, Category="Pickups", meta=(ClampMin="0"))
    int32 WaveSupplyAmmoAmount = 45;
    UPROPERTY(EditDefaultsOnly, Category="Pickups", meta=(ClampMin="0.0"))
    float WaveSupplyHealthAmount = 30.0f;
    UPROPERTY(EditDefaultsOnly, Category="Pickups", meta=(ClampMin="1.0"))
    float EnemyPickupLifetime = 24.0f;
    UPROPERTY(EditDefaultsOnly, Category="Pickups", meta=(ClampMin="1.0"))
    float WaveSupplyLifetime = 38.0f;

    int32 CurrentWave = 0;
    int32 EnemiesRemaining = 0;
    int32 Score = 0;
    bool bMatchStarted = false;
    bool bGameOver = false;
    bool bWaitingForStart = true;
    FTimerHandle WaveTimer;
    FTimerHandle BindPlayerTimer;
    FTimerHandle DeathMenuTimer;
};
