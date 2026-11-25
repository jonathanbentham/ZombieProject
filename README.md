# Defend Grandad

A round-based zombie FPS made in **Unreal Engine 5** and **C++**. Grandad can't move, so you have to protect him from waves of the undead while you explore the map for weapons and perks, and spend points in his shop on ammo, a front door, a turret or a dog.

This is a solo project. Apart from the 3D models, animations and sound effects, I built everything.

**Case study, clips and devlogs:** https://jonathanbentham.github.io/zombie-game

## Code highlights

If you only have a few minutes, these are the parts worth reading:

| System | Where | What it shows |
|---|---|---|
| **Dismemberment** | [`Zombie.cpp` – `OnBodyPartBeginOverlap`](Source/ZombieGame/Characters/Zombie/Zombie.cpp#L96) | One overlap handler for every hit box, with per-limb health. Limbs are removed by hiding bones and spawning Niagara effects instead of expensive mesh slicing. A zombie can only lose one arm, and losing its legs makes it crawl. |
| **Zombie AI** | [`ZombieAIController.cpp` – `EnemyDetected`](Source/ZombieGame/Characters/Zombie/AI/ZombieAIController.cpp#L67) | Event-driven targeting through AI Perception and the blackboard. Zombies chase the player while they can see them, check the last known location, then go back to the front door or Grandad. |
| **Async EQS task** | [`BTTask_FindClosestZombie.cpp`](Source/ZombieGame/Characters/BTTask_FindClosestZombie.cpp#L16) | A latent behaviour tree task, shared by the dog and turret, that runs an EQS query and finishes when the result comes back. It is instanced per AI and always finishes, even when the query fails. |
| **Player state machine** | [`PlayerCharacter/States/`](Source/ZombieGame/Characters/PlayerCharacter/States) | The state pattern for Idle, Aiming, Fire, Reloading and Swapping, keeping each state's rules out of the character class. The character keeps one instance of each state and reuses it. |
| **Wave spawning** | [`ZombieGameMode.cpp` – `SpawnZombies`](Source/ZombieGame/GameMode/ZombieGameMode.cpp#L19) | Wave scaling with boss waves every fifth round, and sphere-swept ground placement with a bounded number of attempts, configurable from the editor. |

## Systems

- **Game mode:** round-based waves, boss rounds and win and lose conditions
- **Weapons:** pistol, shotgun and assault rifle, each with its own projectile, damage and bullet drop
- **AI:** behaviour trees, AI Perception and EQS for zombies, dogs and turrets
- **Dismemberment:** shoot off heads, arms and legs
- **Shop:** items driven by a data table and added to the UI at runtime
- **Perks:** Extended Mag, Fully Auto, Health Juice and Max Speed perk machines
- **UI:** HUD, wave announcements, shop, and win and death screens

## Project

- Unreal Engine 5.2, C++ and Blueprint
- Source code is in [`Source/ZombieGame`](Source/ZombieGame)
- Large assets are stored with Git LFS
