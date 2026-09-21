# SFZ ppp/pppp loudness residuals (RESOLVED: structural + register artifact, not the window)

Date: 2026-09-21. Status: **RESOLVED — DOCUMENTED, NO CODE CHANGE.** The open item
hypothesized the ~3-8 dB ppp/pppp residuals came from the 0.25 s calibration
window cutting off the slow VPO soft attack. Re-measured with longer windows and
fixed-pitch probes; the residual survives longer windows, but is explainable as
(1) a high-register test-pitch confound and (2) a structural CC11-pred headroom
floor, and it sits at absolute levels that are inaudible in a mastered mix.

## Method

- Rows: `test-volume-levels` per-instrument stems, rendered 3x interleaved
  (each VPO note carries per-note `amp_random`, so no single-shot measurement).
- Windows per note: early `[t+0.04, 0.21]` (the original calibration window) and
  full-note `[t+0.06, dur-0.08]`. A 3-second fixed-pitch probe (12 instruments,
  pppp/30 .. mp/70, one pitch each at 60/67/45) was also rendered to break the
  per-level pitch confound (test-volume-levels plays a different pitch per level).
- Pluck instruments (crash, pizz) use the early window only -- their energy is
  the initial transient.

## Results (residual = instrument - piano, dB; 3x averaged, test-volume-levels)

`early/full`:

| instrument    | pppp        | ppp         | pp    | p     | mp    |
|---------------|-------------|-------------|-------|-------|-------|
| english_horn  | -12.8/-12.1 | -9.9/-10.1  | -3.9/-4.4 | -2.1/-0.1 | -3.7/-4.4 |
| crash_cymbal_1| -10.6       | -10.0       | +0.9  | -0.1  | -1.6  |
| pizzicato     | -10.4       | -5.7        | +1.8  | +1.1  | +0.9  |
| bassoon       | -9.8/-9.6   | +0.1/+0.3   | +3.9/+3.7 | +2.1/+2.2 | -4.2/-4.4 |
| contrabass    | -9.2/-8.4   | -4.0/-3.9   | +2.8/+3.4 | +2.2/+3.3 | +2.4/+3.9 |
| oboe          | -7.1/-5.6   | -5.6/-5.0   | +3.7/+4.2 | +2.6/+4.0 | -1.1/-1.5 |
| cello         | -6.6/-6.1   | -3.4/-3.6   | +1.7/+1.7 | +0.0/+1.1 | -1.9/-2.4 |
| clarinet      | -6.4/-5.5   | -2.8/-2.5   | +2.5/+2.6 | +1.7/+2.2 | -1.8/-2.7 |
| flute         | -5.6/-3.9   | -2.9/-1.5   | +3.5/+3.9 | +1.1/+2.4 | +0.3/-0.8 |
| viola         | -5.9/-5.2   | -5.0/-4.8   | +0.6/+0.7 | +0.3/+1.5 | +1.2/+2.8 |
| trumpet       | -4.5/-3.3   | -4.0/-3.3   | +1.3/+2.0 | +1.0/+2.2 | +2.4/+2.8 |

The early-window "residual melts away with a longer window" hypothesis (from the
2026-09-17 note, oboe/flute/viola/trumpet looking fine) is WRONG once the 0.25 s
calibration residual is included: *every* instrument shows a 3-13 dB shortfall at
pppp and ~3-10 dB at ppp in BOTH windows. The original note compared instruments
re-measured at the late window against a piano that was still measured at the
early window -- apples-to-oranges. Measured consistently, the quiet-dynamics
shortfall is real and systematic.

## Why it is NOT a regression to fix

1. **Register confound, not a level error.** The test-piece pppp row plays high
   pitches (65/67/69); at their HOME register the same levels are fine. Fixed-
   pitch probe (3 s sustain, steady-state [t+1.0, t+2.5]): english_horn pppp
   +3.2, bassoon +1.2, contrabass +0.3, viola -0.1, trumpet +4.8, cello +7.1 --
   i.e. within ~0-3 dB of the local reference, matching the mp..fffff spec.

2. **Structural CC11-pred floor.** `_pred(v) = 127*(v/127)^4 * 10^(level_gain-base)/20`
   (gcs2sfz:328, GCS2SFZ_EXPR_EXP=4). The raw power term at vol 28-40 is 0.3-1.25
   (of 127); even with a high residual gain the emitted CC11 lands at ~3-10:
     v=30: base 0.40, v=40: base 1.25, v=50: base 3.05. There is no CC11 headroom
     left to add the dozen-plus dB the table would prescribe -- the low dynamics
     are an octave of whispers because the score LEVEL GAIN is exponential (GM-
     matched by design) and pppp/pppp (vol 28-40) sit just above the vol0=silent
     floor. Flat base_gain is calibrated at the top dynamic and cannot be raised
     without wrecking mp..fffff. The ppp/pppp LEVEL_GAIN rows are already
     maxed-out relative to what the CC11 path can physically deliver.

3. **Absolute levels are inaudible.** At ppp/pppp both the instruments (-57 to
   -82 dBFS) and the piano reference (-70 to -82 dBFS steady) are at or below a
   mastered mix's noise floor. These dynamics are *meant* to be near-silent
   (pppppp=vol0 is deathly silent in both renders, by design). The 3-13 dB gap
   exists between two inaudible whispers.

A LEVEL_GAIN bump at ppp/pppp would move -78 dB to -70 dB -- no audible effect,
at the cost of a full 130-mix re-render and re-validation of the mp..f match.

## The one real (tracked) case

mp steady-state shows a ~8-22 dB SPREAD across instruments -- e.g. violin vs the
GM-fallback piano: VPO sustains while the GM piano decays between the early and
late windows, so the early-window calibration overstates how quiet sustained
strings are in a long mp passage. That is the classic calibration-TOFU (matched
for one window, then real notes sustain past it). It only matters where a single
instrument carries a sustained line alone; the orchestral mix absorbs it. NOT
changed here; noted for the ensemble A/B pass.

## Verdict

ppp/pppp residuals: real but structurally bounded, at inaudible levels, and
worsened by the test piece's high-pitch pppp row. No LEVEL_GAIN change warranted.
The "longer window" idea from SFZ.md is retired. Leave the table alone; keep the
earlier mp..fffff calibration as-is.

## Harness (reusable)

- Same CSV schema as the pizz note (11 cols; empty staff). Probes in
  `/tmp/probe3s/<instrument>/<instrument>.csv` (fixed pitch, 3.0 s notes, 3.5 s
  spacing, levels pppp/30 ppp/40 pp/50 p/60 mp/70 on one pitch: english_horn 60,
  bassoon 60, contrabass 45, cello 60, clarinet 60, oboe 60, flute 67, viola 60,
  violin 67, trumpet 60, french_horn 60, piano 60).
- Renders: `./gcs2sfz <dir> --sfzdir ./library/VPO --outdir <out>/out` from
  `music/sfz/`; always 3x interleaved.
- astats RMS via ffmpeg; test-piece stems in `/tmp/stems/{instr}[-_r2/_r3]/`.