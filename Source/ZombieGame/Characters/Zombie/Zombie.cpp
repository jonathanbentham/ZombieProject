#include "Zombie.h"
#include "Kismet/GameplayStatics.h"
#include "ZombieGame/GameMode/ZombieGameMode.h"
#include "ZombieGame/Characters/PlayerCharacter/ZombieGameCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/DamageEvents.h"
#include "AIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"


// Sets default values
AZombie::AZombie()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Create and attach the audio component
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);

	HeadBoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadBoxCollisionComponent"));
	HeadBoxCollisionComponent->SetupAttachment(GetMesh(), TEXT("head"));

	TorsoBoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("TorsoBoxCollisionComponent"));
	TorsoBoxCollisionComponent->SetupAttachment(GetMesh(), TEXT("spine_01"));

	RightLegBoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("RightLegBoxCollisionComponent"));
	RightLegBoxCollisionComponent->SetupAttachment(GetMesh(), TEXT("calf_r"));

	RightArmBoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("RightArmBoxCollisionComponent"));
	RightArmBoxCollisionComponent->SetupAttachment(GetMesh(), TEXT("lowerarm_r"));

	LeftLegBoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftLegBoxCollisionComponent"));
	LeftLegBoxCollisionComponent->SetupAttachment(GetMesh(), TEXT("calf_l"));

	LeftArmBoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftArmBoxCollisionComponent"));
	LeftArmBoxCollisionComponent->SetupAttachment(GetMesh(), TEXT("lowerarm_l"));

	
	UCharacterMovementComponent *ZombieMovement = Cast<UCharacterMovementComponent>(GetMovementComponent());
	ZombieMovement->MaxWalkSpeed = 300.f;

	Health = MaxHealth;
}

// Called when the game starts or when spawned
void AZombie::BeginPlay()
{
	Super::BeginPlay();

	// All body-part hit boxes share one overlap handler, which works out which part was hit
	for (UBoxComponent *BodyPart : {HeadBoxCollisionComponent, TorsoBoxCollisionComponent, LeftArmBoxCollisionComponent,
									 RightArmBoxCollisionComponent, LeftLegBoxCollisionComponent, RightLegBoxCollisionComponent})
	{
		BodyPart->OnComponentBeginOverlap.AddDynamic(this, &AZombie::OnBodyPartBeginOverlap);
	}
	GetMesh()->OnComponentHit.AddDynamic(this, &AZombie::OnMeshHit);

}

void AZombie::HandleDamage(float const DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser)
{
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (IsDead == false)
	{
		DamageToApply = FMath::Min(Health, DamageToApply);
		Health -= DamageToApply;
		if (Health > 0)
		{
			PlayAnimMontage(HitMontage);
			ZombieHitCheck = true;
			UE_LOG(LogTemp, Warning, TEXT("Health left %f"), Health);
		}
		else
		{
			Death();
		}
	}
}



namespace
{
	constexpr int32 HeadshotPoints = 100;
	constexpr int32 BodyShotPoints = 50;

	const FName HeadBone(TEXT("head"));
	const FName LeftArmBone(TEXT("lowerarm_l"));
	const FName RightArmBone(TEXT("lowerarm_r"));
	const FName LeftLegBone(TEXT("calf_l"));
	const FName RightLegBone(TEXT("calf_r"));
}

