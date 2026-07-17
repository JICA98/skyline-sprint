# UI/UX Design — Skyline Sprint

## 1. Design Goals

Skyline Sprint must look intentional while remaining technically conservative enough for an early PS4 homebrew toolchain and emulator.

Priorities:

1. Instant readability at television distance.
2. Clear controller-first navigation.
3. Original visual identity.
4. Low asset and renderer complexity.
5. Useful compatibility diagnostics.
6. No visual similarity to Mario or another commercial platform franchise.
7. Stable 16:9 layout with safe margins.
8. Minimal text during normal play.

The visual experience should feel like a compact arcade homebrew test title, not a generic engineering sample.

## 2. Visual Identity

### 2.1 Theme

The game takes place across an abstract twilight skyline made of modular rooftops and floating infrastructure.

The player character, **Pulse**, is a small rounded courier shape with:

- a dark central body;
- a bright visor line;
- two short legs;
- a subtle trailing glow represented with simple particles;
- no hat, moustache, overalls, familiar silhouette, or resemblance to a commercial mascot.

World elements use geometric silhouettes:

- rooftops: long slabs and stepped towers;
- safe platforms: solid top edge plus darker body;
- jump pads: chevron pattern;
- energy shards: small rotated diamonds;
- drones: original hexagonal or triangular machines;
- hazards: warning-striped barriers and gaps;
- distant city: simple parallax rectangles and antenna lines.

### 2.2 Palette

Use a restrained palette and centralize it in one theme file.

Suggested starting values:

| Role | Hex |
|---|---:|
| Deep sky | `#111827` |
| Far skyline | `#1F2937` |
| Near skyline | `#374151` |
| Platform body | `#334155` |
| Platform edge | `#94A3B8` |
| Primary accent | `#22D3EE` |
| Secondary accent | `#A78BFA` |
| Pickup | `#FACC15` |
| Success | `#4ADE80` |
| Warning | `#FB923C` |
| Failure | `#FB7185` |
| Main text | `#F8FAFC` |
| Muted text | `#CBD5E1` |

The agent may adjust contrast after testing, but must keep essential game objects distinguishable in grayscale and avoid rapid full-screen flashing.

### 2.3 Motion language

- Player motion should be smooth and responsive.
- Background parallax should be slow and subtle.
- Pickups may pulse by scaling or brightness, not by aggressive flashing.
- Failure should use a short freeze, small screen shake, and fade.
- Menus should use simple opacity/position transitions under 250 ms.
- Stress mode can add more particles, but particle brightness and lifetime must remain bounded.
- Respect a reduced-effects option by disabling screen shake and lowering particle count.

## 3. Display and Safe Area

Logical design resolution:

```text
1920 × 1080, 16:9
```

Requirements:

- Scale the logical scene to the actual output while preserving aspect ratio.
- Use letterboxing if required rather than stretching.
- Keep critical UI within a 96 px horizontal and 54 px vertical margin.
- Avoid placing text at the extreme screen edges.
- Minimum normal UI text height at logical resolution: approximately 32 px.
- Menu title height: approximately 72–96 px.
- Diagnostics body text may be smaller, approximately 24–28 px, but must remain readable.
- All screens must remain usable if slight overscan crops the outer 3–5%.

## 4. Navigation Model

The application has these top-level states:

```text
Boot
  ├── Fatal Error
  └── Title
        ├── Play
        ├── Test Modes
        ├── Settings
        ├── Diagnostics
        └── Credits
             ↓
          Gameplay
        ├── Pause
        ├── Failure
        └── Diagnostics Overlay
```

Navigation rules:

- One clearly highlighted selection.
- D-pad and left stick navigate.
- Cross confirms.
- Circle returns.
- Options pauses during gameplay.
- The currently expected action appears in a small footer.
- Input repeat must have a delay and controlled repeat rate.
- Accidental analog drift must not race through menu items.
- No mouse is required.
- Keyboard support is allowed for desktop development.

## 5. Boot Flow

### 5.1 Startup screen

Target duration: under two seconds after renderer initialization.

Display:

- project logo mark;
- `SKYLINE SPRINT`;
- build identifier;
- a small `Open-source PS4 homebrew compatibility test` subtitle.

Do not show a long unskippable splash.

During initialization:

- log each subsystem;
- if audio fails, continue silently;
- if controller is absent, show a nonblocking notice;
- if video or required assets fail, show the fatal-error screen.

### 5.2 Fatal-error screen

