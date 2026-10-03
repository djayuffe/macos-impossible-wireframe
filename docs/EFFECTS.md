# Effects and scene catalogue

This demo renders mathematically unusual wireframe geometry with a deterministic
beat timeline. Exact canonical constructions are labelled separately from
sampled or artistic numerical visualizations.

## Canonical / derived scenes

- **600-cell projection** — generated from the standard 120-vertex coordinate
  construction, rotated in 4D and projected into 3D.
- **600-cell face slice** — moving 4D hyperplane intersection using face-aware
  section edges.
- **120-cell dual graph** — derived from the validated 600-cell tetrahedral
  cell incidence.
- **24-cell / 16-cell / tesseract family** — canonical polychora used by the
  geometry core and tests.

## Approximation / visualization scenes

- **Gyroid, Schwarz P, Schwarz D, Neovius, I-WP** — sampled TPMS wire lattices.
- **Hopf fibres** — stereographic-style fibre projection.
- **Boy surface** — analytic immersion grid.
- **Superformula and Clifford torus** — parametric wire surfaces.
- **Poincare-ball-inspired hyperbolic chamber** — visual hyperbolic graph; not
  claimed to be a Coxeter honeycomb.
- **Quaternion Julia boundary lattice** — deterministic escape-boundary sample;
  not claimed to be an exact extracted fractal manifold.
- **Lissajous knot and Lorenz attractor** — motion-led mathematical trails.
- **Discovered surface bank** — deterministic seeded harmonic radial generator.

## Music/timeline

The show uses a BPM clock (`--bpm`, default 132) and can optionally play the
Public Domain `Silicon Dancer` MOD through SDL2/libopenmpt. The timeline emits
beat, bar, beat phase and pulse values; decoded module RMS is folded into the
visual pulse for wire brightness and background color. Geometry generation never
depends on audio callback timing.

## HDR, glimmer and background pass

Wire geometry is rendered into an RGBA16F offscreen framebuffer before a shader
composite pass. The composite adds a procedural nebula/star background, glimmer
streaks, local bloom-like sampling around bright wire pixels, music-reactive
color lift, vignette, grain and exponential tone mapping. This keeps the core
geometry mathematically simple while giving the final image a brighter
high-dynamic-range demoscene finish.
