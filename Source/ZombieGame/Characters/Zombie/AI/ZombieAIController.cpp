#include "ZombieAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ZombieGame/Characters/Zombie/FireZombieBoss.h"
#include "ZombieGame/Characters/Grandad/Grandad.h"
#include "ZombieGame/Characters/PlayerCharacter/ZombieGameCharacter.h"
#include "ZombieGame/GameMode/ZombieGameMode.h"
#include "ZombieGame/Characters/FrontDoor/FrontDoor.h"
#include "ZombieGame/Widgets/ShopItem.h"

namespace ZombieBlackboardKeys
{
    const FName PlayerCharacter(TEXT("PlayerCharacter"));
    const FName LastKnownPlayerLocation(TEXT("LastKnownPlayerLocation"));
    const FName DoorLocation(TEXT("DoorLocation"));
    const FName GrandadLocation(TEXT("GrandadLocation"));
}
namespace ZombieKeys = ZombieBlackboardKeys;

AZombieAIController::AZombieAIController()
{
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
}

void AZombieAIController::BeginPlay()
{
    Super::BeginPlay();

    // Start the behaviour tree first, so the blackboard exists before any keys are set
    if (AIBehavior)
    {
        RunBehaviorTree(AIBehavior);
    }

    if (AZombieGameMode *GameMode = Cast<AZombieGameMode>(UGameplayStatics::GetGameMode(this)))
    {
        GameMode->OnDoorSpawn.AddDynamic(this, &AZombieAIController::HandleDoorSpawned);
        GameMode->OnDoorOpen.AddDynamic(this, &AZombieAIController::HandleDoorToggled);
    }

    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AZombieAIController::EnemyDetected);
    }

    InitializeGrandadBlackboardValue();
}

void AZombieAIController::InitializeGrandadBlackboardValue()
{
    UBlackboardComponent *BlackboardComp = GetBlackboardComponent();
    AGrandad *Grandad = AGrandad::GetInstance();

    // Grandad registers himself in his own BeginPlay, which can run after this one,
    // so if he isn't ready yet, try again next frame instead of guessing a delay
    if (!BlackboardComp || !Grandad)
    {
        GetWorldTimerManager().SetTimerForNextTick(this, &AZombieAIController::InitializeGrandadBlackboardValue);
        return;
    }

    BlackboardComp->SetValueAsVector(ZombieKeys::GrandadLocation, Grandad->GetActorLocation());
    Target = Grandad;
}

// Bound to the perception component's OnTargetPerceptionUpdated, so it runs whenever the player enters or leaves sight
void AZombieAIController::EnemyDetected(AActor *Actor, FAIStimulus Stimulus)
{
    ACharacter *Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    UBlackboardComponent *BlackboardComp = GetBlackboardComponent();
    if (!Player || Actor != Player || !BlackboardComp)
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        // Player spotted: chase them
        Target = Player;
        BlackboardComp->ClearValue(ZombieKeys::LastKnownPlayerLocation);
        BlackboardComp->SetValueAsObject(ZombieKeys::PlayerCharacter, Player);
        return;
    }

    // Lost sight of the player: search where they were last seen, then go back to the door or Grandad
    BlackboardComp->ClearValue(ZombieKeys::PlayerCharacter);
    BlackboardComp->SetValueAsVector(ZombieKeys::LastKnownPlayerLocation, Stimulus.StimulusLocation);
    Target = GetDefaultTarget();
}

AActor *AZombieAIController::GetDefaultTarget() const
{
    // A closed front door has to be broken down before the zombies can reach Grandad
    AFrontDoor *FrontDoor = AFrontDoor::GetInstance();
    if (FrontDoor && FrontDoor->bIsSpawned && !FrontDoor->bDoorOpen)
    {
        return FrontDoor;
    }
    return AGrandad::GetInstance();
}

void AZombieAIController::HandleDoorSpawned()
{
    UBlackboardComponent *BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return;
    }

    // Called when a door is bought, and again when zombies destroy it
    AFrontDoor *FrontDoor = AFrontDoor::GetInstance();
    if (FrontDoor && FrontDoor->bIsSpawned)
    {
        BlackboardComp->SetValueAsVector(ZombieKeys::DoorLocation, FrontDoor->GetActorLocation());
    }
    else
    {
        BlackboardComp->ClearValue(ZombieKeys::DoorLocation);
    }
    Target = GetDefaultTarget();
}

void AZombieAIController::HandleDoorToggled()
{
    UBlackboardComponent *BlackboardComp = GetBlackboardComponent();
    AFrontDoor *FrontDoor = AFrontDoor::GetInstance();
    if (!BlackboardComp || !FrontDoor)
    {
        return;
    }

    // An open door lets zombies straight through to Grandad; a closed one has to be broken down first
    if (FrontDoor->bDoorOpen)
    {
        BlackboardComp->ClearValue(ZombieKeys::DoorLocation);
    }
    else
    {
        BlackboardComp->SetValueAsVector(ZombieKeys::DoorLocation, FrontDoor->GetActorLocation());
    }
    Target = GetDefaultTarget();
}
