#pragma once

#include "CoreMinimal.h"
#include "ZombieGameProjectile.h"
#include "ZombieGame/Widgets/BuyableItemInterface.h"
#include "PistolProjectile.generated.h"

/**
 *
 */
UCLASS()
class ZOMBIEGAME_API APistolProjectile : public AZombieGameProjectile, public IBuyableItemInterface
{
	GENERATED_BODY()

public:
	APistolProjectile();

	virtual void HandleBuyItem() override;

};
