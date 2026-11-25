#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZombieGame/Characters/PlayerCharacter/ZombieGameCharacter.h"
#include "CharacterStates.generated.h"

/**
 * Base class for the player character's states (Idle, Aiming, Fire, Reloading, Swapping).
 * Each state overrides these; the base versions do nothing.
 */
UCLASS(BlueprintType)
class ZOMBIEGAME_API UCharacterStates : public UObject
{
	GENERATED_BODY()

public:
	virtual void EnterState(AZombieGameCharacter* Character);

	virtual void TryEnterState(AZombieGameCharacter* Character);

	virtual void TryExitState(AZombieGameCharacter* Character);

};
