ude Code Master Prompt: Build a Token-Efficient, Intelligent Git Merge Conflict Resolution System

## Primary Objective

Analyze, optimize, and implement an intelligent Git merge conflict resolution pipeline that uses an LLM to resolve merge conflicts while minimizing API costs, output tokens, unnecessary model invocations, and processing time.

The highest priority is **minimizing cost per successfully merged PR without sacrificing merge correctness, code quality, or semantic integrity.**

Do not simply implement a basic LLM-powered merge resolver. Engineer the entire workflow to avoid unnecessary LLM calls in the first place.

Before making changes:
1. Inspect the existing repository architecture.
2. Identify the current merge workflow and every point where LLM calls are triggered.
3. Identify opportunities to reduce token consumption, eliminate redundant calls, and avoid unnecessary conflict resolution.
4. Understand the existing testing infrastructure.
5. Develop an implementation plan before modifying files.

Prioritize the following requirements in order.

---

# PRIORITY 1: Token-Minimizing Prompt Architecture

**This is the highest-priority requirement.**

LLM output tokens typically cost significantly more than input tokens. The architecture must therefore aggressively minimize both input and output token consumption, particularly when resolving conflicts in large source files.

### 1.1 Extract Only the Relevant Conflict Hunk

Never send an entire source file to the LLM when only a small section contains a merge conflict.

Implement intelligent conflict extraction.

For every conflict, extract:

- The conflicting code from the `<<<<<<<` marker.
- The common ancestor section when available (`|||||||`).
- The incoming branch section.
- The `=======` separator information as structural metadata.
- The `>>>>>>>` marker information as structural metadata.
- Approximately 25 lines of surrounding context above the conflict.
- Approximately 25 lines of surrounding context below the conflict.
- The commit messages associated with both branches.
- The source file path and relevant language information.

Example:

Instead of sending a 4,000-line `vulkan_pipeline.cpp` file, send only the relevant 15-line conflict and its surrounding context.

Target a compact prompt that gives the model sufficient information to make a correct decision without unnecessary repository content.

### 1.2 Make Context Adaptive

Do not blindly assume 25 lines are always sufficient.

Implement an adaptive context strategy:

- Default: 25 lines above and below.
- If the conflict involves incomplete functions, expand context to include the entire relevant function.
- If the conflict involves class declarations, include the relevant class definition.
- If the conflict involves imports, declarations, macros, or type definitions, include the relevant dependencies.
- If additional context is required to understand a symbol, retrieve only the relevant definition.
- Avoid retrieving entire files unless absolutely necessary.

Use language-aware parsing where practical.

The objective is to provide the smallest sufficient context, not simply the smallest possible prompt.

### 1.3 Include Branch Intent

A merge conflict is not purely a textual problem.

The model must understand what each branch was attempting to accomplish.

Include:

- Source branch commit messages.
- Target branch commit messages.
- Relevant PR descriptions when available.
- Relevant commit diffs when necessary.

Do not send complete commit histories.

Prefer concise, deduplicated summaries of the commits that actually introduced the conflicting changes.

### 1.4 Strict Hunk-Only Output

The LLM must return ONLY the resolved replacement code.

Use an output contract similar to:

"You are an expert software engineer resolving a Git merge conflict.

Resolve the provided conflict while preserving the intended behavior of both branches.

Requirements:
- Return ONLY the replacement code.
- Do not include explanations.
- Do not include reasoning.
- Do not include Markdown.
- Do not include comments explaining the merge.
- Do not repeat surrounding context.
- Do not output Git conflict markers.
- Preserve existing formatting and coding conventions.
- Do not introduce unrelated modifications.
- Do not remove functionality from either branch unless the changes are genuinely incompatible.
- If the conflict cannot be resolved safely with the provided information, explicitly signal that human intervention is required."

Use a machine-readable response contract if necessary to distinguish successful resolution from unresolved conflicts.

Avoid verbose JSON responses unless they are required by the application.

### 1.5 Enforce Output Token Limits

Implement strict output token limits.

The maximum output should be proportional to the size of the conflict hunk.

For example:

- Small conflict: 100–300 output tokens.
- Medium conflict: 300–800 output tokens.
- Large conflict: dynamically calculated limit.

Never allow the model to regenerate an entire source file.

If the output exceeds the expected size:
1. Reject the response.
2. Record the token overrun.
3. Retry with a stricter output contract or escalate to human review.

Do not silently truncate code.

### 1.6 Measure Token Efficiency

Track:

- Input tokens per conflict.
- Output tokens per conflict.
- Original conflict size.
- Resolved code size.
- Number of LLM calls per PR.
- Cost per resolved conflict.
- Cost per successfully merged PR.

Use these measurements to continuously improve prompt efficiency.

---

# PRIORITY 2: Eliminate Redundant LLM Calls Before Merge Resolution

**The system should avoid invoking the LLM whenever a conflict can be prevented or eliminated through normal Git operations.**

