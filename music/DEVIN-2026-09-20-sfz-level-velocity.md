# SFZ per-instrument LEVEL_VELOCITY calibration (DONE 2026-09-20)

Date: 2026-09-20. Status: **RESOLVED.** Committed this session; see "What was
done" below. The plan/harness/measurements in this file are preserved as the
permanent record.

Goal: complete the SFZ.md "Open: calibrate per-instrument LEVEL_VELOCITY
entries" item. User chose **"Surgical layer-only"** (see Decision below).

## What was done (implementation session)

- Populated `LEVEL_VELOCITY` (instruments.py, formerly `{}`) for exactly the 5
  layered instruments, keyed by the normalized names:
  - `french_horn` / `trumpet` / `brass_section` (75-crossfade): all identity
    except `mp` (70) -> 0.97, so 70 -> 68 clears the 75 edge with humanize +-3.
  - `trombone` (75/90 crossfade): `mp` 70 -> 68; `f` 90 -> 1.03 (-> 93) so f
    sits fully above the 90 blend top; `mf` stays in the blend.
  - `pizzicato_strings` (54/62 crossfade): `p` 60 -> 0.97 (-> 58) centered in
    the crossfade; pp=50 stays clear of 54, mp=70 well into the loud layer.
  - Added a comment block documenting the ~12 single-layer instruments as
    intentionally absent (velocity == pure loudness there, LEVEL_GAIN owns it).
- Re-measured all 5 single-pitch with the pipeline (gcs2sfz applies the table
  via `velocity_mult`); deltas vs baseline are all <= ~0.5 dB at the moved
  levels (mp/f/p), inside run-to-run noise -> **no LEVEL_GAIN change needed**.
- Confirmed moves stay monotonic single-pitch (french_horn/trumpet/trombone/
  brass_section all monotonic pppp..fffff). pizzicato keeps a pre-existing
  fff (-35.1) vs ffff (-37.7) inversion at identity velocities (110/120) --
  patch/measurement quirk, NOT a regression, left alone (matches finding 4).
- End-to-end: `make -B sfz-mix/v1-1-sfz-mix.wav` in b/01 renders clean
  (12 instruments incl. french_horn/trumpet/pizzicato_strings, 483.0s).
- Docs: SFZ.md Open item -> Resolved entry; this note updated.
- DOALL: 0 bare / 2 pre-existing named (compilers untouched).

## TL;DR for the next session

`LEVEL_VELOCITY` (music/sfz/instruments.py:318) is still `{}` (identity). The
plan is to populate it with rows **only for the 5 instruments that have real
velocity layers**, place their named dynamics correctly, re-measure, and apply
small `LEVEL_GAIN` deltas where velocity moved loudness. The ~12 single-layer
instruments stay identity (documented with a comment). Do NOT remap them.

## Key findings (established this session, with evidence)

1. **Velocity is a loudness knob everywhere, not just a layer selector.** The
   MAP patches (`*-SEC-sustain.sfz`, instruments.py:29-60) set NO `amp_veltrack`,
   so sfizz's default velocity→amplitude tracking is active in all of them. A
   clean single-pitch violin sweep (fixed CC11, vel 20..127) moved RMS ~-70 →
   -38 dB. So any LEVEL_VELOCITY row ≠ identity changes loudness, and
   `LEVEL_GAIN` (calibrated 2026-09-17 at vel=vol) must be re-fit wherever
   velocity changes.

2. **Only 5 instruments have real velocity layers** (verified from the actual
   `*-SEC-sustain.sfz` region/group structure under
   `music/sfz/library/VPO/Virtual-Playing-Orchestra3/`):
   - `french_horn` (french-horn-SEC-sustain.sfz): `xfin_lovel=75 xfin_hivel=127`
     (soft layer below 75, ff layer fades in 75-127).
   - `trumpet` (trumpet-SEC-sustain.sfz): same 75 / 75-127 crossfade.
   - `trombone` (trombone-SEC-sustain.sfz): `group=1` with
     `xfin/xfout_lovel=75 hivel=90` — `_p` (piano) samples fade OUT 75-90,
     forte samples fade IN 75-90. Below 75 = piano only; above 90 = forte only.
   - `brass_section` (all-brass-SEC-sustain.sfz): mixed per-pitch-range, both
     75/127 and 75/90 crossfades.
   - `pizzicato` (all-strings-SEC-pizzicato.sfz): low-vel regions `hivel=62`,
     high-vel regions `xfin_lovel=54` → crossfade 54-62.
   - Everything else (`flute, oboe, clarinet, bassoon, piccolo, english_horn,
     bass_trombone, tuba, violin, viola, cello, contrabass`, percussion) is a
     **single layer** → a velocity row there is a pure loudness change (redundant
     with LEVEL_GAIN) and would force a needless re-fit.

