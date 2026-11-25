
#include "BTTask_FindClosestZombie.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Zombie/Zombie.h"

UBTTask_FindClosestZombie::UBTTask_FindClosestZombie()
{
	NodeName = TEXT("FindClosestZombie");

	// The dog and turret share this task, and it keeps state between ExecuteTask and the
	// async query callback, so each AI needs its own instance
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_FindClosestZombie::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn() || !FindZombieEQS)
	{
		return EBTNodeResult::Failed;
	}

	// Stored so the async query callback can finish this task
	CachedOwnerComp = &OwnerComp;

	FEnvQueryRequest FindZombieRequest(FindZombieEQS, AIController->GetPawn());
	FindZombieRequest.Execute(EEnvQueryRunMode::SingleResult, this, &UBTTask_FindClosestZombie::ZombieQueryResult);
	return EBTNodeResult::InProgress;
}

void UBTTask_FindClosestZombie::ZombieQueryResult(TSharedPtr<FEnvQueryResult> Result)
{
	if (!CachedOwnerComp)
	{
		return;
	}

	AZombie *ClosestZombie = nullptr;
	if (Result.IsValid() && Result->IsSuccessful() && Result->Items.Num() > 0)
	{
		ClosestZombie = Cast<AZombie>(Result->GetItemAsActor(0));
	}

	// A latent task must always be finished, otherwise the behaviour tree waits here forever
	if (!ClosestZombie)
	{
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);
		return;
	}

	CachedOwnerComp->GetBlackboardComponent()->SetValueAsObject(TEXT("Zombie"), ClosestZombie);
	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}
