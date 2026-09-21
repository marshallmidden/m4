# SFZ pizzicato high-velocity "inversion" (RESOLVED: measurement artifact)

Date: 2026-09-21. Status: **RESOLVED — NOT A BUG.** Investigation recorded here
as the permanent record; NO code change to gcs2sfz/instruments.py.

## The open item (SFZ.md 2026-09-20)

> pizzicato high-velocity inversion (saw it again 2026-09-20: single-pitch
> fff=-35.1 vs ffff=-37.7 at identity velocities 110/120) -- same VPO
> inverted-layer-volume quirk as the cello, needs a cello-style layer-balance
> fix, not LEVEL_VELOCITY remapping.

## What the measurements show (this session)

Single-note measurements at velocity 110 vs 120 give RUBBISH because every
pizzicato note is hit with `amp_random=1.5` (group opcode, all four sections of
`all-strings-SEC-pizzicato.sfz`) -- +/-1.5 dB of per-note random amplitude
swing. One-shot probes flip sign randomly around the true curve. Repeat-averaged
(3-4 interleaved renders of each point):

1. **Raw patch, fixed CC11=127** (vol 127 removes all expression + LEVEL_GAIN
   residual variation): v110 -> v120 is MONOTONIC at every pitch tested
   (36, 43, 48, 52, 56, 60, 64, 67, 72, 79, 84, 85, 90, 91, 95, 100, 105):
   +1.1 .. +2.2 dB louder at 120.

2. **Full pipeline** (predistorted CC11 at vol 110/120 + LEVEL_GAIN as shipped):
   fff(vel110,vol110) -> ffff(vel120,vol120) MONOTONIC at every pitch
   (67, 72, 79, 84, 56, 60): +0.5 .. +1.5 dB. The LEVEL_GAIN row's top-end
   wobble (111:8.6 dB, 118:4.8 dB, 127:5.5 dB) pulls ffff back ~3.8 dB in CC11
   residual, but the velocity gain still clears it -- margin is SMALL (~0.5 dB
   at some pitches), so single unlucky-note draws can still look inverted.

3. **The reported -35.1/-37.7 pair is the finding-4 confound.** `test-volume-
   levels` plays a DIFFERENT pitch per level (fff on 3f8, ffff on 3g8), and the
   patch's higher-register pizz samples are naturally ~2-3 dB quieter (raw,
   vol=127: p100=-36.6, p105=-39.9). Rendering the actual piece CSV here:
   fff=-25.9, ffff=-24.4 -> monotonic.

4. **Real-piece dynamic range** (pitch 67, CC11=127, v50..v80 step, repeat-
   averaged): strictly monotonic every step (-49.6 -> -40.9 dB). No boundary
   dip at the 54/62 crossfade either -- the earlier single-shot "dips" at v62/
   v70/v105 were amp_random noise, not layers.

Conclusion: there is no deterministic high-velocity inversion in the VPO pizz
patch or in the gcs2sfz pipeline; the open item was a single-note measurement
artifact (amp_random=1.5) compounded by test-volume-levels' per-level pitch
changes. Nothing to "fix" -- mirror the LEVEL_VELOCITY finding 4 resolution
(do not remap for a measurement artifact).

## Residual follow-up (optional, NOT blocking)

The pizz LEVEL_GAIN top end is non-monotonic-smoothed (111:8.6, 118:4.8,
127:5.5). fffff is calibrated off base_gain_db (127)=5.5; the 4.8 at 118 is
likely the same multi-pitch confound. It does NOT invert the pipeline on
average (see 2), but a future single-pitch re-measure of the top 3 levels would
glide it: expected ~8.6(111) -> ~7(118) -> ~5.5(127). Leave alone unless a
re-render shows a real fff/ffff audible seam.

## Measurement harness used (bake into /tmp if needed again)

- CSV schema: `start,dur,midi_note,velocity,pan,reverb,vol_start,vol_end,
  pan_end,articulation,staff` (11 cols; staff empty -> single stream).
- name the CSV `pizzicato_strings.csv` (else -> GM fallback).
- 2.8 s spacing, 1.8 s notes, 1.3 s astats window starting 0.15 s into each.
- run `./gcs2sfz <dir> --sfzdir ./library/VPO --outdir <out>` FROM `music/sfz/`.
- ALWAYS repeat each point 3-4x interleaved; never trust a single note (amp_random).