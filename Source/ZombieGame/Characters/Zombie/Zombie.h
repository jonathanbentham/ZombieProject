#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "ZombieGame/Characters/Zombie/AI/DamageableInterface.h"
#include "NiagaraSystem.h"
#include "Perception/AIPerceptionTypes.h"
#include "Zombie.generated.h"



UCLASS()
class ZOMBIEGAME_API AZombie : public ACharacter, public IDamageableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AZombie();

	// getter and setter for HitCheck
	bool GetZombieHitCheck() const { return ZombieHitCheck; }
	void SetZombieHitCheck(bool NewValue) { ZombieHitCheck = NewValue; }

	// Getter for IsDead
	bool GetIsZombieDead() const { return IsDead; }

	// Getter for IsCrawling
	bool GetIsCrawling() const { return IsCrawling; }

	virtual void HandleDamage(float const DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent *HeadBoxCollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent *RightArmBoxCollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent *TorsoBoxCollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent *RightLegBoxCollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent *LeftLegBoxCollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent *LeftArmBoxCollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FX")
	UAudioComponent *AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	class UNiagaraSystem *ExplodeFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	class UAnimMontage *HitMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsDead; // used in abp and zombieaicontroller and turret, only need a getter

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsCrawling = false; // used in abp and zombieaicontroller, and BTTastk_Attack only need a getter

	UPROPERTY(EditAnywhere, Category = Health)
	float MaxHealth = 100;

	UPROPERTY(EditAnywhere, Category = Health)
	float Health = 100;

	UPROPERTY(EditAnywhere, Category = Health)
	float HeadHealth = 100;

	UPROPERTY(EditAnywhere, Category = Health)
	float TorsoHealth = 100;

	UPROPERTY(EditAnywhere, Category = Health)
	float LeftArmHealth = 50;

	UPROPERTY(EditAnywhere, Category = Health)
	float RightArmHealth = 50;

	UPROPERTY(EditAnywhere, Category = Health)
	float LegHealth = 50;

	UPROPERTY(EditAnywhere, Category = VFX)
	UNiagaraSystem* ZombieImpactVFX;

private:
	// Applies a projectile hit to one body part. Returns true once that part's health has run out.
	bool ApplyBodyPartHit(float Damage, float &BodyPartHealth, int32 Points);

	// Hides a limb's bone, plays the blood effect and removes the limb's hit box
	void RemoveLimb(FName Bone, UPrimitiveComponent *LimbCollision);

	// Every body-part hit box is bound to this one handler
	UFUNCTION()
	void OnBodyPartBeginOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp,
								int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void OnMeshHit(
		UPrimitiveComponent *HitComponent, // The component that was hit
		AActor *OtherActor,				   // The actor that hit the component
		UPrimitiveComponent *OtherComp,	   // The other component involved in the collision
		FVector NormalImpulse,			   // The impulse applied for the hit
		const FHitResult &Hit			   // Information about the hit, including impact point and normal
	);
	// Functions

	bool ZombieHitCheck = false; // also used in ZombieAIController, uses getter and setter

	void Death();

	void DestroyZombieMesh();

	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
};