### 2.1 Avoid Processing Every Intermediate Commit

Do not resolve conflicts against every intermediate contributor commit.

A PR containing 80 commits, including:

- Fix typo.
- WIP.
- Formatting.
- Rename variable.
- Fix formatting.
- Update comments.

Should not trigger 80 separate LLM resolution cycles.

Instead, resolve against the final intended branch state whenever possible.

Prefer:

- Direct merging of the PR tip.
- Squash merging.
- Merge-based integration.
- Appropriate use of `git-imerge` when incremental integration is genuinely beneficial.

Do not automatically rebase through every commit when doing so creates redundant conflict-resolution work.

### 2.2 Detect and Eliminate Formatting-Only Conflicts

Formatting conflicts should never consume expensive LLM tokens unnecessarily.

Implement automatic formatting normalization before attempting semantic conflict resolution.

Recommended workflow:

1. Detect the repository's formatting configuration.
2. Identify supported formatters.
3. Run the appropriate formatter consistently.
4. Normalize whitespace and line endings.
5. Attempt the Git merge again.
6. Only send remaining semantic conflicts to the LLM.

Support tools such as:

- clang-format
- Prettier
- Black
- gofmt
- rustfmt

Use the repository's existing configuration rather than imposing arbitrary formatting rules.

Never automatically reformat unrelated files.

### 2.3 Detect Trivial Conflicts

Implement deterministic resolution for conflicts that can be resolved safely without an LLM.

Examples:

- Whitespace-only differences.
- Equivalent formatting changes.
- Identical changes on both branches.
- Non-overlapping modifications.
- Certain import-order conflicts.
- Duplicate declarations introduced by identical changes.

Use Git's native merge capabilities first.

Only invoke the LLM when deterministic resolution fails.

### 2.4 Deduplicate Conflict Processing

If multiple commits or merge attempts produce the same effective conflict:

- Calculate a normalized conflict fingerprint.
- Include file path, relevant context, and conflict content in the fingerprint.
- Cache previously resolved conflicts.
- Reuse valid resolutions when the same conflict reappears.

The cache must be conservative.

Never reuse a resolution if the surrounding code, branch intent, dependencies, or relevant context have materially changed.

---

# PRIORITY 3: Build an Intelligent Multi-Stage Resolution Pipeline

Implement a staged resolution system that escalates only when necessary.

Recommended architecture:

**Stage 1: Native Git Resolution**
- Attempt normal Git merging.
- Accept clean merges.
- Avoid LLM usage entirely.

**Stage 2: Deterministic Conflict Resolution**
- Detect identical changes.
- Resolve whitespace-only conflicts.
- Resolve safe formatting conflicts.
- Handle other provably equivalent changes.

**Stage 3: Minimal-Context LLM Resolution**
- Extract the conflict hunk.
- Add adaptive surrounding context.
- Include relevant branch intent.
- Request hunk-only output.
- Validate the replacement.

**Stage 4: Context Expansion**
Only if the initial LLM attempt fails:
- Retrieve relevant function or class context.
- Retrieve necessary declarations.
- Add additional commit information.
- Retry with a more informative prompt.

**Stage 5: Human Escalation**
If the model cannot confidently resolve the conflict:
- Preserve the original conflict.
- Record the failure.
- Provide a concise explanation outside the resolution response.
- Request human intervention.

Never force an uncertain resolution merely to achieve a successful merge.

---

# PRIORITY 4: Validate Every LLM-Generated Resolution

Correctness is more important than saving tokens.

Every generated replacement must pass validation before being applied.

Implement:

### 4.1 Structural Validation
- Verify that the response contains only replacement code.
- Reject unexpected Markdown or explanations.
- Reject remaining conflict markers.
- Ensure the replacement is applied to the correct file and conflict boundaries.
- Detect accidental deletion of surrounding code.

### 4.2 Syntax Validation
Where possible, run:
- Language-specific parsers.
- Compiler checks.
- Linters.
- Formatting checks.

### 4.3 Semantic Validation
Run relevant tests and compare the resulting changes against the intended branch modifications.

Check that:
- Both branches' intended functionality is preserved.
- No unrelated functionality has disappeared.
- No duplicate declarations or incompatible definitions were introduced.
- No obvious API or type inconsistencies exist.

### 4.4 Safe Failure Handling

If validation fails:
- Do not automatically commit the result.
- Restore or preserve the original conflict state.
- Record the failed attempt.
- Escalate with sufficient diagnostic information.

Never sacrifice correctness for a lower token bill.

---

# PRIORITY 5: Optimize LLM Provider and Model Usage

Build a provider-independent LLM integration layer.

Requirements:

- Support configurable LLM providers.
- Support model selection.
- Support temperature and output limits.
- Track token usage and API costs.
- Allow inexpensive models to handle straightforward conflicts.
- Escalate difficult conflicts to more capable models only when necessary.

Use the cheapest model that consistently meets the required correctness threshold.

