# Shooter Game Design Reference

## Table of Contents

1. Core Shooter Pillars
2. Camera and Movement Model Choices
3. Weapon Taxonomy and Roles
4. Encounter and Map Design Principles
5. TTK, DPS, and Balance Heuristics
6. AI and Enemy Role Design
7. Mode and Objective Design
8. Progression and Retention Loops
9. Playtest Method for Shooter Tuning

## 1. Core Shooter Pillars

- Define a clear combat fantasy (tactical, arcade, hero, survival, extraction).
- Keep player readability high: threat sources, damage feedback, and objective state must be obvious.
- Reward skill expression through positioning, aim discipline, timing, and resource decisions.
- Preserve response clarity: player action must create immediate, understandable feedback.

## 2. Camera and Movement Model Choices

- Choose FPS when precision aim and gunfeel are central.
- Choose TPS when spatial awareness, character readability, and traversal are central.
- Align movement speed with weapon precision:
- Faster movement supports close combat and chaos.
- Slower movement supports lane control and tactical decisions.
- Decide early whether airborne combat is strong, neutral, or weak.

## 3. Weapon Taxonomy and Roles

- Build weapon identity around role, not raw stats.
- Use a compact role set for early prototypes:
- Primary rifle: reliable mid-range pressure.
- Shotgun: high burst at short range.
- Precision weapon: high reward for accurate shots.
- Area weapon: space denial and movement forcing.
- Sidearm: low-risk fallback.

### Distinguish Weapons Through Levers

- Time-to-kill profile (burst vs sustained).
- Effective range and spread behavior.
- Magazine size and reload risk.
- Recoil and handling complexity.
- Utility effects (stagger, splash, slow, mark).

## 4. Encounter and Map Design Principles

- Design combat spaces with readable lanes, cover rhythm, and flank routes.
- Avoid single dominant sightlines that invalidate movement diversity.
- Use verticality intentionally:
- Advantage: information and angle control.
- Counter: exposure and predictable retreat paths.
- Place resources (ammo, health, power weapons) to create rotation decisions.

### Arena Heuristic

- Ensure every high-value zone has at least two approach routes.
- Ensure each route has a tradeoff (speed, safety, exposure, resource access).

## 5. TTK, DPS, and Balance Heuristics

- Tune using a target TTK band per mode and skill tier.
- Keep early prototype ranges broad; narrow them after playtests.

### Useful Calculations

- `shots_to_kill = ceil(target_health / damage_per_shot)`
- `ttk_seconds = (shots_to_kill - 1) / shots_per_second`
- `sustained_dps = damage_per_shot * shots_per_second`

### Practical Tuning Rules

- Reduce damage spikes before reducing average damage if fights feel unfair.
- Adjust reload downtime before adjusting damage if pacing feels flat.
- Adjust spread and recoil before damage when precision difficulty is the issue.
- Avoid simultaneous multi-variable changes during one test pass.

## 6. AI and Enemy Role Design

- Build enemy squads with complementary roles instead of identical units.
- Use role-based pressure:
- Chaser: collapses player space.
- Suppressor: punishes peeking.
- Flanker: punishes tunnel vision.
- Tank: creates temporary objective around positioning.
- Keep telegraphs readable for high-threat attacks.

## 7. Mode and Objective Design

- Match objective type to combat pacing goals.
- Use clear objective state transitions and comeback opportunities.

### Common Shooter Mode Patterns

- Elimination / Team Deathmatch: emphasize dueling and macro positioning.
- Control / Capture: emphasize timing and area denial.
- Payload / Escort: emphasize forward momentum and staging fights.
- Extraction / Survival: emphasize risk, loot value, and escape windows.

## 8. Progression and Retention Loops

- Separate power progression from competitive fairness where possible.
- Use unlock systems to expand options rather than grant unavoidable advantage.
- Reward mastery milestones:
- Accuracy improvement.
- Objective contribution.
- Tactical diversity.
- Provide short-loop rewards (match-end) and medium-loop goals (weekly progression).

## 9. Playtest Method for Shooter Tuning

### Session Setup

- Test one hypothesis per session.
- Lock all unrelated variables.
- Capture player skill context for each result.

### Observations to Track

- Average engagement duration.
- First-hit to kill consistency.
- Weapon pick rates and abandonment rates.
- Death causes (positioning, aim loss, unavoidable burst, objective overexposure).
- Player-reported fairness vs frustration moments.

### Post-Test Adjustment Loop

1. Classify issue as readability, balance, pacing, or content problem.
2. Apply one targeted adjustment.
3. Re-run the same scenario.
4. Compare against previous values before broadening changes.
