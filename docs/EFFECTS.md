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
- **v4 exotic family pack** — Mobius strip, Klein bottle, Enneper, helicoid,
  catenoid, Dini, pseudosphere, Roman surface, cross-cap, torus/viviani/
  hypotrochoid curves, Duffing/Rossler/Thomas attractors, Sierpinski
  tetrahedron and eight algebraic implicit visualizations.
- **Unknown-lab procedural objects** — curated supercage, knot-bundle,
  phyllotaxis, ruled and star-cage variants ported from the 49-object lab pack.

## Music/timeline

The show uses a BPM clock (`--bpm`, default 132) and can optionally play the
Public Domain `Silicon Dancer` MOD through SDL2/libopenmpt. The timeline emits
beat, bar, beat phase and pulse values; decoded module RMS is folded into the
visual pulse for wire brightness and background color. Geometry generation never
depends on audio callback timing.

## HDR, glimmer and background pass

Wire geometry is rendered into an RGBA16F offscreen framebuffer before a shader
composite pass. The composite adds a procedural nebula/star background, glimmer
streaks, half-resolution thresholded Gaussian bloom, music-reactive
color lift, vignette, display-space dither and exponential tone mapping. This keeps the core
geometry mathematically simple while giving the final image a brighter
high-dynamic-range demoscene finish.

## Size, zoom and flyover choreography

The renderer now applies a slow breathing scale, music-lifted line width,
animated field-of-view, z-axis zoom and lateral camera flyover. The object stays
centered enough for manual browsing, but the framing constantly changes so large
surfaces feel like fly-through sculptures instead of static turntables.

## Wire lighting matrix

The wire shader now derives color from object-space position instead of using a
flat line tint. Every edge participates in a moving color cycle, so the color
appears to travel through the object as it rotates. Three high-frequency
procedural lattice masks add “lighting matrix” flashes across x/y/z object
coordinates, while three soft light sheets sweep through the mesh from different
directions. Music RMS raises the sheet intensity, line bloom and blue electric
lift without changing the deterministic geometry.

## Shader background optimization

The background remains fully procedural and single-pass, but the fractal noise
octave count is kept low and reused for nebula, aurora and matrix-line layers.
This avoids uploading textures or spawning extra framebuffers while still adding
depth behind the wireframe: sparse stars, diagonal glimmer, aurora bands,
music-reactive matrix streaks, a layered pseudo-3D tunnel, vignette and HDR tone
mapping all happen in the same composite shader.

## Overlay and branding

The branding pass is composited after HDR tone mapping with alpha blending. The
first red UBER logo pulses during the intro. No greets card or alternate logo
is rendered; greetings appear in the scroller text. Source PNGs are kept in
`assets/branding/`; dependency-free RGBA8 derivatives in `assets/overlay/` are
loaded directly by the OpenGL renderer.

Logo edges receive a chromatic split, scanline shimmer, bloom halo and procedural
lightning arcs. The overlay samples the HDR wireframe buffer itself: bright edges
raise the logo flash, lightning energy and blend alpha, while the shared music
pulse supplies the timing. This makes the branding react to what is actually on
screen rather than merely running as a separate animation.

The logo also uses a slow depth illusion: its scale and vertical position move
toward and away from the wireframe, then a smooth envelope fades it out after the
intro so the scene remains uncluttered for the later catalogue.

The overlay vertex pass rotates each card about its own centre in pixel space.
The fragment pass samples the HDR wireframe using screen coordinates to modulate
lightning and glow without coupling the image to arbitrary texture coordinates.

The bottom scroller advances with an accumulated, clamped UV clock rather than
sampling the audio time directly. This prevents visible jumps when a module
decoder corrects its position; its speed is deliberately set for readable
letters, and its presentation height is increased while linear texture filtering
and mipmap texture filtering keep diagonal strokes smooth. Each glyph has its
own crop and advance, with padded cells preventing neighbour bleed. Twelve
subdivisions per glyph carry a vertex sine wave without changing its UV crop
or clipping its top and bottom. The viewport aspect ratio keeps letters proportional.

## GPU wire coverage and depth

Every indexed edge is clipped against the near plane and expanded by a geometry
shader into a screen-space ribbon. Smooth fragment coverage antialiases the two
sides. This provides consistent width even on drivers whose native wide-line
range is limited. Depth testing compares wires with other wires; the meshes do
not contain filled faces and do not provide hidden-surface removal. `D` disables
depth testing for a wire x-ray view.

## Bloom and display conversion

The linear RGBA16F wire buffer feeds two half-size RGBA16F targets. Four
alternating horizontal/vertical passes extract highlights above 1.0 and spread
them with a separable Gaussian. The composite combines scene, bloom and
background, applies exposure and exponential tone mapping, then explicitly
encodes sRGB. Low-amplitude dither is added after encoding to avoid magnifying
noise in dark areas. Branding uses display-space RGBA8 artwork after tone mapping.
