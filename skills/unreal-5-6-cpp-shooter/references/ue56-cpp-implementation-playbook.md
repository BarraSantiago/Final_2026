# UE5.6 C++ Shooter Implementation Playbook

## Table of Contents

1. Build and Module Baseline
2. Core Gameplay Architecture
3. Weapon Implementation Patterns
4. Damage, Health, and Death Flow
5. AI for Shooter Encounters
6. HUD and Feedback Integration
7. Networking Checklist
8. Performance and Debugging Checklist

## 1. Build and Module Baseline

- Keep gameplay code in one module until feature boundaries are stable.
- Add modules intentionally in `*.Build.cs` (`AIModule`, `UMG`, `EnhancedInput`, `GameplayTasks`) only when needed.
- Favor fast iteration by reducing full rebuild churn:
- Use forward declarations in headers.
- Include heavy engine headers in `.cpp` files.
- Keep template-heavy code isolated.

## 2. Core Gameplay Architecture

- Keep high-level match rules in `AGameMode`.
- Mirror shared match state in `AGameState`.
- Track per-player health, score, team, and status in `APlayerState`.
- Keep player input and camera intent in `APlayerController`.
- Keep movement and combat embodiment in `ACharacter`.
- Extract weapon handling, inventory, and damage-response behaviors into components when classes grow too large.

### Recommended Event Flow

1. Receive input in `APlayerController` or bound input component.
2. Route intent to `ACharacter` or weapon component.
3. Execute authoritative fire logic.
4. Apply damage through a consistent damage API.
5. Broadcast hit and death events for UI, audio, and FX.

## 3. Weapon Implementation Patterns

### Choose Firing Model

- Choose hitscan when immediate response and precision are required.
- Choose projectile when travel time, dodge windows, or arc behavior is desired.
- Mix both when weapon roles differ (rifle hitscan, launcher projectile).

### Hitscan Baseline

- Perform line trace from camera or muzzle depending on aim model.
- Validate collision channel and ignored actors.
- Compute final hit and call damage once per confirmed trace.
- Send cosmetic effects separately from authoritative damage logic.

### Projectile Baseline

- Spawn projectile on authoritative context.
- Configure movement component for velocity, gravity scale, and lifespan.
- Handle collision on projectile actor.
- Apply radial or point damage in one predictable path.

### Reload and Ammo State

- Keep ammo state in weapon or inventory component.
- Block fire during reload with explicit state checks.
- Use deterministic reload duration (timer-driven) before introducing animation-dependent windows.

## 4. Damage, Health, and Death Flow

- Use one primary health owner (`ACharacter` or `UHealthComponent`).
- Clamp health to valid ranges.
- Trigger death once; guard against duplicate death execution.
- Separate gameplay death from presentation:
- Gameplay: disable input, stop firing, set death state.
- Presentation: play montage, ragdoll, sound, camera effects.

### Practical Sequence

1. Receive damage event.
2. Validate alive state.
3. Subtract health and clamp.
4. If health reaches zero, execute death flow once.
5. Notify killer/victim systems for score and UI.

## 5. AI for Shooter Encounters

- Use behavior trees or state trees for readable state transitions.
- Define enemy archetypes by role:
- Pressure: pushes player, short-range aggression.
- Anchor: holds key space, suppresses movement lanes.
- Flanker: punishes static player behavior.
- Keep perception setup explicit (sight ranges, lose-sight time, peripheral angle).
- Drive fire cadence from tunable data rather than hardcoded constants.

## 6. HUD and Feedback Integration

- Bind HUD to replicated or event-driven gameplay data.
- Prioritize readability: health, ammo, hit confirmation, damage direction.
- Avoid polling every frame when events can update UI.
- Keep reticle behavior consistent with firing model.

### Minimum HUD for Playable Shooter Slice

- Health value and critical threshold indicator.
- Current ammo and reserve ammo.
- Hit confirmation cue.
- Death and respawn status text or overlay.

## 7. Networking Checklist

- Decide authority model before writing combat code.
- Route client input to server-authoritative combat entry points.
- Replicate only required state (health, ammo, reload state, match data).
- Keep cosmetic effects predictable:
- Trigger on replicated state change or multicast events.
- Avoid duplicate FX on owning client.
- Validate anti-cheat-sensitive actions server-side (fire rate, ammo consumption, hit validation assumptions).

### Multiplayer Shooter Guardrails

- Avoid client-authoritative damage.
- Avoid trusting client ammo counts.
- Avoid running heavy traces on every simulated proxy tick.

## 8. Performance and Debugging Checklist

- Profile hotspots before rewriting architecture.
- Reduce Tick usage; prefer timers and event-driven updates.
- Batch expensive debug drawing and disable in shipping paths.
- Add structured logs for combat events during debugging windows.
- Keep one test map dedicated to combat regression checks.

### Regression Smoke Pass

1. Spawn and movement remain responsive.
2. Weapon fire rate matches design intent.
3. Reload lockout and ammo transitions are consistent.
4. Damage, death, and respawn states do not desync.
5. HUD values match authoritative state.
