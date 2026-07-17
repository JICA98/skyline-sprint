# Tasks — End-to-End Agent Plan for Skyline Sprint

## How to Use This File

Execute phases in order. Do not skip a phase because later code appears more interesting.

For every checkbox:

1. inspect the relevant source or official OpenOrbis sample;
2. implement;
3. build;
4. test;
5. record evidence in `PROGRESS.md`;
6. mark complete only after evidence exists.

Commands and file paths below are goals, not permission to invent toolchain syntax. Derive exact PS4 compilation and packaging commands from the selected current OpenOrbis release and working samples.

---

# Phase 0 — Repository Reconnaissance and Version Lock

## 0.1 Inspect OpenOrbis

- [ ] Clone or open the user-selected OpenOrbis PS4 Toolchain checkout.
- [ ] Inspect the README, release notes, `DOCKER.md`, `docs/`, `samples/`, `extra/`, and current build scripts.
- [ ] Identify the newest stable release suitable for building the samples.
- [ ] Prefer v0.5.4 when it is still the newest compatible release; otherwise document why a different tag is selected.
- [ ] Record:
  - release tag;
  - commit hash;
  - host OS;
  - `clang --version`;
  - `ld.lld --version`;
  - Python version;
  - package-tool dependencies;
  - `OO_PS4_TOOLCHAIN` path.
- [ ] Confirm whether the checkout contains prebuilt libraries/tools. If not, use the official release package or follow the official source-build instructions.
- [ ] Build one unmodified simple official sample.
- [ ] Build the official SDL2 mini-game/sample.
- [ ] Identify the exact commands that produce:
  - object files;
  - target ELF/OELF;
  - `eboot.bin`;
  - package metadata;
  - final `.pkg`.
- [ ] Identify the current package module requirements from a working sample.
- [ ] Identify the exact controller API and button constants used by a working sample.
- [ ] Save findings in `docs/OPENORBIS_BASELINE.md`.

### Acceptance criteria

- An official sample builds successfully, or a precise external/environment blocker is documented with command output.
- No proprietary SDK is installed or referenced.
- All future build decisions can cite a verified sample file or current header.

---

# Phase 1 — Project Bootstrap

## 1.1 Create repository structure

- [ ] Create the source layout described in `general_instructions.md`.
- [ ] Add `.gitignore` for host build output, target build output, package staging, logs, and generated temporary files.
- [ ] Add `LICENSE`.
- [ ] Add `THIRD_PARTY_NOTICES.md`.
- [ ] Add initial `README.md`.
- [ ] Add `BUILDING.md`, `TESTING.md`, `ARCHITECTURE.md`, `KNOWN_ISSUES.md`, `CHANGELOG.md`, and `RELEASE_CHECKLIST.md`.
- [ ] Add `PROGRESS.md`.

## 1.2 Bootstrap from verified SDL2 sample

- [ ] Copy only the minimal required build structure from the current official SDL2 sample.
- [ ] Preserve required licence notices.
- [ ] Rename package/project identity to Skyline Sprint.
- [ ] Remove unused sample content.
- [ ] Keep package module files only when required and redistributable.
- [ ] Make the target show a blank coloured scene and cleanly exit through the verified mechanism.
- [ ] Add build ID and toolchain ID constants.

## 1.3 Desktop target

- [ ] Create a desktop SDL2 target for game-logic development.
- [ ] Keep platform-specific code behind a small interface.
- [ ] Add keyboard mappings:
  - arrows/A-D: movement;
  - Space: jump;
  - Escape: pause/back;
  - F1: compact diagnostics;
  - F2: full diagnostics;
  - F3: automatic test;
  - F4: stress mode.
- [ ] Ensure desktop and PS4 targets compile the same gameplay modules.

### Acceptance criteria

- Desktop target opens, renders, and exits cleanly.
- PS4 target compiles to the stage reached by the verified sample.
- No game logic contains PS4-only headers.

---

# Phase 2 — Deterministic Core and Test Harness

## 2.1 Fixed timestep

- [ ] Implement a 60 Hz fixed-timestep accumulator loop.
- [ ] Clamp excessive frame deltas.
- [ ] Count simulation ticks.
- [ ] Add render interpolation parameter.
- [ ] Add a headless simulation entry point for tests.

## 2.2 Deterministic PRNG

- [ ] Implement PCG32, xorshift, or another small explicitly documented PRNG.
- [ ] Add known-vector tests.
- [ ] Store seed in game state.
- [ ] Support fixed and selected seeds.
- [ ] Format seeds as readable hexadecimal.

