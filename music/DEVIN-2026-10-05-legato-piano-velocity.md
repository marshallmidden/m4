# Legato on piano: `slur_velocity` is the wrong lever (2026-10-05)

Dev notes @ 2026-10-05. Third follow-up in the legato series, after
`DEVIN-2026-09-27-legato-slur.md` and `DEVIN-2026-09-28-legato-gm-mono.md`.
Read-only session: **no compiler source was modified.** Everything below is
measurement plus the two conclusions it forces.

Objective: make `l`-slurred notes in `.gcs` sound legato on piano and then on
strings — no chopping, some attack and release, sustain matching the previous
note's sustain.

## The short version

The shipped `slur_velocity` default of **45% is roughly 2x too aggressive**, and
the CC64 seam pedal that the code credits with making piano slurs work
**contributes nothing measurable.** The note-off overlap is doing all the work.

| | shipped | measured better |
|---|---|---|
| `slur_velocity` | 45% | **~85%** (or 100) |
| CC64 seam pedal | claimed load-bearing | **~0 dB contribution** |

## Measurement

Micro-benchmark: piano only, GeneralUser.sf2, 120bpm, 4/4, one voice, a 4-note
slur `4c4l, 4e4l, 4f4l, 4g4` then two plain quarters. RMS dB per note, attack =
first 40ms from onset, body = 120-240ms after onset. Onsets exactly 500ms apart
so the windows never overlap. Fresh-struck notes are the reference.

| `slur_velocity` | fresh note | slur 1 | slur 2 | slur 3 |
|---|---|---|---|---|
| **45 (shipped)** | -38/-41 | **-43/-50** | **-48/-49** | **-47/-49** |
| 65 | -38/-41 | -40/-48 | -45/-47 | -44/-47 |
| 75 | -38/-41 | -38/-45 | -43/-45 | -42/-44 |
| **85** | -38/-41 | **-37/-44** | **-41/-43** | **-40/-43** |
| 95 | -38/-41 | -35/-42 | -40/-42 | -39/-41 |

Also measured, isolating the pedal (`ovp_ms` = how long the source's note-off is
held into the slurred-in onset):

| variant | slur 1 atk/body | slur 2 atk/body | slur 3 atk/body |
|---|---|---|---|
| sv79, pedal 45ms, ovp 31ms | -37.2/-45.7 | -40.1/-46.8 | -39.6/-46.3 |
| sv79, **no pedal**, ovp 60ms | -37.2/-45.7 | -40.3/-46.7 | -39.6/-46.3 |

**Identical to within 0.3 dB.** The pedal is not carrying the sustain.

## Conclusions

1. **The `l`-semantics work from the previous sessions is sound; only the
   velocity lever is wrong.** At 45% the slur-ins sit 8-12 dB under a fresh
   strike *and the gap widens down the run* (-43 → -48), because each note is
   both quieter and shorter. That decaying staircase is the "chopping" complaint,
   reproduced exactly.

2. **Cutting velocity cannot work on a piano patch, and this is structural, not
   a tuning miss.** On piano, velocity scales the whole note — attack *and*
   body. There is no separate "soft onset" control, so any cut that tames the
   attack necessarily starves the sustain. This is why `DEVIN-2026-09-29` was
   heading for 68%: right diagnosis, insufficiently aggressive remedy.

3. **85% beats 68% — and for a different reason than round 3 assumed.**
   `DEVIN-2026-09-29-legato-round3-plan.md` argued the pedal now carries the
   attack-masking so a milder cut became safe. The data says the pedal carries
   essentially nothing and the **overlap** does, so the cut can go to nearly
   nothing. At 85% the slur-ins land within 1-4 dB of a fresh strike and the run
   no longer decays.

4. **The code comment at `imscomp:7121-7131` is wrong and must be corrected.**
   It claims the CC64 seam pedal "is what makes the pair read as connected on a
   percussive patch like the piano." Measured, it is worth ~0 dB. This is
   load-bearing: an incorrect causal comment is how round 2 ended up bending
   code to match a wrong theory (see "What was learned the hard way" in
   `DEVIN-2026-09-27`). Either justify the pedal with a case where it earns its
   place, or drop it and say the overlap does the work.

