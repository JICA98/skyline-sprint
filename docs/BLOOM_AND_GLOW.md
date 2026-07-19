# Neon Glow & Bloom Notes — Skyline Sprint

## Current Implementation (Additive Blending)

Because the PS4 target uses the OpenOrbis SDL2 **software renderer**, custom pixel shaders are not available. Instead we approximate neon glow with **SDL_BLENDMODE_ADD**:

1. **Particles** (jump trails, landing dust, pickup bursts)  
   - Two-pass additive fill: large soft outer halo + bright core.  
   - Colors already use the cyan / yellow / gray palette.

2. **Player “Pulse”**  
   - `Player::RenderGlow()` draws three concentric additive rects under the sprite before the normal textured draw.  
   - Produces a soft cyan aura that reads as neon against the dark sky.

3. **Energy shards**  
   - Additive yellow under-glow drawn in `World::Render` immediately before the shard texture.

4. **Enemy drones**  
   - Additive cyan thruster glow under the body.

These passes are cheap (a few `SDL_RenderFillRect` calls) and work identically on the software renderer and on desktop accelerated backends.

Platform neon bands are already baked into the platform texture as high-intensity cyan pixels; they do not need an extra additive pass.

## Exploring True Shader-Based Bloom

A classic multi-pass bloom pipeline would look like:

```
Scene → Offscreen color target
     → Bright-pass extract (threshold + soft knee)
     → Downsample → Separable Gaussian blur (horizontal then vertical)
     → Upsample / combine (optionally multi-scale / Kawase)
     → Additive composite back onto the original scene
```

### Why it is not implemented yet

- **PS4 / OpenOrbis path**: the current homebrew uses a pure software `SDL_Renderer` + `SDL_BlitScaled` into VideoOut. There is no Gnm, no fragment shaders, and no easy access to an offscreen floating-point target that can be blurred efficiently.
- **Desktop**: SDL2’s accelerated renderer does not expose a convenient custom-shader API. Achieving bloom would require dropping to raw OpenGL / Metal / Vulkan (or using a higher-level engine), which would diverge from the single codebase that must also compile for the PS4 software path.
- **Performance budget**: even a ½-resolution two-pass Gaussian is expensive in software; a full 1080p bloom would push the emulator and low-end hosts out of 60 fps.

### Recommended future path

If a hardware-accelerated PS4 renderer (or a desktop-only advanced mode) is added later:

1. Render the whole scene (platforms, player, particles, UI) into an RGBA16F (or RGBA8) render target.
2. Extract pixels brighter than ~0.7 into a quarter-resolution buffer.
3. Apply 2–3 Kawase or dual-filter blur passes.
4. Composite with a controllable intensity (settings option “Bloom Strength”).
5. Keep the existing additive particle/glow path as a cheap fallback when the advanced renderer is not present.

Until then the additive neon technique already gives the game a strong glowing cyberpunk feel without sacrificing the PS4 software-renderer compatibility goal.
