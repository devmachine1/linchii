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
| #313 | merged | Pipeline eviction bounded by 1/8 of RAM. Re-merged 2026-11-07 for 09d20d291a, which composes it with #550 (now in nx2). |
| #321 | merged | Held draw builds its own pipeline; fewer 1-2 s spikes. |
| #463 | merged | No stale uploads into new images; OoT Castle Town ~+20%. |
| #559 | merged | Waits only for a macro DMA read's own query reports. Re-merged after its rebase. |
| #437 | merged | Migrates v53 pipeline caches into v54 at boot. |
| #571 | merged | AV1 movies in time with audio (OoT title movie). |
| #570 | merged | Occlusion reset threshold; OoT water/window flicker. |
| #337 | merged | Fence-flagged query reports kept pending; OoT white flashes. Conflicts with #559 resolved by us (twice): watch for load hangs. |
| #288 | merged | Skips uploads a join copy overwrites (part of #408's stutter work). |
| #268 | merged | 12 GB card fills 5/8 of its budget before collecting textures. |
| #259 | merged | Opt-in `EDEN_NX2_PREFLUSH=1`: OoT 26 → 40 fps on 16 GB, some flicker. |
| #645 | merged | Retries the 14 GiB fastmem allocation near the commit limit; losing fastmem made OoT ~1.4x slower. |
| #609 | merged | Fence thread no longer clobbers `dma_increment_once` (hang, #479). |
| #600 | merged | `nx2:aud:u` returns the registered managers (silent audio). |
| #576 | merged | Valid long-path manifest namespace. |
| #649 | merged | Rename of a shadowing local in `BypassOotNetwork`; no behaviour change. |
| #415 | merged | Native compute marks only the bytes its stores reach; OoT white/flickering leaves, bad exposure, 24 → 45 fps at the Deku Tree (RX 6600 XT). Mask widened to u64 by us for nx2's 64 storage buffers. Back to the old way: setting `native_compute_write_mark=Full`. |
| #278 | superseded | nx2 already skips the read throttle for Switch 2 titles. |
| #591 | superseded | #437 already keeps v53 caches. |
| #251 | superseded | #571 already syncs OoT movies. |
| #387, #253 | superseded | nx2 has DLSS for OoT (#606, imports #456). |
| #408 | skipped | Bumps the cache version again (55); revisit with the owner. |
| #557 | skipped | Alternative to #551, already in nx2; conflicts. |
| #573 | skipped | AMD-focused; re-check its split-thread parts for NVIDIA. |
| #335, #626, #579 | skipped | Pre-build or warm more pipelines at boot (more RAM, minutes of CPU). |
| #555 | skipped | Only acts with `EDEN_NX2_PRELOAD_ALL_PIPELINES`, which won't fit in 16 GB. |
| #611, #235 | skipped | Only act on settings we don't use (`skip_cpu_inner_invalidation`, GPU accuracy High). |
| #231, #166 | skipped | Large stacks, gains behind switches, 1.5-4 fps measured. |
| #643, #614, #642, #610, #638, #575, #114, #143 | skipped | Other GPUs or games (MoltenVK/Intel/near-limit devices, Duskbloods, Tomodachi, Ampere GPU-time for another game). #575 is a general device-loss fix: revisit if OoT loses the device. |
| #629, #634, #448, #354, #378, #300, #494, #223 | skipped | AMD only. |
| #639, #631, #630, #628, #617, #632, #417 | skipped | macOS/MoltenVK/ARM64 host/Linux. |
| #652, #620, #612, #586, #516, #618, #543, #213, #180, #219, #267 | skipped | Docs, UI, rename, features (photo mode, rumble, HDR), CI, write-ups. |
| #637, #552, #561, #560 | skipped | Duskbloods, online services. |
| #243, #345 | later | Split-front macro DMA reads; #243 +8.5% in OoT Kokiri (RTX 4090). Needs a manual port onto #559/#337; #345 is its superset. |
| #621 | later | Follow-up to nx2's 50 us poll; unmeasured. Candidate for a debug-build A/B. |
| #589 | later | Take it if the log shows `FrontRunMacro: Native MME ... failed` with a black screen. |
| #635 | later | Keeps OoT glows out of DLSS history; only if the owner switches FSR → DLSS. |
| #640, #615, #161, #376 | later | Large or unmeasured (cache record cleanup, Eglantine #754 port, timing bundle, VRAM-pressure GC). |
| #264, #296, #488 | later | Generic, no OoT measurement. |

## Local paths

- Source checkout: `C:\dev\Lichii`. Builds: `C:\dev\lichii-build\<dir>\bin\lichii.exe`, one folder per build so the working one stays playable.
- User data: `%APPDATA%\eden`. Backups: `%USERPROFILE%\LichiiBackups` (`base\` holds the keys and pre-migration caches).
