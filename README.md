# SexLab Defeat NG (SKSE)

Native SKSE plugin that backs the **Andb SexLab Defeat script patch**. It moves hit/knockdown processing out of Papyrus `OnHit` into C++, keeps actor defeat state in sync with scripts, drives the Dynamic Defeat widget, and exposes natives used by `defeat_skse_api`.

Companion Papyrus sources: [SLDefeat (Andb patch)](https://github.com/AndbGame/SLDefeat) — install **both** the DLL and the patched scripts. Neither piece works alone as a full Defeat replacement.

| | |
| --- | --- |
| Plugin DLL | `SexLabDefeat.dll` |
| Project / Papyrus class | `SexLabDefeat` / `defeat_skse_api` |
| Runtime target | Skyrim SE / AE / VR (CommonLibSSE-NG) |
| Version (CMake) | `0.2.0` |
| Repo | https://github.com/AndbGame/SLDefeatNG |

## Why this exists

Stock SexLab Defeat evaluates knockdown heavily in Papyrus. Under load that path lags, drops hits, and fights other mods. This plugin:

1. Listens to native combat / hit / bleedout events
2. Applies Defeat / LRG rules (thresholds, Dynamic Defeat, vulnerability, sexuality filters, spam guards)
3. Calls back into Papyrus only when a knockdown / interrupt / extra-data fetch is needed

Development status is tracked in `Changelog.txt` (Experimental 1–7 and later fixes). Treat builds as experimental until you have tested them with your load order.

## Features

### Native hit pipeline

- `TESHitEvent`, combat-state, and bleedout sinks feed a `DefeatCombatManager`
- Separate handling for player victim, follower, and NPC-vs-NPC (NvN)
- Hit / projectile spam guards (`CombatManager.HitSpamGuardMs` in the INI)
- Actor processing states: hits only while `ACTIVE`; after knockdown the actor is set `DISACTIVE` until Papyrus restores `ACTIVE`

### Actor state API

Papyrus natives on `defeat_skse_api`:

| Native | Role |
| --- | --- |
| `setActorState` / `getActorState` | Sync Defeat states (`Knockdown`, `Trauma`, `Surrender`, …) |
| `tryExchangeActorState` | Atomic state swap (avoids double-claim races in NvN) |
| `isActorTied` | Tied-state check |
| `DrawWeapon` / `SheatheWeapon` | Weapon draw helpers |
| `responseActorExtraData` | Cache SexLab gender/sexuality/race key, ignore-on-hit, DFW vulnerability |
| `queryNvNScene` | Pick an aggressor for a downed NPC |
| `getNearestActorsForGangBang` | Extra aggressors for NvN gangbang |
| `getLastHitAggressor` | Last recorded hit aggressor |

Plugin → script callbacks include `playerKnockDownEvent`, `npcKnockDownEvent`, `sexLabSceneInterrupt`, and `requestActorExtraData`.

### Dynamic Defeat widget

Reads Defeat QTE widget script properties and keeps the on-screen Dynamic Defeat meter aligned with the native meter value (including desync fixes from later experimental builds).

### Soft dependencies

Detected from the load order at runtime (no hard requirement):

| Plugin | Effect |
| --- | --- |
| `SexLabDefeat_LRG_Patch.esp` | Dynamic Defeat / Vulnerability forms & MCM values |
| `DeviousFramework.esm` | Optional DFW vulnerability on hit |
| `BaboInteractiveDia.esp` | Ignore faction for Babo Dialogue |
| `ZaZAnimationPack.esm` | ZaZ-related feature flag |

MCM / script config is read live from Defeat Papyrus objects (and LRG fields when the patch ESP is present).

### Other native behavior

- `DoDetect` hook to reduce detection interference while defeated actors are processed
- Optional Papyrus “unhook from framerate” for registered natives (`General.UnhookPapyrus`)
- Logging to the SKSE log directory (`SexLabDefeat.log`)

## Requirements

### Players

- [SKSE](https://skse.silverlock.org/) matching your Skyrim build
- **SexLab Defeat** (Bane V5.3.6 / maintained SE package recommended)
- **SexLab Framework** and Defeat’s usual soft deps (SkyUI, PapyrusUtil / JContainers as shipped with your Defeat package)
- Matching **Andb Defeat Papyrus patch** (scripts that declare and call `defeat_skse_api`)

Optional: Dynamic Defeat LRG patch ESP, Devious Framework, Babo Dialogue, ZaZ — same as the script patch.

### Developers (build)

- Visual Studio 2022 with C++ desktop workload
- CMake ≥ 3.24
- [vcpkg](https://github.com/microsoft/vcpkg) with `VCPKG_ROOT` set
- Git (for the CommonLibSSE-NG submodule)

## Installation (players)

1. Install SexLab Defeat and its requirements.
2. Install the Andb Papyrus patch **after** Defeat / LRG so `.pex` files override the originals.
3. Copy this plugin into your mod folder:

```text
SKSE/Plugins/SexLabDefeat.dll
SKSE/Plugins/SexLabDefeat.ini   # optional; defaults work if omitted from a package
```

4. Launch the game once, then confirm Defeat MCM options (Player Victim, NvN, Dynamic Defeat, Vulnerability).
5. If something fails, check `%USERPROFILE%\Documents\My Games\Skyrim Special Edition\SKSE\SexLabDefeat.log` (or the AE/VR equivalent path).

Load order keeps `SexLabDefeat.esp` (and `SexLabDefeat_LRG_Patch.esp` if used) as usual. This project ships the SKSE DLL, not a new ESP.

## Configuration

`SKSE/Plugins/SexLabDefeat.ini`:

```ini
[General]
# Unhook Papyrus natives from the framerate (safer, slower)
UnhookPapyrus = 1

# 0 = errors only path effectively off for info; 1 = info; 2 = trace
Logging = 2

[CombatManager]
# Minimum interval (ms) before another hit from the same source/actor counts
# (projectile / magic spam guard)
HitSpamGuardMs = 500
```

Gameplay knobs (thresholds, Dynamic Defeat multipliers, sexuality filters, On/Off toggles) remain in the Defeat MCM / Papyrus config scripts; the DLL reads those values at runtime.

## Building from source

```text
andb - SL Defeat NG/
├── cmake/                 # XSEPlugin, Plugin.h.in, overlay ports
├── extern/CommonLibSSE-NG # submodule (alandtse CommonLibVR `ng` branch)
├── include/Defeat.h       # Shared types / interfaces
├── src/                   # Plugin sources
├── SKSE/Plugins/          # Sample INI (+ local build artifacts, gitignored DLL)
├── CMakePresets.json      # preset `ALL` (SE + AE + VR)
└── BuildRelease.bat
```

```powershell
git submodule update --init --recursive

# Optional: deploy path for post-build copy (gitignored)
# CMakeUser.txt → set(OUTPUT_FOLDER "D:/Mods/Andb.Defeat.DLL")

.\BuildRelease.bat
# or:
cmake -S . --preset=ALL
cmake --build build --config Release
```

Output: `build/Release/SexLabDefeat.dll` (and `.pdb`). With `OUTPUT_FOLDER` set, the build also copies the DLL/PDB into `<OUTPUT_FOLDER>/SKSE/Plugins/`.

Dependencies are pulled via vcpkg (`vcpkg.json`): CommonLib utilities, spdlog/fmt, Boost, Detours, ImGui, DirectXTex, xbyak, magic_enum, nlohmann-json, etc.

## Project layout (runtime)

| Component | Responsibility |
| --- | --- |
| `plugin.cpp` | SKSE load, messaging, hook/sink install |
| `DefeatManager` | Forms, soft deps, Papyrus binding, widget reset |
| `DefeatCombatManager` | Hit → knockdown / dynamic meter / interrupts |
| `DefeatActor` / `DefeatActorManager` | Per-actor state, immunity, extra-data cache |
| `DefeatHooks` | Trampoline / event sinks |
| `DefeatWidget` | Dynamic Defeat / QTE meter UI bridge |
| `PapyrusInterface/*` | Script object vars, async extra-data queue |

## Companion repositories

| Repo | Contents |
| --- | --- |
| This repo (`SLDefeatNG`) | SKSE plugin sources |
| Andb `SLDefeat` Papyrus patch | Patched Defeat scripts + `defeat_skse_api.psc` |

Branch lineage on the script side is typically `andb_lrg` (Defeat + Bane + LRG + Andb SKSE bridge). Keep DLL and script revisions from the same experimental generation when possible (`Changelog.txt` on both sides).

## Credits

- **Goubo** — SexLab Defeat
- **Bane** — Defeat V5.3.6 maintenance
- **AndrewLRG (LRG)** — Dynamic Defeat LRG Patch
- **Andb** — SKSE plugin and Papyrus bridge
- **alandtse / CommonLibSSE-NG** contributors — NG CommonLib

Original mod content remains the property of its authors. Distribute this plugin only together with permitted Defeat / LRG packages and under their distribution rules (typically LoversLab for Defeat).

## License

Plugin packaging metadata lists **GPL-3.0** (`vcpkg.json`). Respect upstream Defeat / LRG permissions for the overall mod package; do not redistribute original Defeat assets as your own work.