## 2.3 Core state machine

Implement:

- [ ] Boot.
- [ ] Title.
- [ ] Gameplay.
- [ ] Pause.
- [ ] Failure.
- [ ] Settings.
- [ ] Test modes.
- [ ] Diagnostics.
- [ ] Credits.
- [ ] Fatal error.

## 2.4 Logging

- [ ] Add severity levels: debug, info, warning, error.
- [ ] Add a fixed-capacity circular in-memory log.
- [ ] Add platform log output.
- [ ] Add file logging only through a verified writable path.
- [ ] Prevent per-frame repetitive warnings.
- [ ] Include build ID, seed, tick, and subsystem where relevant.

### Acceptance criteria

- The same seed and input script produce the same state hash in repeated desktop runs.
- Long frame gaps do not destabilize physics.
- The log remains bounded.

---

# Phase 3 — Renderer and Asset Foundation

## 3.1 Logical renderer

- [ ] Implement 1920×1080 logical coordinates.
- [ ] Preserve aspect ratio.
- [ ] Calculate letterbox/pillarbox regions.
- [ ] Implement safe-area helpers.
- [ ] Add camera transforms.
- [ ] Add primitive drawing helpers.
- [ ] Add texture loading using the exact current sample path.
- [ ] Add a readable fatal-error screen after renderer initialization.

## 3.2 Bitmap text

- [ ] Implement an original tiny bitmap font or integrate the verified official font sample with a properly licensed font.
- [ ] Support:
  - uppercase;
  - lowercase if needed;
  - digits;
  - punctuation used by diagnostics;
  - hexadecimal values.
- [ ] Add alignment and simple wrapping.
- [ ] Avoid heap allocation per glyph or per frame.

## 3.3 Deterministic assets

- [ ] Write `scripts/generate_assets.py`.
- [ ] Generate original:
  - logo;
  - icon;
  - platform tiles;
  - shard;
  - drone;
  - warning pattern;
  - splash;
  - WAV effects.
- [ ] Add `scripts/verify_assets.py`.
- [ ] Add asset checksums.
- [ ] Add all asset licences.
- [ ] Ensure generated assets cannot be confused with Mario assets.

### Acceptance criteria

- Title placeholder scene renders on desktop.
- Assets regenerate identically.
- Missing assets generate a readable error rather than a crash.

---

# Phase 4 — Input System

## 4.1 Input abstraction

- [ ] Define logical actions independent of physical buttons.
- [ ] Add current/previous state and pressed/released edges.
- [ ] Add analog dead zones.
- [ ] Add menu repeat delay and repeat rate.
- [ ] Add input recording for deterministic test scripts.

## 4.2 Desktop input

- [ ] Keyboard.
- [ ] SDL game controller when available.
- [ ] Connection/disconnection handling.

## 4.3 PS4 input

- [ ] Implement using the exact current official SDL2/pad sample approach.
- [ ] Verify each required logical action.
- [ ] Handle no controller.
- [ ] Handle controller reconnect.
- [ ] Record raw values for the input visualizer.
- [ ] Use vibration only if a verified supported API exists; make it optional.

### Acceptance criteria

- Menus never double-activate from one press.
- Pause does not immediately unpause.
- Input visualizer reflects real state.
- Controller loss does not crash.

---

# Phase 5 — Player Controller and Collision

## 5.1 Player state

- [ ] Position and previous position.
- [ ] Velocity.
- [ ] Grounded state.
- [ ] Facing direction.
- [ ] Coyote timer.
- [ ] Jump buffer timer.
- [ ] Optional second-jump state.
- [ ] Alive/failure state.
- [ ] Animation timer.

## 5.2 Horizontal movement

- [ ] Acceleration.
- [ ] Deceleration.
- [ ] Air control.
- [ ] Maximum speed.
- [ ] Camera-zone constraints.
- [ ] Central tuning constants.

## 5.3 Jump

- [ ] Jump impulse.
- [ ] Coyote time.
- [ ] Input buffering.
- [ ] Variable jump height.
- [ ] Faster downward control if included.
- [ ] Optional second jump only after base jump is stable.

## 5.4 Collision

- [ ] AABB or capsule-like collision.
- [ ] Swept/sub-stepped movement.
- [ ] Stable floor resolution.
- [ ] Side collision.
- [ ] Ceiling collision.
- [ ] Kill plane.
- [ ] Hazard collision.
- [ ] Tests for tunneling and corner cases.

