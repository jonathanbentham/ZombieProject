#pragma once

#include "CoreMinimal.h"
#include "ZombieGameProjectile.h"
#include "ZombieGame/Widgets/BuyableItemInterface.h"
#include "ARProjectile.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIEGAME_API AARProjectile : public AZombieGameProjectile, public IBuyableItemInterface
{
	GENERATED_BODY()
public:
	AARProjectile();

	virtual void HandleBuyItem() override;


};
