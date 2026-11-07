# Our fork: `lichii-pc`

`gabcmelo/Lichii` branch `lichii-pc` is `zortos293/Lichii` `nx2` plus open upstream PRs picked for one PC and one game. `/fork-sync` (`.claude/skills/fork-sync/SKILL.md`) refreshes it.

## Target

- PC: Ryzen 7 5700X (8c/16t, Zen 3), **16 GB RAM**, RTX 3060 12 GB (NVIDIA, Ampere), Windows 11.
- Game: The Legend of Zelda: Ocarina of Time (`0400486024DEC000`), handheld, 1x, GPU accuracy Low, async shaders, FSR, LSFG frame generation.
- Binding constraint: host RAM. With a large pipeline cache the emulator reached 11.9 GB and Windows paged ~25k pages/s, dropping OoT to 20-30 FPS while the GPU sat at 48%.

## Picking a PR

Take a PR when all hold:

- It targets this game or this hardware class (NVIDIA, Windows, 16 GB), or fixes a crash/hang any title can hit.
- It keeps `VulkanPipelineCacheVersion` (`src/video_core/pipeline_cache_files.h`). A bump discards every pipeline cache: take one only with the owner's yes, and run `tools/nx2/migrate_pipeline_cache.py` or rely on a migration in the PR.
- It does not grow boot-time pipeline builds or resident memory by default (RAM is the bottleneck).
- The author measured a gain or a fix, or it is a few lines with an obvious effect.

Opt-in switches (env vars, settings that default to off) are fine: they cost nothing until enabled. Experiment switches stay ours; they are not meant for upstream nx2 (see AGENTS.md "Toggles and merging").

## Ledger

One row per PR decided. `merged` rows are in `lichii-pc`; re-merge `pr/<n>` when the PR gains commits. Drop a row once the PR lands in `zortos/nx2`.

| PR | Decision | Why / how to enable |
|---|---|---|
| #431 | merged | Trims all-zero guest pages; OoT on 16 GB: 10.7-12.5 GB → 6.0-6.7 GB. Enable `EDEN_NX2_TRIM_ZERO=5`. |
| #313 | merged | Pipeline eviction bounded by 1/8 of RAM, keeps boot prebuilds. Chosen over #550 (keeps all prebuilds resident, ~600 MB more). |
| #321 | merged | Held draw builds its own pipeline; fewer 1-2 s spikes. |
| #465 | merged | Opt-in `native_async_draws=Bounded` (8 ms/frame). Can flash one frame. |
| #446 | merged | GPU thread polls every 50 us; OoT Kokiri ~+11%. |
| #463 | merged | No stale uploads into new images; OoT Castle Town ~+20%. |
| #559 | merged | Waits only for a macro DMA read's own query reports. |
| #475 | merged | HMMA on NVIDIA cooperative matrices, +12-24%. Off: `EDEN_NX2_NO_COOPERATIVE_HMMA=1`. |
| #437 | merged | Migrates v53 pipeline caches into v54 at boot. |
| #571 | merged | AV1 movies in time with audio (OoT title movie). |
| #570 | merged | Occlusion reset threshold; OoT water/window flicker. |
| #337 | merged | Fence-flagged query reports kept pending; OoT white flashes. Conflict with #559 resolved by us: watch for load hangs. |
| #299 | merged | Dynamic FPS stays real down to 10 fps. |
| #250 | merged | Skips GPU-write invalidations of empty pages (part of #408's stutter work). |
| #288 | merged | Skips uploads a join copy overwrites (part of #408's stutter work). |
| #268 | merged | 12 GB card fills 5/8 of its budget before collecting textures. |
| #259 | merged | Opt-in `EDEN_NX2_PREFLUSH=1`: OoT 26 → 40 fps on 16 GB, some flicker. |
| #440, #427, #495 | merged | Image/framebuffer/descriptor limits handled instead of crashing. |
| #484 | merged | Maximized window keeps its size when a game boots. |
| #278 | superseded | nx2 already skips the read throttle for Switch 2 titles. |
| #408 | skipped | Bumps the cache version again (55); revisit with the owner. |
| #550 | skipped | Alternative to #313. |
| #557 | skipped | Alternative to #551, already in nx2; conflicts. |
| #573 | skipped | AMD-focused; re-check its split-thread parts for NVIDIA. |
| #335 | skipped | Pre-warms more pipelines at boot (more RAM). |
| #481, #264, #296, #488 | later | Generic, no OoT measurement. |
| #456, #387, #253 | later | DLSS for OoT (RTX 3060 supports it); three alternatives, one dedicated round. |

## Local paths

- Source checkout: `C:\dev\Lichii`. Builds: `C:\dev\lichii-build\<dir>\bin\lichii.exe`, one folder per build so the working one stays playable.
- User data: `%APPDATA%\eden`. Backups: `%USERPROFILE%\LichiiBackups` (`base\` holds the keys and pre-migration caches).