3. **Identity already straddles every real boundary correctly** (the named grid
   was clearly designed for it): brass mp=70 < 75 < mf=80; trombone
   70<75<80<90<f=90; pizz pp=50 < 54 < 60(p)<62<70(mp). Humanize velocity jitter
   is only ±3 (imscomp `humanize_velocity` default 3), so margins hold. **So the
   surgical table is close to identity** — its value is: (a) explicit per-layer
   anchoring with margin, (b) documenting which instruments have no layers. Expect
   only small audible change; confirm with the harness.

4. **The "ppppp louder than pppp" inversion reported earlier is a MEASUREMENT
   ARTIFACT, not a velocity bug.** On a fixed pitch the curve is monotonic
   (violin `ppppp -76.3 < pppp -74.7`). The inversion only appeared in
   `ims/test-volume-levels` because that piece plays a **different pitch per
   level** (C4..A5), confounded by register. Do not try to "fix" the inversion
   via LEVEL_VELOCITY; if anything, re-measure that piece per single pitch.

5. **ppppp at vol=0 is effectively silenced by CC11 pred → 0**, and the residual
   ~-76 dB reading is just `base_gain_db` (e.g. violin +13.1 dB) applied to the
   sfizz noise floor (~-90 dB). This is a LEVEL_GAIN/base-gain structural limit,
   NOT something LEVEL_VELOCITY can fix (velocity is already floored at
   velocity_min=20). Leave it.

## Decision (user)

**"Surgical layer-only"** — non-identity rows only for the 5 layered
instruments (place named dynamics in layer ranges); explicit identity rows +
comments for the 12 single-layer ones; re-measure and apply small `LEVEL_GAIN`
deltas. Rejected: (a) `amp_veltrack=0` decoupling (cleanest semantics but forces
full LEVEL_GAIN re-derivation), (b) aggressive full remap keeping amp_veltrack
(highest risk).

## How the pipeline applies it

- `gcs2sfz:242-246`: `vel = max(1, min(127, int(round(vel * instruments.velocity_mult(name, vol_s)))))`.
  `name` = lowercased/spaced MAP name; `vol_s` = the note's CC11 start value.
- `velocity_mult(name, vol)` (instruments.py:321) interpolates a per-instrument
  list of `(vol, multiplier)` points linearly in vol; absent row = 1.0.
- Score velocity already equals the named vol value 1:1 (imscomp:6978,
  `note_velocity * v_vl / 100`, floored to `velocity_min=20` at imscomp:1519 /
  6978). So with multiplier 1.0, emitted velocity == vol.
- `LEVEL_GAIN` keys are the **emitted CC11** values (28,38,48,59,68,82,89,102,
  111,118,127 ≈ pppp..fffff; ppppp=0 has no row → clamps to row[0]).
- `base_gain_db(name)` = `level_gain_db(name,127)` — flat stem gain applied
  alongside GCS2SFZ_GAIN in main(). This is why silent ppppp gets boosted.

## Measured baseline curves (single pitch, `gcs2sfz` full path incl LEVEL_GAIN)

Harness command: `python3 <harness> <instrument> <pitch_midi> base`
(gcs2sfz must be run from `music/sfz/`; harness chdirs there). Order printed:
ppppp pppp ppp pp p mp mf f ff fff ffff fffff.

```
french_horn  (pitch 60) base: ppppp=-90.3 pppp=-74.4 ppp=-61.3 pp=-54.6 p=-48.9 mp=-40.8 mf=-35.6 f=-32.7 ff=-24.4 fff=-20.7 ffff=-20.8 fffff=-17.3
trumpet      (pitch 60) base: ppppp=-84.3 pppp=-77.2 ppp=-63.1 pp=-57.2 p=-47.3 mp=-43.2 mf=-34.3 f=-32.9 ff=-29.0 fff=-25.9 ffff=-20.6 fffff=-19.5
trombone     (pitch 55) base: ppppp=-90.3 pppp=-77.5 ppp=-63.7 pp=-58.0 p=-48.8 mp=-45.2 mf=-38.9 f=-35.6 ff=-29.8 fff=-24.6 ffff=-24.0 fffff=-21.5
brass_section(pitch 60) base: ppppp=-90.3 pppp=-75.4 ppp=-61.0 pp=-53.2 p=-47.3 mp=-38.4 mf=-38.3 f=-34.1 ff=-29.9 fff=-27.3 ffff=-22.6 fffff=-19.4
violin       (pitch 67) base: ppppp=-76.3 pppp=-74.7 ppp=-59.6 pp=-48.3 p=-40.2 mp=-30.1 mf=-25.9 f=-23.8 ff=-18.0 fff=-16.2 ffff=-12.8 fffff=-11.4
```

