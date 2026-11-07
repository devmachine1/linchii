---
name: fork-sync
description: Refresh our fork's lichii-pc branch from the newest zortos293/Lichii nx2 and the open PRs worth taking, then build and publish it.
disable-model-invocation: true
---

# fork-sync

Brings `lichii-pc` up to date for the PC and game in `docs/fork/README.md`. Read that file first: its **Target**, **Picking a PR** and **Ledger** sections drive every decision below. Report to the owner in Portuguese (pt-BR).

Each step's last line is its completion criterion.

## 1. Protect the data

Ask the owner to close Lichii if `Get-Process lichii` finds it; never kill it (a killed process can truncate the pipeline cache, which the next boot then deletes). Then run `powershell -ExecutionPolicy Bypass -File tools/fork/backup-lichii.ps1`.

Done when the script printed a `Backup:` line with a save-file count above zero.

## 2. Take upstream nx2

`git fetch zortos`, check out `lichii-pc` in `C:\dev\Lichii` (a build of the same checkout may be running: if so, work in a `git worktree` and fast-forward afterwards), then `git merge zortos/nx2`. Diff `src/video_core/pipeline_cache_files.h` across the merge: if `VulkanPipelineCacheVersion` changed, tell the owner before going on, and plan a cache migration (`tools/nx2/migrate_pipeline_cache.py`).

Done when the merge is committed and the cache version is either unchanged or the owner has said yes.

## 3. Triage the open PRs

`gh pr list -R zortos293/Lichii --state open --limit 200`. For every PR not yet in the ledger, and every `merged` row whose PR gained commits since, read its body and diff (`gh pr view`, `gh pr diff`) and decide against **Picking a PR**. Grep each diff for `VulkanPipelineCacheVersion`. Also scan merged PRs and commits of `Djihads80/Eglantine` since the last run for OoT or NVIDIA fixes the Lichii line lacks.

Done when every open PR has a ledger row: merged, skipped, superseded or later, each with a reason.

## 4. Merge the picks

For each pick: `git fetch zortos pull/<n>/head:pr/<n>`, then `git merge --no-ff pr/<n>` with message `Merge zortos293/Lichii PR #<n>: <title>`. One merge commit per PR keeps each revertible. On a conflict, read both sides: resolve it when the two changes compose (note it in the ledger and the commit message), otherwise abort and mark the PR `later`. Re-check the cache version after the last merge.

Done when every pick is merged or recorded as conflicting, and the ledger matches the branch.

## 5. Build

`tools\fork\build.bat C:\dev\lichii-build\<new dir>`, run in the background; it takes 10-60 min. Use a new folder each time so the owner's current build stays playable. If the link fails, read the first `error:` in the log before changing anything.

Done when `<new dir>\bin\lichii.exe` exists with a fresh timestamp and the log ends without `FAILED`.

## 6. Publish and hand over

Commit the ledger, push `lichii-pc` to `origin` (`gabcmelo/Lichii`), and give the owner:

- the new exe path, and env switches to add to `%APPDATA%\eden\nx2_switches.txt` for newly merged opt-ins;
- a table of what came in, what was skipped and why;
- anything they must test (conflicts we resolved, new opt-ins), and how to go back (previous build folder, backup folder).

Done when the push succeeded and the owner has the table.
