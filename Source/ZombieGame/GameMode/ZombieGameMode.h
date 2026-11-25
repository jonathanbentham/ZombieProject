#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZombieGame/Characters/FrontDoor/FrontDoor.h"
#include "ZombieGame/Characters/Zombie/Zombie.h"
#include "ZombieGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKFOnDoorSpawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKFOnDoorOpen);


/**
 * 
 */
UCLASS()
class ZOMBIEGAME_API AZombieGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FKFOnDoorSpawn OnDoorSpawn;

	UPROPERTY(BlueprintAssignable)
	FKFOnDoorOpen OnDoorOpen;

	// Getter for zombies left
	int GetZombiesLeft() const { return ZombiesLeft; }

	// Getter for zombies left
	int GetCurrentWave() const { return CurrentWave; }

	// Getter for zombies left
	int GetMaxWaves() const { return MaxWaves; }

	// This function is used in Zombie.cpp, so is kept public
	void HandleZombieCountAndRound();

	// A reference to the behaviour tree for the zombies
	UPROPERTY(EditAnywhere)
	class UBehaviorTree* BehaviorTree;

	UPROPERTY()
	TArray<AZombie*> SpawnedZombies;


protected:
	virtual void BeginPlay() override;

	// All zombies are in protected as they need to be set in blueprint
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	TSubclassOf<APawn> ZombiePawn1;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	TSubclassOf<APawn> ZombiePawn2;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	TSubclassOf<APawn> ZombiePawn3;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	TSubclassOf<APawn> ZombiePawn4;
	
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	TSubclassOf<APawn> FireZombiePawn;

	
	// uses a getter to be used in widget, protected so the wave can be edited in BP
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CurrentWave = 5;

	// uses a getter to be used in widget, protected so the wave can be edited in BP
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxWaves = 100;

	// Corners of the world-space box that zombies spawn in
	UPROPERTY(EditAnywhere, Category = "Spawning")
	FVector SpawnAreaMin = FVector(-40870.0, 392000.0, -379670.0);

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FVector SpawnAreaMax = FVector(-63790.0, 409560.0, -378590.0);

	// How many random points to try per zombie before giving up
	UPROPERTY(EditAnywhere, Category = "Spawning")
	int32 MaxSpawnAttempts = 50;

	// How far above and below each point to sweep for the ground
	UPROPERTY(EditAnywhere, Category = "Spawning")
	float SpawnTraceHalfHeight = 6500.f;

	// functionality in blueprint
	UFUNCTION(BlueprintImplementableEvent)
	void SpawnAmmo();

private:
	// functions
	void StartGame();
	void SpawnZombies();
	bool FindSpawnLocation(const FBox& SpawnArea, FVector& OutLocation) const;
	void HandleWaveStart();
	void WaveIncrement();
	void PlayerWins();
	
	// variables
	int ZombieTotal;
	int ZombiesLeft; // uses a getter, as it's used in MainWidget.cpp
	TArray<TSubclassOf<APawn>> ZombiePawnClasses;
};