Do not automatically use the most expensive model for every conflict.

Avoid unnecessary retries.

Implement bounded retries with clear escalation rules.

---

# PRIORITY 6: Observability and Cost Accounting

Build visibility into the entire merge-resolution process.

Track the following metrics:

| Metric | Purpose |
|---|---|
| Total PRs processed | Workload |
| Total conflicts | Conflict frequency |
| Automatically resolved conflicts | Deterministic success |
| LLM-resolved conflicts | LLM dependency |
| Human-escalated conflicts | Failure rate |
| Input tokens | Prompt cost |
| Output tokens | Generation cost |
| LLM calls per PR | Call efficiency |
| Average cost per conflict | Unit economics |
| Average cost per PR | Overall efficiency |
| Validation failures | Resolution quality |
| Cache hit rate | Deduplication effectiveness |

Provide structured logging.

Never log API keys, credentials, or unnecessary sensitive repository data.

The system should make it possible to compare token usage before and after optimization.

---

# PRIORITY 7: Engineering Quality and Implementation Requirements

While implementing the system:

1. Preserve the existing architecture where reasonable.
2. Avoid unnecessary dependencies.
3. Keep individual components modular.
4. Make conflict extraction independently testable.
5. Make prompt generation independently testable.
6. Make LLM response validation independently testable.
7. Make caching independently testable.
8. Make provider integration replaceable.
9. Add comprehensive automated tests.
10. Avoid introducing breaking changes without justification.

Create tests covering:

- Multiple conflict hunks in one file.
- Large source files with tiny conflicts.
- Conflicts with and without a common ancestor section.
- Nested or malformed conflict markers.
- Whitespace-only conflicts.
- Formatting-only conflicts.
- Identical changes from both branches.
- Large functions requiring adaptive context.
- Missing or irrelevant commit messages.
- Invalid LLM responses.
- Excessive output tokens.
- Repeated conflicts and cache hits.
- Failed syntax validation.
- Failed semantic validation.
- Human escalation.

Use realistic test fixtures.

---

# IMPLEMENTATION STRATEGY

Do not immediately rewrite the entire project.

Follow this process:

**Phase 1 — Repository Audit**
- Inspect the codebase.
- Identify the merge-resolution entry points.
- Map existing LLM calls.
- Identify redundant processing.
- Document the existing architecture.

**Phase 2 — Architecture Design**
Design the optimized pipeline.
Identify which existing components can be reused.
Determine the minimum set of required modifications.

**Phase 3 — Core Implementation**
Implement:
1. Minimal conflict extraction.
2. Adaptive context selection.
3. Branch-intent extraction.
4. Hunk-only prompt generation.
5. Strict response parsing.
6. Output token limits.
7. Deterministic resolution.
8. Formatting normalization.
9. Conflict fingerprinting and caching.
10. Validation and escalation.

**Phase 4 — Integration**
Integrate the pipeline into the existing Git workflow.

Ensure existing functionality remains intact.

**Phase 5 — Testing and Benchmarking**
Compare the original implementation against the optimized implementation.

Measure:
- Input tokens saved.
- Output tokens saved.
- Number of LLM calls eliminated.
- Average cost reduction.
- Merge success rate.
- Validation failure rate.
- Processing time.

**Phase 6 — Final Review**
Inspect all modifications.
Run tests.
Fix regressions.
Identify any remaining cost inefficiencies.

---

# NON-NEGOTIABLE DESIGN PRINCIPLES

1. Never send entire source files when conflict-level context is sufficient.
2. Never ask the LLM to regenerate an entire file.
3. Never pay for verbose explanations when only replacement code is required.
4. Never invoke the LLM for conflicts Git can resolve safely.
5. Never process every intermediate commit unnecessarily.
6. Never let formatting-only changes trigger expensive semantic resolution.
7. Never reuse cached resolutions when their context has materially changed.
8. Never automatically accept an invalid or unverified resolution.
9. Never optimize token consumption at the expense of merge correctness.
10. Always measure actual cost per successfully merged PR.

# FINAL EXPECTATION

Implement a production-quality, token-efficient merge conflict resolution system.

The primary optimization target is:

**Minimum total LLM cost per successfully merged PR.**

The secondary optimization targets are:

- Minimum LLM calls.
- Minimum input tokens.
- Minimum output tokens.
- Minimum processing time.
- Maximum deterministic resolution rate.
- Maximum merge correctness.
- Maximum reliability.

Do not stop after producing an implementation plan. After inspecting the repository and presenting your plan, proceed with implementation unless a critical architectural ambiguity or destructive operation requires clarification.

At completion, provide a concise engineering report containing:

1. What was changed.
2. Which files were modified.
3. How the new resolution pipeline works.
4. How redundant LLM calls are prevented.
5. How token consumption is reduced.
6. Test results.
7. Measured before-and-after token savings, if measurable.
8. Remaining limitations and recommended next steps.

Prioritize measurable improvements over theoretical optimizations.