## Per-instrument read (nothing here is measured except piano)

- **Piano** — sustain match is the hard part, because the patch decays on its
  own and velocity is the only level control. The measured fix is to drop the
  cut to ~85% and let the overlap connect. This still does **not** deliver a
  true no-attack legato; nothing in the GM/fluidsynth piano path can without
  editing the SoundFont patch.
- **Violin / strings** — genuinely solvable and probably already is. The VPO
  path has `GCS2SFZ_LEGATO_ATTACK_MS` (default 150, `gcs2sfz:397` `_slur_slow_attack`)
  which re-renders slurred-in notes on a slow-onset copy of the sustain patch and
  `amix`es it with the fast-attack rest. A bowed string *can* have a soft onset
  independent of level — exactly the control piano lacks. **Measure before
  touching.**
- **Horns / brass** — same family as strings (sustained, VPO-mapped), so they
  follow from the violin decision. Untested.
- **Drums / percussion** — exempt outright. A slurred drum note is meaningless
  and CC64 is already excluded by the `l_instrument in instruments` guard at
  `imscomp:7132`. No work.

## Open, unresolved: `slur_velocity` may not be settable from a `.gcs`

A `global slur_velocity 90` line did **not** change the emitted `.fs` velocity —
it stayed 35 for every one of 45 / 68 / 90 / 100. **Not diagnosed.** Either
`global` cannot override a `createglobalvar` at level 0, or the value is read
before the `.gcs` line executes, or the syntax is wrong.

This is the first thing to settle, because it is the one knob that matters. Do
not record it as a confirmed bug until the test below has run:

```
cd music/ims
printf 'title t\ntempo 120,4\nmeter 4/4\nglobal slur_velocity 90\nvoice 1\ninstrument 1 acoustic grand piano\nvolumes mf\nmeasure 1\nv 1: 4c4l, 4e4l, r4\n' > zz.gcs
gcc -E -x c -nostdinc -C -CC -traditional-cpp zz.gcs -o zz.E
./imscomp --fluidsynth zz.E zz.fs && grep noteon zz.fs
```

If `global` genuinely cannot set it, that is a real bug worth its own note.

## Plan for next session

1. Settle the `slur_velocity` plumbing above.
2. Re-run the sweep **through `imscomp`** once (1) is done, then **listen** at
   45 / 75 / 85 / 100. RMS says 85; the ears decide whether 85 is "tasteful
   legato" or "quietly re-struck."
3. If 85 still sounds re-struck, **the answer is not velocity.** It is porting
   the VPO slow-onset trick (`_slur_slow_attack`, `gcs2sfz:397`) to the GM piano
   — an `ampeg_attack` override on a copied patch, i.e. a patch edit rather than
   a MIDI value. Bigger change; treat as a separate decision.
4. Correct the comment at `imscomp:7118-7131` (conclusion 4).
5. Strings/brass: measure via `cd music/ims && make -B test-legato.mp4
   test-legato-sfz.mp4`. If the VPO swell already reads as legato there, the
   string side needs no code and the work is piano-only.

## Caveats on the numbers above

- The velocity sweep was done with **hand-written `.fs`**, not through
  `imscomp`, precisely because of the unresolved plumbing bug. The numbers are
  about the synth, not about the compiler path. Re-measure through the compiler
  before changing a default.
- 40ms / 240ms windows cannot see **perceptual attack sharpness**. At 85% the
  slur-ins may still *sound* more percussive than the dB suggest.
- Piano only. Nothing measured for violin, brass or piano-adjacent patches.

## Traps hit this session

Each of these produced a wrong number or a hung process before being caught:

- **Passing a `.fs` to fluidsynth as a file argument makes it parse it as MIDI
  and drop into the interactive shell** — it hangs until interrupted, and
  produces a 44-byte WAV. It must be piped:
  `cat x.fs | fluidsynth -q -n -K 256 -r44100 -F x.wav <sf2>`.
