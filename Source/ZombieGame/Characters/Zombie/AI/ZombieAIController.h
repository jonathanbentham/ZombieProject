#pragma once

#include "CoreMinimal.h"
#include "Perception/AIPerceptionComponent.h"
#include "AIController.h"
#include "ZombieAIController.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIEGAME_API AZombieAIController : public AAIController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	AZombieAIController();

	UPROPERTY(VisibleAnywhere, Category="AI", meta=(AllowPrivateAccess="true"))
	UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(EditAnywhere)
	class UBehaviorTree* AIBehavior;

	UFUNCTION()
	void EnemyDetected(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void InitializeGrandadBlackboardValue();

	// What a zombie goes for when it can't see the player: a closed front door, otherwise Grandad
	AActor* GetDefaultTarget() const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Blackboard)
	AActor* Target;

	UFUNCTION()
	void HandleDoorSpawned();

	// Called when the player opens or closes the front door
	UFUNCTION()
	void HandleDoorToggled();
	
};