void AZombie::OnBodyPartBeginOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp,
									 int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	const AZombieGameProjectile *Projectile = Cast<AZombieGameProjectile>(OtherActor);
	if (!Projectile)
	{
		return;
	}

	if (OverlappedComponent == HeadBoxCollisionComponent)
	{
		if (ApplyBodyPartHit(Projectile->HeadDamage, HeadHealth, HeadshotPoints))
		{
			RemoveLimb(HeadBone, HeadBoxCollisionComponent);
		}
	}
	else if (OverlappedComponent == LeftArmBoxCollisionComponent || OverlappedComponent == RightArmBoxCollisionComponent)
	{
		const bool bIsLeftArm = OverlappedComponent == LeftArmBoxCollisionComponent;
		float &ArmHealth = bIsLeftArm ? LeftArmHealth : RightArmHealth;
		const float OtherArmHealth = bIsLeftArm ? RightArmHealth : LeftArmHealth;

		// A zombie can only lose one arm, so it can always still attack
		if (ApplyBodyPartHit(Projectile->ArmDamage, ArmHealth, BodyShotPoints) && OtherArmHealth > 0.f)
		{
			RemoveLimb(bIsLeftArm ? LeftArmBone : RightArmBone, OverlappedComponent);
		}
	}
	else if (OverlappedComponent == LeftLegBoxCollisionComponent || OverlappedComponent == RightLegBoxCollisionComponent)
	{
		// Both legs share one health pool. Losing it removes both legs and the zombie starts crawling.
		if (ApplyBodyPartHit(Projectile->LegDamage, LegHealth, BodyShotPoints) && !IsCrawling)
		{
			RemoveLimb(LeftLegBone, LeftLegBoxCollisionComponent);
			RemoveLimb(RightLegBone, RightLegBoxCollisionComponent);
			IsCrawling = true;
		}
	}
	else if (OverlappedComponent == TorsoBoxCollisionComponent)
	{
		// The torso takes damage but is never removed
		ApplyBodyPartHit(Projectile->TorsoDamage, TorsoHealth, BodyShotPoints);
	}
}

bool AZombie::ApplyBodyPartHit(const float Damage, float &BodyPartHealth, const int32 Points)
{
	// Every hit also damages the zombie as a whole
	const FPointDamageEvent DamageEvent(Damage, FHitResult(), FVector::ZeroVector, nullptr);
	HandleDamage(Damage, DamageEvent, GetInstigatorController(), this);

	if (AZombieGameCharacter *Player = Cast<AZombieGameCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		Player->SetPoints(Player->GetPoints() + Points);
	}

	BodyPartHealth -= Damage;
	return BodyPartHealth <= 0.f;
}

void AZombie::RemoveLimb(const FName Bone, UPrimitiveComponent *LimbCollision)
{
	// Hiding the bone and spawning a blood effect is much cheaper than slicing the mesh
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplodeFX, GetMesh()->GetSocketLocation(Bone), FRotator::ZeroRotator);
	GetMesh()->HideBoneByName(Bone, PBO_None);

	if (LimbCollision)
	{
		LimbCollision->DestroyComponent();
	}
}

void AZombie::OnMeshHit(UPrimitiveComponent *HitComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse, const FHitResult &Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Zombie mesh hit"));
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ZombieImpactVFX, Hit.Location);
}



// Called to bind functionality to input
void AZombie::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AZombie::Death()
{
	UE_LOG(LogTemp, Warning, TEXT("ZOMBIE DEAD "));

	// AI can no longer detect the zombie upon death
	if (UAIPerceptionStimuliSourceComponent* StimuliSourceComponent = FindComponentByClass<UAIPerceptionStimuliSourceComponent>())
	{
		// Unregister from the Sight sense
		StimuliSourceComponent->UnregisterFromSense(UAISense_Sight::StaticClass());
	}
	
	IsDead = true;

	GetCharacterMovement()->StopMovementImmediately();

	AudioComponent->SetComponentTickEnabled(false);
	AudioComponent->Stop();
	SetActorEnableCollision(false);

	AZombieGameMode *MyMode = Cast<AZombieGameMode>(UGameplayStatics::GetGameMode(GetWorld())); // gets the game mode

	MyMode->HandleZombieCountAndRound();

	GetMesh()->GetAnimInstance()->Montage_Stop(0.2f, HitMontage);

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AZombie::DestroyZombieMesh, 2.0f, false);
}

void AZombie::DestroyZombieMesh()
{
	Destroy();
}
