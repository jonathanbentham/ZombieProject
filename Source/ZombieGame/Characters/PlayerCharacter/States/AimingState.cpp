
#include "AimingState.h"

#include "IdleState.h"
#include "ReloadingState.h"
#include "Components/Image.h"

void UAimingState::EnterState(AZombieGameCharacter* Character)
{
	// Can enter this state as not reloading
	UE_LOG(LogTemp, Display, TEXT("Aiming Now!!!"));

	Character->MainWidgetInstance->Crosshair->SetVisibility(ESlateVisibility::Hidden);
	if (Character->GetHasMaxSpeed())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	}
	else
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = 200.f;
	}
}

void UAimingState::TryEnterState(AZombieGameCharacter* Character)
{
	if (Character->CurrentStateInstance->IsA<UReloadingState>()) return;
	Character->CurrentStateInstance = Character->GetState<UAimingState>();
	EnterState(Character);
}

void UAimingState::TryExitState(AZombieGameCharacter* Character)
{
	if (!(Character->CurrentStateInstance->IsA<UAimingState>())) return;
	Character->CurrentStateInstance = Character->GetState<UIdleState>();
	Character->CurrentStateInstance->EnterState(Character);
}