### Acceptance criteria

- Player can run and jump across a hand-authored test course.
- Landing is stable at 60 Hz.
- Different render frame patterns yield equivalent simulation results.
- Coyote time and buffered jump pass tests.

---

# Phase 6 — Chunk-Based Infinite World

## 6.1 Chunk representation

- [ ] Choose fixed chunk width.
- [ ] Store chunk index and local objects.
- [ ] Keep active chunks in a bounded window.
- [ ] Generate ahead.
- [ ] retire behind.
- [ ] Reuse storage or cap allocations.

## 6.2 Generator

Create deterministic pattern families:

- [ ] flat rooftop;
- [ ] small gap;
- [ ] raised platform;
- [ ] descending platforms;
- [ ] safe obstacle;
- [ ] shard arc;
- [ ] recovery platform;
- [ ] optional moving element after static patterns are stable.

## 6.3 Reachability validator

- [ ] Calculate maximum safe gap from actual tuned physics.
- [ ] Validate landing width.
- [ ] Validate vertical transitions.
- [ ] Ensure at least one route.
- [ ] Reject impossible obstacle combinations.
- [ ] Use deterministic fallback pattern.
- [ ] Count fallbacks in diagnostics.

## 6.4 World rebasing

- [ ] Rebase origin before floating-point precision becomes risky.
- [ ] Move active world objects consistently.
- [ ] Preserve score/distance/chunk index.
- [ ] Add repeated-rebase tests.

### Acceptance criteria

- A one-hour headless simulation does not grow active chunk/object counts.
- Every generated chunk passes validation or deterministic fallback.
- Same seed produces identical chunk hashes.
- Origin rebasing does not visibly jump the player or camera.

---

# Phase 7 — Gameplay Systems

## 7.1 Collectibles and score

- [ ] Energy shards.
- [ ] Pickup collision.
- [ ] Distance score.
- [ ] Pickup score.
- [ ] Capped multiplier.
- [ ] Multiplier decay.
- [ ] Session best.

## 7.2 Hazards

- [ ] Gaps.
- [ ] Static barriers.
- [ ] One simple original drone type after barriers are stable.
- [ ] Clear telegraphing.
- [ ] Collision tests.

## 7.3 Difficulty

- [ ] Difficulty tier based on distance.
- [ ] Gradual speed increase within safe limits.
- [ ] Gradual pattern weighting changes.
- [ ] Hard caps on gap and obstacle parameters.
- [ ] Recovery chunks at controlled intervals.

## 7.4 Failure and restart

- [ ] Failure freeze.
- [ ] Short visual effect.
- [ ] Stats summary.
- [ ] One-button retry.
- [ ] Replay same seed.
- [ ] Return to title.
- [ ] Complete deterministic reset of run state.

### Acceptance criteria

- A complete run can begin, score, fail, and restart.
- Replay seed reproduces initial world generation.
- Difficulty never creates validator-approved but physically impossible routes in automated tests.

---

# Phase 8 — Menus and HUD

Implement the exact flow in `ui_ux_design.md`.

## 8.1 Title

- [ ] Logo.
- [ ] Play.
- [ ] Test Modes.
- [ ] Settings.
- [ ] Diagnostics.
- [ ] Credits.
- [ ] Build ID and controller status.

## 8.2 HUD

- [ ] Distance.
- [ ] Score.
- [ ] Multiplier.
- [ ] Shards.
- [ ] mode badges.
- [ ] first-run controls strip.

## 8.3 Pause

- [ ] Resume.
- [ ] Restart.
- [ ] Diagnostics.
- [ ] Settings.
- [ ] Title.

## 8.4 Failure

- [ ] Distance.
- [ ] Score.
- [ ] Best.
- [ ] Seed.
- [ ] frame-time summary.
- [ ] Retry.
- [ ] Replay seed.
- [ ] Diagnostics.
- [ ] Title.

## 8.5 Settings and credits

- [ ] Audio.
- [ ] Screen shake.
- [ ] Particle level.
- [ ] Diagnostics default.
- [ ] Session reset.
- [ ] Licences and non-affiliation statement.

### Acceptance criteria

- Controller-only navigation works on every screen.
- Focus is always visible.
- Cross/Circle behavior is consistent.
- All critical UI remains inside safe area.

---

# Phase 9 — Audio and Effects

