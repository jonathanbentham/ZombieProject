#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "AIController.h"
#include "BTTask_FireAtZombie.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIEGAME_API UBTTask_FireAtZombie : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
protected:
	UBTTask_FireAtZombie();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "AI")
    UEnvQuery *FindZombieEQS;

	UBehaviorTreeComponent *CachedOwnerComp; // Member variable to store OwnerComp

	AAIController* AIController;
};