Visual structure:

```text
SKYLINE SPRINT COULD NOT START

Subsystem: <name>
Result: <numeric/hex result>
Message: <short explanation>

Build: <build id>
Toolchain: <release/commit>
Log path: <path or "memory only">

[Cross] Retry     [Circle] Exit
```

Only offer Retry if reinitialization is safe.

## 6. Title Screen

### 6.1 Layout

Left third:

- logo;
- build label;
- menu items.

Right two-thirds:

- a live, noninteractive skyline scene;
- Pulse running on a short looping rooftop;
- current controller status as a small icon/status;
- seed-of-the-day preview is optional but must not use network time.

Menu:

1. Play
2. Test Modes
3. Settings
4. Diagnostics
5. Credits

Footer:

```text
D-pad/Left Stick: Move   Cross: Select   Circle: Back
```

### 6.2 First launch behavior

No mandatory tutorial dialog.

When Play is selected for the first time, show a brief control strip for approximately five seconds:

```text
Move            Jump            Pause
D-pad/Stick     Cross           Options
```

The strip fades once the player jumps.

## 7. Gameplay HUD

Keep normal HUD minimal.

Top-left:

- distance in metres;
- score;
- multiplier.

Top-right:

- energy-shard count;
- compact mode badges only when active:
  - `SEED`;
  - `AUTO`;
  - `STRESS`;
  - `MUTE`.

Bottom-left, only during the first run:

- contextual jump prompt.

Bottom-right:

- optional compact frame-time indicator when diagnostics overlay is enabled.

Do not display health bars; a fall or direct hazard contact ends the run.

### 7.1 Score presentation

- Distance should remain the dominant metric.
- Pickups add score.
- Consecutive pickups increase a capped multiplier.
- Missing a pickup or waiting too long gently decays the multiplier.
- High score may remain session-only unless a verified writable save path is implemented.
- Never make save-data support a blocker for the first release.

## 8. Player Feedback

### 8.1 Jump

On jump:

- short squash/stretch limited to geometric deformation;
- 4–8 small trailing particles;
- short original sound;
- subtle controller vibration only if the exact API is verified and optional.

Coyote time and buffered input should make the control feel forgiving without changing visible UI.

### 8.2 Landing

- one small dust/energy burst;
- short landing sound when falling above a threshold;
- camera impulse only for a strong landing and only when effects are enabled.

### 8.3 Pickup

- shard collapses toward the HUD or bursts into particles;
- score text briefly rises;
- sound pitch can vary slightly if supported;
- no full-screen flash.

### 8.4 Failure

Sequence:

1. freeze simulation for approximately 80 ms;
2. apply a small camera shake;
3. fade gameplay to 55% brightness;
4. show failure panel.

Failure panel:

```text
RUN ENDED

Distance      1,248 m
Score         18,500
Best          1,410 m
Seed          7A3F91C2
Frame avg     16.8 ms

Cross: Retry
Triangle: Replay Seed
Circle: Title
R3: Diagnostics
```

Retry should be selected by default and work quickly.

## 9. Pause Screen

Overlay rather than separate full scene.

Menu:

1. Resume
2. Restart Run
3. Diagnostics
4. Settings
5. Return to Title

Background:

- gameplay remains visible but darkened;
- simulation is stopped;
- diagnostics counters should clearly indicate paused state;
- audio is paused or attenuated.

Prevent the Options button that opened the pause menu from immediately closing it in the same frame.

## 10. Test Modes Screen

This screen is a first-class product feature.

Rows:

### 10.1 Deterministic Seed

- Toggle fixed seed.
- Display editable hexadecimal seed using controller-friendly digit controls.
- Provide `Randomize` using a host time source only for choosing a seed; once selected, generation remains deterministic.
- Provide several built-in known seeds:
  - flat/easy;
  - gap-heavy;
  - obstacle-heavy;
  - vertical-variation;
  - long-run regression.

### 10.2 Automatic Test

When enabled:

- game starts automatically;
- input is generated by a deterministic simple controller;
- failure triggers a delayed restart;
- run number and aggregate stats are displayed;
- user input can cancel at any time.

The automatic controller does not need to master every random level. It may use a dedicated validated test pattern or a known seed.

### 10.3 Stress Mode

Stress mode increases:

- background decorative objects;
- particles;
- active non-colliding drones;
- HUD graph sampling;
- optional extra audio voices.

It must not create unbounded objects or alter the deterministic collision route.

### 10.4 Input Visualizer