## 9.1 Audio

- [ ] Verify target audio support from current sample/docs.
- [ ] Initialize audio.
- [ ] Load generated WAV effects.
- [ ] Jump sound.
- [ ] Pickup sound.
- [ ] Landing sound.
- [ ] Menu sound.
- [ ] Failure sound.
- [ ] Mute and volume.
- [ ] Silent fallback.
- [ ] Diagnostics status.

## 9.2 Visual effects

- [ ] Fixed-capacity particle pool.
- [ ] Jump trail.
- [ ] Landing burst.
- [ ] Pickup burst.
- [ ] bounded failure shake.
- [ ] reduced-effects option.

### Acceptance criteria

- Audio failure does not prevent gameplay.
- Particle count is bounded.
- Effects do not alter deterministic collision state.

---

# Phase 10 — Emulator Diagnostics and Automated Test

## 10.1 Performance metrics

- [ ] Current FPS.
- [ ] Rolling average frame time.
- [ ] minimum and maximum frame time.
- [ ] fixed updates per rendered frame.
- [ ] frame-time sample ring buffer.
- [ ] rendered objects.
- [ ] active chunks.
- [ ] active particles.
- [ ] generator fallback count.
- [ ] origin rebase count.

## 10.2 Compact overlay

- [ ] Toggle.
- [ ] Stable formatting.
- [ ] No per-frame heap churn.
- [ ] Build, seed, tick, object, pad, and audio summary.

## 10.3 Full diagnostics

- [ ] Runtime page.
- [ ] Performance page.
- [ ] Input page.
- [ ] World page.
- [ ] Log page.
- [ ] Licence page.

## 10.4 Automatic test mode

Implement a deterministic repeatable test:

- [ ] Use a validated known seed or dedicated deterministic pattern.
- [ ] Script movement and jumps.
- [ ] Automatically restart after failure.
- [ ] Track run count.
- [ ] Track uptime.
- [ ] Track failures by reason.
- [ ] Allow immediate user cancellation.
- [ ] Log periodic summaries without spam.

## 10.5 Stress mode

- [ ] Increase only bounded decorative/render/audio workload.
- [ ] Keep collision route unchanged.
- [ ] Display mode badge.
- [ ] Record object count and frame-time change.
- [ ] Allow toggling from diagnostics.

### Acceptance criteria

- A tester can diagnose startup, video, input, audio, world generation, and performance without attaching a debugger.
- Automatic test can run repeatedly without memory growth.
- Stress mode is visibly and measurably heavier but bounded.

---

# Phase 11 — PS4 Package Integration

## 11.1 Package metadata

- [ ] Derive metadata from a current working OpenOrbis sample.
- [ ] Create a unique non-retail homebrew package identity.
- [ ] Use `Skyline Sprint`.
- [ ] Set version.
- [ ] Add original icon.
- [ ] Add original splash/background.
- [ ] Include only required files.
- [ ] Include required redistributable modules exactly as current sample dictates.
- [ ] Verify paths and case sensitivity.

## 11.2 Target build wrapper

Implement `build_ps4.sh`:

- [ ] `set -euo pipefail` or equivalent robust behavior.
- [ ] Validate `OO_PS4_TOOLCHAIN`.
- [ ] Validate compiler/linker/package tools.
- [ ] Print selected versions.
- [ ] Build in isolated directory.
- [ ] Produce `eboot.bin`.
- [ ] Stage package.
- [ ] Produce `.pkg`.
- [ ] Copy outputs to `dist/`.
- [ ] Generate SHA-256 sums.
- [ ] Print final artifact paths.

## 11.3 Clean build

- [ ] Remove all output.
- [ ] Regenerate assets.
- [ ] Rebuild desktop.
- [ ] Run tests.
- [ ] Rebuild PS4 target.
- [ ] Repackage.
- [ ] Compare expected generated-asset checksums.
- [ ] Record full command transcript.

### Acceptance criteria

- `eboot.bin` exists and is nonempty.
- `.pkg` exists and is nonempty.
- Package staging contains no host binaries, source secrets, retail content, or unrelated files.
- SHA-256 file is generated.
- Clean-build instructions are accurate.

---

# Phase 12 — Validation

## 12.1 Static inspection

- [ ] Search for prohibited proprietary names/files.
- [ ] Search for retail title IDs.
- [ ] Search for absolute developer-machine paths.
- [ ] Search for missing asset licences.
- [ ] Search for TODO/FIXME placeholders.
- [ ] Search for unbounded containers in hot paths.
- [ ] Search for ignored API return values.
- [ ] Verify release does not contain logs with personal paths.

