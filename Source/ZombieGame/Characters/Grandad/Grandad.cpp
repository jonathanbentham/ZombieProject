#include "ZombieGame/Characters/Grandad/Grandad.h"
#include "ZombieGame/GameMode/ZombieGameMode.h"
#include "ZombieGame/Characters/PlayerCharacter/ZombieGameCharacter.h"
#include "Kismet/GameplayStatics.h"

// Initialize the Singleton instance - this is done outside of any function
AGrandad *AGrandad::GrandadInstance = nullptr;

// Sets default values
AGrandad::AGrandad()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the box collision and attach it to the root
	BoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollisionComponent"));
	BoxCollisionComponent->InitBoxExtent(FVector(100.f, 100.f, 100.f));
	BoxCollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	BoxCollisionComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void AGrandad::BeginPlay()
{
	Super::BeginPlay();

	BoxCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AGrandad::OnItemBeginOverlap);

	// The only way I've found to get an exact instance of an actor spawned in the world
	TArray<AActor *> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(TEXT("GrandadTag")), FoundActors);
	if (FoundActors.Num() > 0)
	{
		GrandadInstance = Cast<AGrandad>(FoundActors[0]);
	}

	if (!ShopWidgetInstance)
	{
		ShopWidgetInstance = CreateWidget<UShopWidget>(GetWorld(), ShopWidgetClass);
		ShopWidgetInstance->AddToViewport();
		ShopWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AGrandad::OnItemBeginOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp,
								  int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	
	if (OtherActor->IsA<AZombieGameCharacter>())
	{
		UE_LOG(LogTemp, Log, TEXT("OVERLAPPING WITH GRANDAD"));
		ACharacter *PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		AZombieGameCharacter *Character = Cast<AZombieGameCharacter>(PlayerCharacter);
		APlayerController *PlayerController = Cast<APlayerController>(Character->GetController());

		if (MainWidgetInstance)
		{
			MainWidgetInstance->RemoveFromParent();
		}

		if (!ShopWidgetInstance)
		{
			ShopWidgetInstance = CreateWidget<UShopWidget>(GetWorld(), ShopWidgetClass);
			ShopWidgetInstance->AddToViewport();
		}
		ShopWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		
		PlayerController->SetInputMode(FInputModeUIOnly());
		PlayerController->bShowMouseCursor = true;
		Character->GetCharacterMovement()->MaxWalkSpeed = 0;
		
	}
}

void AGrandad::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// The instance pointer is static, so clear it or the next play session would see a destroyed Grandad
	if (GrandadInstance == this)
	{
		GrandadInstance = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

// Static method to get the Singleton instance
AGrandad *AGrandad::GetInstance()
{
	return GrandadInstance;
}

// Called every frame
void AGrandad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AGrandad::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGrandad::HandleDamage(float const DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser)
{
	// Run the base TakeDamage so damage events still fire, then apply the result to Grandad's health
	const float DamageTaken = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health -= FMath::Min(Health, FMath::RoundToInt(DamageTaken));
	UE_LOG(LogTemp, Log, TEXT("Grandad health left: %d"), Health);

	// Check after applying damage, so the killing hit ends the game
	if (Health <= 0)
	{
		if (AZombieGameCharacter *Character = Cast<AZombieGameCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
		{
			Character->HandleCharacterDeath();
		}
	}
}


void AGrandad::HandleDeath()
{
	IsDead = true;
	AZombieGameMode *MyMode = Cast<AZombieGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	MainWidgetInstance->ShowDeathWindow();
}
