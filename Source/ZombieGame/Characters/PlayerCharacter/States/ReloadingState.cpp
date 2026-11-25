
#include "ReloadingState.h"
#include "AimingState.h"
#include "IdleFireState.h"
#include "IdleState.h"
#include "Components/Button.h"
#include "ZombieGame/Characters/Grandad/Grandad.h"

namespace
{
	// Name of the shop item that sells ammo for a weapon type
	FString GetAmmoShopItemName(const EWeaponType WeaponType)
	{
		switch (WeaponType)
		{
		case EWeaponType::E_Pistol:
			return TEXT("Pistol Ammo");
		case EWeaponType::E_AssaultRifle:
			return TEXT("Assault Rifle Ammo");
		case EWeaponType::E_Shotgun:
			return TEXT("Shotgun Ammo");
		default:
			return FString();
		}
	}
}

void UReloadingState::EnterState(AZombieGameCharacter* Character)
{
	const ABaseWeapon* Weapon = Character->Weapons[Character->CurrentWeaponIndex];
	Character->GunMesh->PlayAnimation(Weapon->WeaponReloadMontage, false);

	// Refill just before the reload animation ends, so the ammo count updates as the mag goes in
	const float ReloadDelay = Weapon->WeaponReloadMontage->GetPlayLength() - 0.2f;
	FTimerHandle ReloadTimerHandle;
	const FTimerDelegate ReloadDelegate = FTimerDelegate::CreateUObject(this, &UReloadingState::RefillAmmo, Character);
	Character->GetWorldTimerManager().SetTimer(ReloadTimerHandle, ReloadDelegate, ReloadDelay, false);
}

void UReloadingState::TryEnterState(AZombieGameCharacter* Character)
{
	const ABaseWeapon* Weapon = Character->Weapons[Character->CurrentWeaponIndex];
	const bool bHasReserveAmmo = Weapon->TotalWeaponAmmo != 0;
	const bool bClipIsFull = Weapon->CurrentWeaponAmmo == Weapon->MaxWeaponClipSize;
	if (!bHasReserveAmmo || bClipIsFull)
	{
		return;
	}

	// Can't start a reload while already reloading, aiming or firing
	const UCharacterStates* CurrentState = Character->CurrentStateInstance;
	if (CurrentState->IsA<UReloadingState>() || CurrentState->IsA<UAimingState>() || CurrentState->IsA<UIdleFireState>())
	{
		return;
	}

	Character->CurrentStateInstance = Character->GetState<UReloadingState>();
	Character->CurrentStateInstance->EnterState(Character);
}

void UReloadingState::TryExitState(AZombieGameCharacter* Character)
{
	Character->CurrentStateInstance = Character->GetState<UIdleState>();
	Character->CurrentStateInstance->EnterState(Character);
}

void UReloadingState::RefillAmmo(AZombieGameCharacter* Character)
{
	ABaseWeapon* Weapon = Character->Weapons[Character->CurrentWeaponIndex];

	// Move as much reserve ammo into the clip as it can hold
	const int32 NeededAmmo = Weapon->MaxWeaponClipSize - Weapon->CurrentWeaponAmmo;
	const int32 AmmoToLoad = FMath::Min(NeededAmmo, Weapon->TotalWeaponAmmo);
	if (AmmoToLoad > 0)
	{
		Weapon->CurrentWeaponAmmo += AmmoToLoad;
		Weapon->TotalWeaponAmmo -= AmmoToLoad;
	}

	// Ammo for this weapon can now be bought again, so re-enable its shop button
	const AGrandad* Grandad = AGrandad::GetInstance();
	const FString AmmoItemName = GetAmmoShopItemName(Weapon->WeaponType);
	if (Grandad && Grandad->ShopWidgetInstance && !AmmoItemName.IsEmpty())
	{
		for (UShopItem* ShopItem : Grandad->ShopWidgetInstance->ShopItems)
		{
			if (ShopItem && ShopItem->Item && ShopItem->ItemButton && ShopItem->Item->Name.ToString() == AmmoItemName)
			{
				ShopItem->bIsOwned = false;
				ShopItem->ItemButton->SetBackgroundColor(FLinearColor::Gray);
			}
		}
	}

	Character->AmmoArray[Character->CurrentWeaponIndex] = Weapon->TotalWeaponAmmo;
	Character->CurrentStateInstance->TryExitState(Character);
}