## 12.2 Desktop soak tests

- [ ] Ten-minute manual normal run.
- [ ] One-hour headless world simulation.
- [ ] 1,000 restart cycles in headless state test.
- [ ] repeated pause/resume.
- [ ] repeated controller reconnect when testable.
- [ ] automatic-test soak.
- [ ] stress-mode soak.
- [ ] monitor active object counts and memory.

## 12.3 Emulator/PS4 checklist

When a runtime is available:

- [ ] Install package.
- [ ] Launch to title.
- [ ] Verify video.
- [ ] Verify controller.
- [ ] Verify audio or silent fallback.
- [ ] Start normal run.
- [ ] Fail and retry.
- [ ] Open compact diagnostics.
- [ ] Open every diagnostics page.
- [ ] Enable automatic test.
- [ ] Enable stress mode.
- [ ] Run ten minutes.
- [ ] collect logs/screenshots.
- [ ] record exact runtime/version and package checksum.

Do not mark target-runtime checks complete without direct evidence.

---

# Phase 13 — Documentation and Release

## 13.1 README

- [ ] Project description.
- [ ] Original/legal-content statement.
- [ ] Screenshot or generated preview.
- [ ] Controls.
- [ ] Test modes.
- [ ] Build summary.
- [ ] Package checksum.
- [ ] Licence.
- [ ] Non-affiliation statement.

## 13.2 BUILDING

- [ ] Supported host systems actually tested.
- [ ] Exact OpenOrbis release/commit.
- [ ] Exact dependencies.
- [ ] Environment variables.
- [ ] Desktop commands.
- [ ] PS4 commands.
- [ ] Troubleshooting from real encountered errors.
- [ ] Clean-build procedure.

## 13.3 TESTING

- [ ] Known seeds.
- [ ] Automatic test behavior.
- [ ] Stress mode.
- [ ] Diagnostics controls.
- [ ] Linsui/shadPS4 report template.
- [ ] Log collection steps.
- [ ] Clear distinction between verified and unverified results.

## 13.4 Release bundle

Create:

```text
dist/
├── SkylineSprint-<version>.pkg
├── eboot.bin
├── SHA256SUMS
├── BUILD_INFO.txt
├── THIRD_PARTY_NOTICES.md
├── SOURCE_COMMIT.txt
└── release-notes.md
```

- [ ] `BUILD_INFO.txt` includes toolchain release/commit and host tool versions.
- [ ] Release notes state that no retail game content or proprietary SDK material is included.
- [ ] Package and source licences are consistent.
- [ ] Tag the release if version control is available.

### Acceptance criteria

- A new developer can follow `BUILDING.md` from a clean checkout.
- A tester can identify exactly which package was tested.
- All release files are redistributable.

---

# Phase 14 — Final Audit

Before declaring completion, answer each item with evidence:

- [ ] Did an official OpenOrbis sample build first?
- [ ] Is the selected toolchain version pinned?
- [ ] Does desktop Skyline Sprint build?
- [ ] Do deterministic tests pass?
- [ ] Does the PS4 target produce `eboot.bin`?
- [ ] Does package creation produce `.pkg`?
- [ ] Are all assets original or permissively licensed?
- [ ] Is there any Sony SDK or retail game material? The required answer is **no**.
- [ ] Are package identity and metadata original?
- [ ] Is infinite generation bounded?
- [ ] Does automatic test mode work?
- [ ] Does stress mode remain bounded?
- [ ] Are diagnostics usable from the controller?
- [ ] Are failures reported honestly?
- [ ] Are runtime tests clearly distinguished from build-only validation?
- [ ] Are SHA-256 checksums present?
- [ ] Are all required docs complete?
- [ ] Are all remaining limitations listed in `KNOWN_ISSUES.md`?

## Final Agent Response Format

At completion, report:

1. **Result** — complete, partially complete, or blocked.
2. **What was built**.
3. **OpenOrbis version and commit**.
4. **Commands executed**.
5. **Tests passed**.
6. **Artifacts with exact paths and SHA-256 values**.
7. **PS4/Linsui verification status**.
8. **Known limitations**.
9. **Legal-content audit result**.
10. **Exact next action**, only when something remains incomplete.

Never claim a package ran on real hardware or Linsui unless it was actually tested there.
