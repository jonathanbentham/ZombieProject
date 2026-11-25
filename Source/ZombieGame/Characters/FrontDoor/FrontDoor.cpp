
#include "ZombieGame/Characters/FrontDoor/FrontDoor.h"
#include "ZombieGame/Characters/PlayerCharacter/ZombieGameCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UMG/Public/Components/TextBlock.h"
#include "ZombieGame/GameMode/ZombieGameMode.h"


// Initialize the Singleton instance - this is done outside of any function
AFrontDoor* AFrontDoor::FrontDoorInstance = nullptr;

// Sets default values
AFrontDoor::AFrontDoor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	RootComponent = _RootComponent;

	// Create the box collision and attach it to the root
	BoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollisionComponent"));
	BoxCollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	BoxCollisionComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door Mesh"));
	DoorMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFrontDoor::BeginPlay()
{
	Super::BeginPlay();

	BoxCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AFrontDoor::OnItemBeginOverlap);
	BoxCollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AFrontDoor::OnItemEndOverlap);
}

void AFrontDoor::OnItemBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp,
                                    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Log, TEXT("OVERLAPPING FRONT DOOR"));
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	AZombieGameCharacter* Character = Cast<AZombieGameCharacter>(PlayerCharacter);
	if (OtherActor && OtherActor->IsA<AZombieGameCharacter>())
	{
		if (Character->MainWidgetInstance)
		{
			FString Text = FString::Printf(TEXT("Press 'E' To Open/Close"));
			Character->MainWidgetInstance->EquipItemText->SetText(FText::FromString(Text));
			Character->MainWidgetInstance->EquipItemText->SetVisibility(ESlateVisibility::Visible);

			Character->OverlappingFrontDoor = Cast<AFrontDoor>(OverlappedComponent->GetOwner());

			// UE_LOG(LogTemp, Warning, TEXT("Overlapped Component: %s"), *OverlappedComponent->GetOwner()->GetName());
		}
	}
}

void AFrontDoor::OnItemEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	AZombieGameCharacter* Character = Cast<AZombieGameCharacter>(PlayerCharacter);
	if (Character->MainWidgetInstance)
	{
		Character->MainWidgetInstance->EquipItemText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AFrontDoor::UseFrontDoor()
{
	// Swing the door 90 degrees open or closed
	FRotator NewRotation = DoorMesh->GetRelativeRotation();
	NewRotation.Yaw += bDoorOpen ? -90.0f : 90.0f;
	DoorMesh->SetRelativeRotation(NewRotation);
	bDoorOpen = !bDoorOpen;

	// Tell the zombies after the door has changed, so they read its new state
	if (AZombieGameMode* GameMode = Cast<AZombieGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->OnDoorOpen.Broadcast();
	}
}

void AFrontDoor::HandleDamage(float const DamageAmount, struct FDamageEvent const& DamageEvent,
                              class AController* EventInstigator, AActor* DamageCauser)
{
	// Apply the damage first, then check, so the hit that breaks the door destroys it
	const float DamageTaken = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health -= FMath::Min(Health, FMath::RoundToInt(DamageTaken));
	if (Health > 0)
	{
		return;
	}

	bIsSpawned = false;
	Destroy();
	if (AZombieGameMode* GameMode = Cast<AZombieGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->OnDoorSpawn.Broadcast();
	}
}

void AFrontDoor::HandleBuyItem()
{
	const AZombieGameMode* GameMode = Cast<AZombieGameMode>(UGameplayStatics::GetGameMode(this));

	SetInstance(Cast<AFrontDoor>(GetWorld()->SpawnActor(FrontDoor, &DoorSpawnLocation, &DoorSpawnRotation)));
	GetInstance()->bIsSpawned = true;
	GameMode->OnDoorSpawn.Broadcast();
}

AFrontDoor* AFrontDoor::GetInstance()
{
	return FrontDoorInstance;
}


void AFrontDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// The instance pointer is static, so clear it when this door goes away (destroyed, or the play session ends)
	if (FrontDoorInstance == this)
	{
		FrontDoorInstance = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void AFrontDoor::SetInstance(AFrontDoor* NewInstance)
{
	FrontDoorInstance = NewInstance;
}