- **`ffmpeg` `astats` prints `RMS level dB:` only when `-v error` is absent,
  and `ametadata=print` writes to stderr, not stdout.** A helper returning `None`
  for every window is almost always this, not a broken render.
- **MIDI varlen encoding.** The naive `while True: out.append((v & 0x7f) |
  0x80); v >>= 7` builds the bytes LSB-first and silently corrupts every delta
  >127 — total track length came out ~7x long. Correct form: append `v & 0x7f`,
  shift, break when zero, then `reverse()` and set the continuation bit on all
  but the last byte. Verify with a round-trip decode.
- **Deleting a MIDI event must carry its delta** into the next event, or every
  subsequent timestamp shifts.
- **Two `sleep`-accounting bugs in my own generator** double-counted the note
  slot and put onsets at 0.42s instead of 0.50s, which invalidated the first two
  rounds of comparison before I checked the onsets. *Always verify the onsets
  before believing the levels* — the numbers look perfectly plausible either way.

## Files touched

**None.** Read-only session. `git status` clean at `aabaea5c`.

Scratch created (all gitignored — `music/ims/.gitignore` has a bare `*`, so
`make clean` will remove these; copy out if the harness is wanted):

- `music/ims/zz-bench.gcs` — the 6-line piano legato micro-benchmark.
- `music/ims/zz-bench.E`, `zz-bench.fs` — build artifacts.
- `music/ims/zz-legato-bench/` — reusable harness:
  - `gen5.py` — builds a parametrized `.fs` (`sv`, `ovp_ms`, `ped_ms`, `ped`,
    `note_len`, `chan`/`prog`/`sfid`); onsets exactly `note_len` apart.
  - `cmp2.py` — per-note pre-onset / attack / body RMS table.
  - `measure.py` — same idea for a real compiler-produced `.fs`.
  - `decay.py` — 100ms-step decay envelope of the first note.

**These should probably move somewhere tracked** (`music/sfz/` or a top-level
`bench/`) so the next session does not have to rebuild them.

## Regression guardrails (unchanged, for whoever edits the code)

- Baseline: **0 bare ARGH / 2 named** (gershwin `new-g3`, both compilers failing
  identically — not a mirror problem). Full `music/ims/DOALL` takes >10 min and
  exits non-zero on gershwin **by design**.
- Fast smoke: `b/01/v1-1`, `b/01/v1-2`, `ims/test-legato` across `--vert --hori
  --midi1csv --fluidsynth --abc`, diff `imscomp` vs `musicomp2abc` per format,
  must be 0 diffs.
- Both compilers must stay **byte-identical** for everything except
  `--sfzpipecsv`. If `slur_velocity`'s default changes, it changes in
  `ims/imscomp:1519` AND `musicomp2abc/musicomp2abc:1519`.
- `pedal_overlap`'s default also drives the legacy `.gcs` `sustain_pedal`
  feature (`imscomp:7207-7219`) — changing it shifts that timing too. That is a
  known coupling, not a bug.
- Never judge this work by sample-domain A-B diffing of rendered WAVs; see
  "Still open" #2 in `DEVIN-2026-09-27` and the trap list in `AGENTS.md`.

---

# Follow-up 2026-10-09: implemented a soft-onset filter (Option A)

This session resolved the two open items and shipped a lever that is **not**
velocity and **not** a patch edit.

## The plumbing bug is settled

`global slur_velocity 90` does nothing because `global` only *declares* a name
(it reaches `handle_vars_and_global` -> `createlocalvar`); it never assigns.
The correct piece-side syntax is **`calc slur_velocity = 90`**. Verified: a
slur-in note's emitted velocity moves 35 -> 70. So `slur_velocity` *is* settable
from a `.gcs`; the old note's "unresolved" is closed.

## The lever: NRPN FILTERFC closed only at the slurred-in onset