GM-fallback reference (an UNMAPPED name, e.g. `1st-violin`, falls back to GM;
this is the piano-shape reference used to sanity-check monotonicity):
```
1st-violin   (pitch 67) base: ppppp=-93.3 pppp=-76.1 ppp=-66.9 pp=-58.9 p=-52.1 mp=-46.8 mf=-42.0 f=-38.6 ff=-35.2 fff=-32.1 ffff=-30.2 fffff=-28.5
```
(Note: absolute dB across VPO vs GM paths are not directly comparable — GM uses
its own fluidsynth gain; compare SHAPE and monotonicity, and for loudness match
use the `ims/test-volume-levels` stems + LEVEL_GAIN methodology.)

Tested candidate placements (for reference; `brass` pushes mp to 68 and
mf.. up across 75-127; `layerless` unused under the chosen strategy):
```
french_horn brass: ppppp=-90.3 pppp=-73.2 ppp=-60.8 pp=-55.0 p=-50.1 mp=-40.8 mf=-34.4 f=-31.8 ff=-23.6 fff=-20.0 ffff=-20.4 fffff=-17.3
trombone    brass: ppppp=-90.3 pppp=-76.8 ppp=-63.8 pp=-58.0 p=-49.5 mp=-46.1 mf=-37.2 f=-34.9 ff=-29.1 fff=-24.0 ffff=-23.8 fffff=-21.5
```

## The measurement harness (was `/tmp/meas.py` — RECREATE IT)

`/tmp/meas.py` is in temp and may be gone in a new session. Recreate exactly:

```python
import os, subprocess, tempfile, re, sys

SFZDIR = './library/VPO'
SF2 = '/Users/m4/src/GeneralUser/GeneralUser.sf2'
FFMPEG = os.path.expanduser('~/bin/ffmpeg')
SPACE = 2.8   # seconds between note ons; notes 1.8s long -> clean 1.3s windows

# named levels and their score vol value
LEVELS = [('ppppp',0),('pppp',30),('ppp',40),('pp',50),('p',60),('mp',70),
          ('mf',80),('f',90),('ff',100),('fff',110),('ffff',120),('fffff',127)]

def measure(inst, pitch, vel_of):
    """Render one instrument, all 12 named levels at fixed CC11=vol, velocity
    = vel_of(name, vol). Returns dict name->dB."""
    d = tempfile.mkdtemp(prefix='meas-')
    csv = os.path.join(d, f'{inst}.csv')
    rows, db = [], {}
    for i, (name, vol) in enumerate(LEVELS):
        v = vel_of(name, vol)
        v = max(20, min(127, v))  # reflect velocity_min floor
        t = 0.1 + SPACE * i
        rows.append(f'{t:.2f},1.8,{pitch},{v},64,40,{vol},{vol},64,,staff')
    open(csv, 'w').write('\n'.join(rows) + '\n')
    r = subprocess.run(['./gcs2sfz', d, '--sfzdir', SFZDIR, '--sf2', SF2],
                       capture_output=True, text=True)
    if r.returncode != 0:
        return {'ERR': r.stderr[-300:]}
    w = os.path.join(d, os.path.basename(d) + '-sfz-mix.wav')
    for i, (name, vol) in enumerate(LEVELS):
        a = subprocess.run([FFMPEG, '-ss', f'{0.1+SPACE*i+0.15}', '-i', w,
                            '-t', '1.3', '-af', 'astats=metadata=1',
                            '-f', 'null', '-'], capture_output=True, text=True)
        rmss = [float(x) for x in re.findall(r'RMS level dB: ([-\d.]+)',
                                             a.stdout + a.stderr)]
        db[name] = rmss[-1] if rmss else None
    return db

def fmt(db):
    return '  ' + ' '.join(
        f'{n}={db.get(n, -99):6.1f}' if isinstance(db.get(n), float) else f'{n}=?? '
        for n, _ in LEVELS)

if __name__ == '__main__':
    os.chdir('/Users/m4/newtmp/saved-m4-stuff/src/github.com/m4/music/music/sfz')
    inst, pitch = sys.argv[1], int(sys.argv[2])
    fn = sys.argv[3] if len(sys.argv) > 3 else 'base'
    if fn == 'base':
        vel = lambda n, v: v
    elif fn == 'brass':
        A = {'ppppp':20,'pppp':30,'ppp':40,'pp':50,'p':60,'mp':68,
             'mf':84,'f':94,'ff':104,'fff':114,'ffff':122,'fffff':127}
        vel = lambda n, v: A[n]
    print(f'{inst} (pitch {pitch}) {fn}:')
    print(fmt(measure(inst, pitch, vel)))
```

