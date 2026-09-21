# SFZ timpani / tuba / piccolo LEVEL_GAIN calibration (RESOLVED: rows added)

Date: 2026-09-21. Status: **RESOLVED** with code change (three LEVEL_GAIN rows
added to `instruments.py`). The "deliberately absent" rows existed because the
test-piece pitches made all three measure as silence. Fixed with in-range
fixed-pitch probes and PCM-based RMS.

## The open item (SFZ.md 2026-09-20)

> timpani/tuba/piccolo LEVEL_GAIN absent (deliberate) -- their out-of-range
> note mappings measure as silence, so any gain would over-boost the audible
> notes (comment in instruments.py). Fix the VPO mappings first, then
> calibrate their LEVEL_GAIN rows.

## Why they measured as silence

`test-volume-levels` plays every instrument at the SAME pitches (quiet half
60,62,64,65,67,69; loud half repeats). That is out of range for all three
patches, so every test level was silent:

- `piccolo-SOLO-sustain.sfz`: regions d5(74)..c#6(85) -> 60-69 no region.
- `tuba-SOLO-sustain.sfz`: regions d1(26)..d4(62) -> 62+ partially out, 64-69
  silent.
- `timpani-hit.sfz`: one-shot hits c2(36)..c4(72) -> 60(C4)=edge in, 62+ up
  silent.

Measured as silence, the apparent gain to match the piano is ~+80-130 dB --
the reason the rows were never added (a flat gain would have over-boosted the
actual audible notes).

## Method (in-range fixed-pitch probes)

One probe per instrument at a note that is INSIDE its patch range and a
per-pitch piano reference, so instrument-vs-piano comparison is valid:

| instrument | probe pitch | patch regions used                       |
|-----------|-------------|------------------------------------------|
| piccolo   | 78 (f#5)    | e5..g5 keycenter f#5                     |
| tuba      | 43 (g2)     | f2..g#2 keycenter g2                     |
| timpani   | 50 (d3)     | c#3..d#3 keycenter d3                    |

- CSV: 11 notes, one per named dynamic (30,40,50,60,70,80,90,100,110,120,127),
  0.45 s notes at 0.6 s spacing, velocity=vol, vol constant, pan 64, reverb 40.
- Rendered 3x interleaved (amp_random=1..1.5 on these patches), piano helpers
  single (GM, deterministic).
- Measured with **Python `wave` PCM RMS** (0.33 s window at +0.04 s into each
  note), NOT ffmpeg astats/volumedetect -- both report broken int16 scales
  here (e.g. identical numbers for a file that PCM says is -27 dBFS peak);
  PCM never lies.

## Resulting LEVEL_GAIN rows (instrument-minus-piano dB)

```
piccolo  28:10.7 38:15.2 48:12.0 59:11.1 68:10.5 82:9.3  89:6.5  102:4.9 111:2.0 118:0.8 127:-1.1
tuba     28:17.0 38:16.0 48:11.0 59:8.0  68:5.8  82:3.5  89:1.0  102:-0.2 111:-3.3 118:-4.3 127:-5.8
timpani  28:17.7 38:24.4 48:23.3 59:20.3 68:18.1 82:16.2 89:13.1 102:11.2 111:9.0 118:7.5 127:5.2
```

All within the existing +/-30 dB guard rails. Timpani carries the most flat
gain (hits are transient; needs help to match a sustaining piano reference).

## Validation

Re-rendering the probes WITH the rows applied gives residuals vs the piano:

```
pp...(vol 128-127 skip pppp/ppp only to vol>=50):
  piccolo mean +0.1  max +0.7  min -0.4
  tuba    mean  0.0  max +0.4  min -0.5
  timpani mean  0.0  max +0.4  min -0.5
ppp (vol 40): piccolo +1.0  tuba +1.1  timpani -2.5
pppp (vol 30): piccolo -4.4 tuba -3.1  timpani -9.5   <- known structural
    CC11-pred floor (pppp sits at the vol0=silence end; matches every other
    instrument's pppp behavior, see DEVIN-2026-09-21-sfz-ppp-residuals.md)
```

Real piece: re-rendered b/01 v1-1's timpani stem (12-instrument piece).
Timpani notes (pitches 43/48 G2/C3, vols ~91-102) now peak ~-9 dBFS with no
samples over 0.99 FS; without the row the stem would be ~10 dB quieter in the
mix.

## Remaining limit

Notes above a patch's top region still hit no region and stay silent: e.g.
timpani above c4 (72), piccolo above c#6 (85), tuba above d4 (62). Timpani D4+
never occurs in current scoring. Map the pit's full register if a future piece
needs it (or add KIT_FALLBACK-style key clamping for out-of-range hits).

## Harness

Probe CSVs/renders under `/tmp/calib/{piccolo,tuba,timpani[,piano_78|43|50]}[_r2/_r3]`.
Measurement: `wave` + `array` PCM RMS, 0.33 s window at +0.04 s per 0.45 s note.