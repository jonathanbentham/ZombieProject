#include "ZombieGameMode.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "ZombieGame/Characters/PlayerCharacter/ZombieGameCharacter.h"
#include "Components/StaticMeshComponent.h"

void AZombieGameMode::BeginPlay()
{
    Super::BeginPlay();

    StartGame();
}

void AZombieGameMode::StartGame()
{
    SpawnZombies();
}

void AZombieGameMode::SpawnZombies()
{
    ZombiePawnClasses = {ZombiePawn1, ZombiePawn2, ZombiePawn3, ZombiePawn4};

    // Every fifth wave is a boss wave: fewer zombies, but they are all fire bosses
    const bool bIsBossWave = CurrentWave % 5 == 0;
    ZombieTotal = bIsBossWave ? CurrentWave / 5 : CurrentWave * 4;

    const FBox SpawnArea(SpawnAreaMin, SpawnAreaMax);
    int32 ZombiesSpawned = 0;

    for (int32 i = 0; i < ZombieTotal; i++)
    {
        FVector SpawnLocation;
        if (!FindSpawnLocation(SpawnArea, SpawnLocation))
        {
            UE_LOG(LogTemp, Warning, TEXT("SpawnZombies: no valid spawn location after %d attempts"), MaxSpawnAttempts);
            continue;
        }

        const TSubclassOf<APawn> SpawnPawnClass = bIsBossWave
            ? FireZombiePawn
            : ZombiePawnClasses[FMath::RandRange(0, ZombiePawnClasses.Num() - 1)];

        APawn *SpawnedPawn = UAIBlueprintHelperLibrary::SpawnAIFromClass(GetWorld(), SpawnPawnClass, BehaviorTree, SpawnLocation);
        if (!SpawnedPawn)
        {
            continue;
        }

        ZombiesSpawned++;
        if (AZombie *SpawnedZombie = Cast<AZombie>(SpawnedPawn))
        {
            SpawnedZombies.Add(SpawnedZombie);
        }
    }

    // Only count zombies that actually spawned, otherwise the wave could never end
    ZombiesLeft = ZombiesSpawned;
    if (ZombiesLeft == 0)
    {
        WaveIncrement();
    }
}

bool AZombieGameMode::FindSpawnLocation(const FBox &SpawnArea, FVector &OutLocation) const
{
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    for (int32 Attempt = 0; Attempt < MaxSpawnAttempts; Attempt++)
    {
        const FVector Candidate = FMath::RandPointInBox(SpawnArea);
        const FVector SweepStart = Candidate + FVector(0.f, 0.f, SpawnTraceHalfHeight);
        const FVector SweepEnd = Candidate - FVector(0.f, 0.f, SpawnTraceHalfHeight);

        // A sphere sweep covers more ground than a line trace, which often missed and spawned zombies under the map
        FHitResult Hit;
        if (!GetWorld()->SweepSingleByChannel(Hit, SweepStart, SweepEnd, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(75.f), QueryParams))
        {
            continue;
        }

        // Skip static meshes (houses, props) so zombies only spawn on open ground
        if (Hit.GetComponent() && Hit.GetComponent()->IsA<UStaticMeshComponent>())
        {
            continue;
        }

        OutLocation = FVector(Candidate.X, Candidate.Y, Hit.Location.Z);
        return true;
    }
    return false;
}

void AZombieGameMode::HandleZombieCountAndRound()
{
    ZombiesLeft--; // zombies left -1
    if (ZombiesLeft == 0)
    {
        WaveIncrement();
    }
}

void AZombieGameMode::WaveIncrement()
{
    if (CurrentWave == MaxWaves)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player has won the game!"));
        PlayerWins();
    }
    else
    {

        // Sets a timer of 10 seconds for both HandleWaveStart
        FTimerHandle RoundStartTimerHandle;
        FTimerDelegate TimerCallback;
        TimerCallback.BindLambda([this]()
                                 { HandleWaveStart(); });
        GetWorld()->GetTimerManager().SetTimer(RoundStartTimerHandle, TimerCallback, 10.f, false);
    }
}

void AZombieGameMode::HandleWaveStart()
{
    CurrentWave ++;
    SpawnZombies();
    SpawnAmmo();

    ACharacter *PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    const AZombieGameCharacter *Character = Cast<AZombieGameCharacter>(PlayerCharacter);
   
    if (Character->MainWidgetInstance)
    {
        Character->MainWidgetInstance->ShowWaveStart(CurrentWave);
    }
}

void AZombieGameMode::PlayerWins()
{
    ACharacter *PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    AZombieGameCharacter *Character = Cast<AZombieGameCharacter>(PlayerCharacter);
    APlayerController *PlayerController = Cast<APlayerController>(Character->GetController());
    if (Character->MainWidgetInstance)
    {
        Character->GetCharacterMovement()->StopMovementImmediately();
        PlayerController->SetInputMode(FInputModeUIOnly());
        PlayerController->bShowMouseCursor = true;
        Character->MainWidgetInstance->ShowWinText();
    }
}