Velocity cannot decouple onset from body on a piano (this note's conclusion 2),
but a low-pass can: close `FILTERFC` (gen 8) for a few tens of ms at the
slurred-in onset, then reopen it. The hammer-strike HF is shaved; the body is
untouched. This is what the old "single knob cannot give soft onset + full
sustain" wall was missing.

Shipped defaults (`ims/imscomp:1519-1521`, mirrored):
- `slur_velocity` **45 -> 100** (no velocity cut).
- `slur_filter` = `-1500` timecents FILTERFC offset at the slur-in onset
  (`0` disables). `-1000..-2000` is the measured sweet spot.
- `slur_filter_hold` = `24` ticks to keep it closed before reopening
  (~50ms at quarter=480 ticks). Emission at `ims/imscomp:7156` + mirrored.

## The NRPN gotcha that makes or breaks it

FluidSynth resets `chan->nrpn_select` to `0` after the **first** DATA_ENTRY
(`fluid_synth.c:1865`). So the MIDI-spec CC6+CC38 pair makes CC38 re-trigger
`set_gen` against **gen 0** (STARTADDROFS) and corrupts the sample start. The
only reliable form is **one data-entry byte per NRPN message, re-selecting the
gen each time**: `CC99=120, CC98=8, CC6=msb` (LSB left at 0). Then
`offset = (msb*128 - 8192) * 2`, i.e. `msb = 64 + offset/256`. This is also why
the previous session's hand-written filter sweep was subtly contaminated.

## Measured result (A/B, `slur_filter=0` vs `-1500`)

`test-legato` grand piano, per-note RMS. Fresh-struck notes are byte-identical
between the two renders (the reopen at +24t leaves them clean); only slur-ins
change:

| note | band | `slur_filter=0` | `slur_filter=-1500` | delta |
|---|---|---|---|---|
| slur-in (p74) | >2kHz 0-12ms | -45.2 dB | **-52.7 dB** | **-7.5 dB** |
| slur-in (p76) | >2kHz 0-12ms | -44.4 dB | **-51.6 dB** | **-7.2 dB** |
| slur-in body | 100-300ms | -37.1 / -37.9 | -37.1 / -38.2 | ~0 dB |
| fresh notes | >2kHz 0-12ms | -44.6..-48.0 | same | 0 dB |

Onset HF down ~7 dB, body within ~0.3 dB, no pre-onset gap (<-48 dB). That is
the structural fix the velocity lever could never be. Tune `slur_filter` to
taste (-1000 mild, -2000 stronger).

Harness: `zz-legato-bench/hfband.py <wav...>` (per-note HF-band vs body table;
reads the sibling `.fs` for onsets).

## SFZ is deliberately pinned to the old behavior

`--sfzpipecsv` shares `print_out_midi1csv_notes()`, so a bare default change
would have leaked both the uncut slur-in velocity *and* the NRPN FILTERFC events
into the VPO CSVs (gcs2sfz relies on the old 45% cut riding in the CSV, and
sfizz must never see a GM filter NRPN). The SFZ dispatch branch instead pins
`slur_velocity=45`, `slur_filter=0` for `--sfzpipecsv` only (imscomp-only, in
SFZ-feature territory, so musicomp2abc stays source-identical). Verified: the
`acoustic_grand_piano.csv` for `test-legato` is byte-identical to the
pre-change compiler (slur-ins still 43/44, no NRPN), while the GM `.fs` carries
the fix. **SFZ output is untouched.**

## Guardrails re-run

- `music/b/AAA.diff._2`: 0 ARGH.
- Full `music/ims/DOALL`: **bare 0 / named 2** (the known gershwin `new-g3`),
  i.e. baseline. Both compilers byte-identical on `test-legato` in `--csv` and
  `--fluidsynth`.
- Files now touched (this is no longer a read-only note): `ims/imscomp`,
  `musicomp2abc/musicomp2abc` (mirrored), `ims/test-legato.gcs` (pre-existing
  user edit), `DEVIN-2026-09-29-legato-round3-plan.md`.

## Follow-up 2026-10-10: the shipped lever is now VELOCITY, default 55

Listening session on the GeneralUser grand resolved which lever carries the
GM slur-in soft onset. Findings:

- **The filter (from the 10-09 follow-up) was the wrong philosophy and is now
  an option, not the default.** At the measured sweet spot (-1500..-2000) the
  slur-in *spectral* onset softens but the **body stays at full fresh-note
  level**, so the new note still jumps ~9 dB on top of the old note's decayed
  tail ("new note too present"). Pushed to -4000 you cross a cliff: the attack
  event vanishes and the ear attributes the continuing sound to the OLD note
  ("the first note sustains, not the new one"). That reading is physical, not
  perceptual: the previous note's tail rings ~0.1 s into the onset under the
  seam pedal while the new onset is ~14 dB down.
- **What the user actually described is the velocity model**: cut the old note
  off, re-strike the slur-in at a velocity that puts its A/D (same duration —
  velocity never changes ADSR time) *and* its sustain at the old note's
  decayed level. On a piano patch velocity scales the whole envelope, so this
  is one number: **55% at ff was approved by ear**; the sweep below proves one
  % carries to every volume, so no per-dynamic table is needed.

Note-body sweep (gen5 harness `zz-legato-bench/sweep.py`, starting velocity =
dynamic's vlprint base 50/60/70/80/90/100, slur-in = 55% of it, 100-300ms body
window):

| dyn | base | slur vel | fresh body | slur body | slur-under-fresh |
|---|---|---|---|---|---|
| pp | 50 | 28 | -49.6 | -60.0..-61.3 | 10.4..11.7 dB |
| p  | 60 | 33 | -46.8 | -56.7..-57.7 | 9.9..10.9 dB |
| mp | 70 | 38 | -44.6 | -54.0..-54.8 | 9.4..10.2 dB |
| mf | 80 | 44 | -42.8 | -51.2..-51.9 | 8.4..9.1 dB |
| f  | 90 | 50 | -41.1 | -50.0..-50.6 | 8.9..9.5 dB |
| ff | 100 | 55 | -39.8 | -48.5..-49.1 | 8.7..9.3 dB |

The Grand's velocity→gain curve is effectively log-linear, so 55% gives a
nearly constant ~9-11 dB offset under a fresh note at every volume — exactly
the "sustain = where the old note was" behavior, uniformly. Sanity: earlier
trap hit re-confirmed — my first sweep measured worsening offsets down a run
(10→30→13 dB) purely from misaligned onsets (gen5's ped/overlap shift each
onset; must parse them from the generated `.fs`, not assume `note_len`
spacing).

### Shipped changes (2026-10-10)

- `slur_velocity` default **100 → 55** (comment rewritten): ADSR-timed,
  whole-envelope soft re-strike.
- `slur_filter` default **-1500 → 0** (the NRPN FILTERFC onset close is now an
  opt-in known as the "spectral" arm; the emission code and its NRPN gotcha
  notes are untouched). `slur_filter_hold` stays 24.
- `--sfzpipecsv` pin (imscomp-only) unchanged: `slur_velocity=45`,
  `slur_filter=0` — re-verified, VPO GM-fallback CSV untouched.
- Both compilers mirrored (byte-identical on v1-1/v1-2 and test-legato with
  the user's slur overrides; b-suite AAA 0 ARGH).
- `AGENTS.md` slur paragraph updated to the velocity-55 reality.

### Verified on the real piece

`b/sonata14/s14.gcs` (Moonlight, solo piano, pp/p/mf with cresc/dimin, 38 `l`
slurs) builds clean with the new defaults: 0 NRPN events, slur-ins at the 55%
layer, `s14_2.mp4` rendered (139.7 s). Pre-existing note: s14 `.fs` has a
long-standing ~±50-100 tick `pitch_bend` discrepancy between the two
compilers in the per-voice headers (confirmed present at `aabaea5c`, untouched
by this work); it is not slur-related and not part of the tracked console-diff
metric.

### Still open

Adaptive slur velocity (pick the value from the previous note's current
length/decay instead of a global %) remains the future improvement; the sweep
shows a global 55 needs no per-volume correction, but a global cannot track a
long-decayed note the way an adaptive one would.