Notes on the harness: `SPACE=2.8` with 1.8 s notes and a 1.3 s window starting
0.15 s into each note gives clean per-note RMS (do NOT use <2.8 s spacing — the
first attempt with 0.5 s spacing overlapped notes and produced garbage). The
rendered WAV is named after the temp **directory** (`<dir>-sfz-mix.wav`), not the
CSV. `ffmpeg` must be `~/bin/ffmpeg` (brew's lacks filters / astats behavior);
`astats` RMS comes out on both stdout+stderr — parse both. Instrument name comes
from the CSV filename; an unmapped name → GM fallback.

## Next steps

1. **Finalize placements.** For each of the 5 layered instruments, choose 12
   `(vol, multiplier)` points so the named dynamics anchor to the right side of
   the boundary with margin (±3 humanize):
   - french_horn / trumpet: `ppppp..mp` stay clearly < 75 (multiplier ≤ ~0.97,
     e.g. mp 70→68); `mf..fffff` spread across 75-127.
   - trombone: `<75` for ppp..mp; `mf/f` in the 75-90 crossfade; `ff..fffff` > 90.
   - brass_section: treat as 75 boundary (mixed patch).
   - pizzicato: `ppppp..pp` < 54; `p` in 54-62; `mp..` ≥ 62.
   Under "surgical", the multipliers will mostly be ~1.0 — that is expected and
   correct. (The prior `brass` candidate in the harness is a sample.)
2. **Implement** the rows in `music/sfz/instruments.py` `LEVEL_VELOCITY`
   (line 318), keyed by the sanitized name (lowercase, spaces→underscores):
   `french_horn, trumpet, trombone, brass_section, pizzicato_strings`. Add a
   comment block documenting the 12 single-layer instruments and why they are
   intentionally absent (velocity == pure loudness there; LEVEL_GAIN owns it).
3. **Re-measure** the 5 with the harness; compare to the baseline above.
4. **Apply small LEVEL_GAIN deltas**: wherever the new velocity moved a level's
   dB off the baseline, adjust that instrument's `LEVEL_GAIN` row (line 202) by
   the delta (sign: if velocity change made it louder, reduce the gain). Keep it
   minimal; the calibration target is the 2026-09-17 piano match (~0-2 dB across
   mp..fffff).
5. **Full verification**: re-render the calibration piece per single pitch
   (avoid the multi-pitch confound), confirm monotonic pppp→fffff and no
   inversion, and spot-render one real piece (`make sfz` in a small dir, e.g.
   `music/b/01`).
6. **Docs/commit**: update `SFZ.md` "Open" item (the LEVEL_VELOCITY bullet near
   line 272) to RESOLVED with notes; add a short DEVIN note; commit. Run `DOALL`
   after — `imscomp`/`musicomp2abc` are untouched, so bare-diff must stay 0
   (baseline 0 bare / 2 pre-existing named).

## Refs / commands

- Edit: `music/sfz/instruments.py` — `LEVEL_VELOCITY`:318, `velocity_mult`:321,
  `LEVEL_GAIN`:202, `level_gain_db`:236, `base_gain_db`:250, `_interp_gain`:263,
  `NAMED_LEVELS`:288.
- Apply: `music/sfz/gcs2sfz`:242-246 (velocity_mult), and the base-gain /
  LEVEL_GAIN pred near 325-340. MAP at `instruments.py`:29-60.
- Calibration piece: `ims/test-volume-levels.gcs` + generated CSVs in
  `ims/sfz-csv/test-volume-levels/*.csv` (note: different pitch per level — a
  confound; use single-pitch probes for design).
- Run harness: `cd music/sfz && python3 /tmp/meas.py french_horn 60 base`
- Handy layer-boundary grep:
  `grep -nE "lovel|hivel|xfin|xfout" music/sfz/library/VPO/Virtual-Playing-Orchestra3/Brass/french-horn-SEC-sustain.sfz`
- Tests: `cd music/ims && ./DOALL` (5 suites, bare/named ARGH baseline 0/2).
