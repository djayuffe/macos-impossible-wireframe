# Optional music

The demo can run from its internal BPM clock and does not require bundled audio.
Use a different tracker module than previous demo music if you want local music while
capturing or presenting the demo.

Suggested local layout:

```text
assets/music/other-module.mod
assets/music/other-module.s3m
assets/music/other-module.xm
```

Music files are ignored by Git. Pass timing with `--bpm`; the renderer and
geometry are deterministic and do not depend on an audio callback.
