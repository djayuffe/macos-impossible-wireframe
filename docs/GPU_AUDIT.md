# GPU renderer audit — 2026-10-05

Scope: the executable, scene-to-GPU updates, OpenGL resource lifecycle, all
rendering shaders, branding/scroller pass, controls and the music transport that
drives animation. This is not a proof of every mathematical geometry generator.

![Actual GPU framebuffer capture](gpu-audit.png)

## Enabled pipeline

```text
CPU scene cache --revision--> streamed vertex/index buffers
                                      |
                            vertex transform (MVP)
                                      |
                            geometry shader
                         near clip + ribbon width
                                      |
                          wire fragment shading
                         coverage + depth testing
                                      |
                            RGBA16F + depth24
                              |           |
                              |     half-size HDR target A
                              |       threshold + blur H
                              |           |
                              |     target B: blur V
                              |           |
                              |     target A: blur H
                              |           |
                              |     target B: blur V
                              +-----------+
                                      |
                        procedural background + bloom
                       exposure -> tone map -> sRGB
                                      |
                       first logo / sine text ribbon
                         alpha composite -> display
```

Geometry construction and 4D projection remain CPU work with rate-limited scene
caching. Model/view/projection, wire expansion, shading, background, bloom and
overlay effects run on the GPU. No unused GPU feature is claimed to be enabled:
there are no compute shaders, tessellated filled surfaces, ray tracing, surface
normals, shadow maps or native Metal backend in this project. It targets Apple's
[OpenGL 4.1 profile](https://developer.apple.com/documentation/appkit/opengl-profiles).

## Findings fixed

| Defect | Change |
| --- | --- |
| Mesh pointer/count/radius could stay constant while vertex data changed | Explicit cache revision drives GPU upload |
| Seed omitted from scene cache key | Seed changes rebuild the cached mesh |
| Native wide lines depended on driver limits | Geometry-shader ribbons with pixel coverage |
| Perspective division could happen before clipping | Clip near-plane crossings before division |
| Reverse-edge `smoothstep` produced undefined GLSL results | Ordered edges and explicit inversion |
| Bloom was a small neighbourhood approximation | Dedicated thresholded separable HDR blur |
| Light boost saturated colors and dark noise was amplified | Removed nonlinear highlight boost; sRGB encoding and display-space dither |
| Overlay sampled scene using artwork UVs | Sample HDR with screen coordinates |
| Logo rotated around screen origin | Pixel-space rotation about card centre |
| Glow outlived artwork fades | Fade applies to the complete overlay alpha |
| Fixed-cell font crops clipped glyphs and included adjacent strokes | Native-resolution individual crops, transparent gutters and per-glyph advances |
| Scroller UV distortion clipped lettering | Subdivided glyph ribbons displaced by the vertex shader |
| Failed framebuffer allocation could cache invalid dimensions | Commit dimensions only after all targets pass checks |
| Reinitialization retained old allocation sizes | Reset all capacities, dimensions and animation state |
| Texture loader could allocate from an unchecked file header | Check actual file length, little-endian header, GPU limit and 256 MiB cap |
| Resource paths depended on launch directory | Locate assets beside the executable when absent in cwd |
| Shader-only edits were not copied until relink | Resource synchronization runs on every build |
| Left from first scene fell into automatic mode | Explicit modular navigation |
| End-of-module could stall animation time | Repeating module and continuous sample-count transport |
| Runtime render failures returned success | Nonzero exit status and stage-specific GL errors |

Only the first UBER logo remains active. Alternate-logo/greets textures are not
loaded or drawn. The message is editable in `assets/overlay/scroller.txt`.

GLSL's ordering requirement is defined in the
[Khronos smoothstep reference](https://github.com/KhronosGroup/OpenGL-Refpages/blob/main/gl4/smoothstep.xml).

## Validation and limits

Validated on Apple M1 Pro, `4.1 Metal - 91.7`, maximum target dimension 16384,
using the installed Xcode SDK and warnings-as-errors. All 51 scenes produced
valid GPU readbacks. Feature comparisons verified bloom, width, depth,
background, overlays and exposure. Tests cover window resizing, empty/rejected
meshes, non-finite render inputs, near-plane crossings and repeated shutdown/init.

The render targets require approximately 16 bytes per full-resolution pixel
(8-byte RGBA16F scene, approximately 4-byte depth, two half-size HDR textures),
excluding driver overhead, default framebuffer, artwork and geometry buffers.
Bloom off skips its four draw passes. Background and overlay toggles skip their
associated work. Buffers are orphaned on update to avoid waiting for in-flight
geometry; unchanged cached meshes do not upload again.

The GPU tests use readback synchronization for validation, not benchmarking.
No universal frame-rate improvement or cross-driver guarantee is claimed.
Linux compilation paths were corrected, but Linux GPU execution was not run.
Wire depth is not solid-surface occlusion because source meshes contain edges
only. Display output is tone-mapped SDR, not an HDR-monitor presentation mode.