Display:

- D-pad state;
- face buttons;
- shoulder buttons;
- Options;
- stick axes;
- trigger values if exposed;
- connection status;
- controller index.

Use the actual verified input values and label unsupported controls as unavailable.

## 11. Diagnostics

### 11.1 Compact overlay

Toggled with L3.

Display in a translucent panel:

```text
FPS 59.8 | 16.7 ms
Tick 0038214 | Seed 7A3F91C2
Objects 184 | Chunks 7 | Particles 22
Pad OK | Audio OK | AUTO OFF | STRESS OFF
```

The compact overlay must not allocate strings continuously. Reuse buffers.

### 11.2 Full diagnostics screen

Opened from title, pause, failure, or R3.

Pages:

1. **Runtime**
   - build;
   - Git commit;
   - OpenOrbis release/commit;
   - platform;
   - video dimensions;
   - renderer name if available;
   - uptime;
   - controller;
   - audio;
   - writable log path.

2. **Performance**
   - current FPS;
   - average/min/max frame time;
   - fixed updates this frame;
   - frame-time histogram or small scrolling graph;
   - rendered objects;
   - particles;
   - chunks;
   - memory counters available to the application.

3. **Input**
   - visualizer;
   - raw verified values;
   - connect/disconnect events.

4. **World**
   - seed;
   - chunk index;
   - origin-rebase count;
   - difficulty tier;
   - generator fallback count;
   - active platform/hazard/pickup counts.

5. **Log**
   - last 128 in-memory messages;
   - severity filter;
   - newest/oldest navigation;
   - clear action;
   - no uncontrolled text scrolling.

6. **Licence**
   - project licence;
   - third-party acknowledgements;
   - explicit no-commercial-assets statement.

Diagnostics navigation:

- L1/R1 changes page.
- D-pad scrolls.
- Triangle resets rolling performance statistics.
- Square toggles stress mode.
- Circle returns.

## 12. Settings

Keep settings small and local.

Settings:

- Master audio: On/Off.
- Sound effects volume: 0–100%, if volume control is supported.
- Screen shake: On/Off.
- Particle level: Low/Normal.
- Diagnostics overlay: Off/On.
- Coyote-time assist: On/Off only if exposing this does not complicate deterministic tests.
- Reset session best.

Persistence is optional for the first release. If settings are not persisted, label them `This session`.

## 13. Credits and Licence Screen

Include:

- project name and version;
- contributors;
- OpenOrbis acknowledgement;
- SDL/OpenOrbis SDL port acknowledgement as required;
- licences for all third-party components and assets;
- a clear statement that Skyline Sprint is not affiliated with Sony, Nintendo, or any commercial game publisher;
- a clear statement that no retail game data is included.

Avoid using third-party logos unless their licence and branding rules permit it.

## 14. Accessibility and Comfort

Required:

- high-contrast text;
- UI information conveyed by labels as well as colour;
- reduced-effects option;
- no rapid flashing;
- controller disconnection prompt;
- generous input timing through coyote time and jump buffering;
- pause available during normal gameplay;
- readable failure seed and error codes;
- no essential audio-only cue.

Recommended:

- alternative pickup colour with stronger luminance contrast if testing shows ambiguity;
- lower particle setting;
- simple icons paired with text.

## 15. Asset Plan

Prefer generated, original assets.

Required generated assets:

- title icon/logo;
- Pulse sprite states or geometric construction;
- platform tile textures;
- energy shard;
- drone;
- warning stripe;
- package icon;
- package splash/background;
- small WAV effects;
- optional ambient loop.

The source for every generated asset must be retained:

```text
assets/source/
scripts/generate_assets.py
```

Generation must be deterministic and documented. Output assets should be committed so a PS4 build does not require image libraries that are unrelated to the target toolchain.

Do not generate near-copies of famous platform-game assets.

## 16. Interaction Acceptance Criteria

The UI/UX is complete when:

- Every screen is navigable using only a standard PS4 controller.
- Menu focus is always visible.
- Back behavior is consistent.
- The title screen reaches gameplay in two confirmations or fewer.
- Retry after failure requires one confirmation.
- Controller disconnect does not crash the game.
- Diagnostics are reachable before and during gameplay.
- The build ID and seed are visible without reading an external log.
- Normal gameplay remains uncluttered.
- UI remains inside the safe area.
- All assets are original or properly licensed.
- No screen or character can reasonably be mistaken for Mario or another commercial platform property.
