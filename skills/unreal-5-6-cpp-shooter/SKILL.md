---
name: unreal-5-6-cpp-shooter
description: Unreal Engine 5.6 C++ shooter-game development workflows for gameplay systems, weapons, projectiles, AI, HUD/UI, input, and balance tuning. Use when Codex needs to design, implement, debug, refactor, or document shooter mechanics in UE5.6 projects, especially first-person or third-person combat loops and iteration-ready shooter prototypes.
---

# Unreal Engine 5.6 C++ Shooter

## Overview

Use this skill to implement and iterate shooter systems in UE5.6 C++ with engine-native patterns and clear playtest loops.

## Workflow Decision Tree

- If asked to write or debug shooter gameplay code, follow `Implementation Workflow` and load `references/ue56-cpp-implementation-playbook.md`.
- If asked for shooter ideas, balancing, modes, or progression guidance, load `references/shooter-game-design.md`.
- If asked for both gameplay code and design tuning, load both references, decide design constraints first, then implement.

## Implementation Workflow

1. Define constraints.
- Confirm first-person or third-person camera, single-player or multiplayer, and keyboard/mouse or gamepad input.
- Confirm target mechanic set: weapon firing, damage, reloading, pickups, AI, and HUD.
- Define the smallest playable loop: move, aim, shoot, hit confirmation, survive or die, restart or progress.

2. Map systems to Unreal classes.
- Place match rules in `AGameMode` and replicated match data in `AGameState`.
- Place per-player replicated data in `APlayerState`.
- Keep input orchestration in `APlayerController`; keep embodiment in `ACharacter` or `APawn`.
- Put reusable combat logic into components to reduce monolithic character classes.

3. Build one vertical slice.
- Implement one weapon (hitscan or projectile), one damage path, and one enemy behavior.
- Implement one pickup and one HUD feedback widget (ammo or health).
- Wire all of it through a single test map before extending features.

4. Validate behavior.
- Compile and resolve warnings before tuning balance.
- Run smoke checks: spawn, movement, fire, reload, hit, kill/death, respawn, pickup, HUD refresh.
- Separate functional bugs from tuning issues; fix functional bugs first.

5. Iterate safely.
- Tune one balance dimension at a time: damage, rate of fire, spread, movement speed, health pool.
- Record changed values and observed gameplay effect.
- Move stable tunables into data-driven assets once the loop feels correct.

## Output Requirements

- Provide compilable C++ snippets with required include hints and class context.
- State assumptions explicitly when project details are missing.
- Mention replication consequences when changing combat logic.
- Include a short verification checklist for every non-trivial gameplay change.

## References

- `references/ue56-cpp-implementation-playbook.md`: Use for UE5.6 C++ architecture, common shooter implementation patterns, replication checklist, and debugging flow.
- `references/shooter-game-design.md`: Use for shooter game fundamentals, weapon and encounter design, pacing, TTK guidance, and progression patterns.
