# SFZ articulation fix — melody drowned out in the SFZ render (2026-09-11)

## Context / test case

Repro: Beethoven Symphony No. 1 first movement, `b/01/v1-1.gcs`, **measure 5**.
The melody (violin1A + violin2A, sfz voices 14/18) was inaudible in the VPO/SFZ
render (`v1-1-sfz.mp4`) while clearly audible in the GM/fluidsynth render.

A trimmed copy `b/01/v.gcs` isolated measure 5 for iteration (deleted after —
do NOT expect it; see findings below to reproduce on `v1-1` directly). Voice
filter `--voices 14`/`--sfzpipecsv` slices per-instrument CSVs, so any sfz
analysis over measure 5 runs exactly as analyzed here.

## Root cause: articulation (VPO sustain smear), NOT loudness

- SFZ violin stem was ~2.5x LOUDER in RMS than the GM violin stem (0.0069 vs
  0.0028) yet the user heard the GM line as clear and the SFZ line as "drowned
  out". Exhaustive flute-boosting experiments (`GCS2SFZ_GAIN="flute:..."`,
  solo-flute) were misdirected — the melody is the violins.
- The real problem: the VPO sustain patches attack over 0.2–0.6s and release
  over 0.6–2.25s. At ~44 bpm a 16th-note is 0.34s, so every onset smears into
  the previous note and the whole melodic run becomes a continuous mush that
  only reads as "texture" in the mix, even though its energy is present.

## Library survey (all numbers from the actual .sfz files)

18 of 26 mapped instruments are smearing sustain patches:
- Bowed strings (worst): violin 0.4/1.9, viola 0.6/~2.0, cello 0.6/2.1,
  contrabass 0.3/2.25 (attack/release seconds).
- Woodwinds: flute 0.3/0.9, oboe 0.2/0.9, clarinet 0.4/0.6, bassoon 0.3/0.6,
  english horn / piccolo ~0.25–0.5.
- Brass: french horn/trumpet/trombone/bass-trombone/tuba/brass-section all
  ~0.2–0.4 / 0.6–0.65.
- Clean: timpani, pizzicato strings, glockenspiel, vibraphone, xylophone,
  tubular bells, one-shots.

Every SEC/SOLO string/wind/brass family also ships a full articulation grid
(`-SEC-<staccato|accent|normal-mod-wheel|sustain>.sfz`), which the fix exploits.

## What was tried (mechanical, measured)

| approach | violin-stem whole-m5 RMS | result |
|---|---|---|
| sustain (old default) | 0.0069 | flat, mushy |
| tightened sustain copy (atk 0.06/rel 0.35) | 0.0030 | quieter, still mushy |
| staccato-only split | 0.0045 | pulses but decays to silence between notes |
| fast-attack sustain (attack→0.03, keep release) | on par | connected but onsets weak |
| **staccato split + sustain-under (blend)** | 1.5x sustain | **wins on listen** |

User verdict on blends/segments: **blend is best** ("articulation is neat, but
drops down quickly" was the staccato-only complaint — that's what
ARTIC_BLEND fixes).

## Shipped fix (now the DEFAULTS in `gcs2sfz`)

`gcs2sfz` renders each part as:
- **Short notes** (dur < `GCS2SFZ_ARTIC_MAX`, default 0.5s) → the instrument's
  `-staccato` variant **plus** the sustain patch layered underneath
  (`GCS2SFZ_ARTIC_BLEND`, default 1), so the staccato gives the crisp onset and
  the sustain keeps the body from decaying to silence.
- **Long notes** → the sustain patch with `ampeg_attack` overridden to
  `GCS2SFZ_ATTACK` (default 0.03s) so legato runs connect with distinct onsets.
  The override rewrites the .sfz text in a same-dir temp copy (a wrapper
  `#include` CANNOT override group-level ampeg) — `_edited_patch()`.
- Same-part split implemented as two/three sfizz renders + ffmpeg `amix`
  (`normalize=0:duration=longest`), then the existing offline aecho room tail.

Env: `GCS2SFZ_ARTIC_MAX` (0 disables split), `GCS2SFZ_ARTIC_BLEND` (0
disables), `GCS2SFZ_ATTACK` (0 disables). Plus the per-instrument loudness knob
(or quiet Sonatina string samples): `GCS2SFZ_GAIN="violin:5"` (−dB/+dB per stem,
applied pre-mix via ffmpeg volume).

Verified in the v1-1 measure-5 mix: violin melody presences pulses at RMS
0.022–0.034 (vs old flat 0.004–0.008).

## Files

- `music/music/sfz/gcs2sfz` — split-articulation renderer; env knobs in module
  docstring; `_staccato_variant`, `_edited_patch`, `_amix`, `_notes_to_csv`,
  `_render_sfz` (extracted from the old single `render_instrument`).
- `music/SFZ.md`, `AGENTS.md` — articulation + gain env knobs documented.

## Open items

1. 1812 (`t/e/e.gcs`) uses nearly all the smearing families (piccolo, oboes,
   clarinets, english horn, bassoons, horns, cornets, trombones, strings) —
   expect the same win there; re-render all 25 mp4s with `make sfz`/`sfz-mp4`
   once the user approves.
2. `GCS2SFZ_GAIN` has no default gain table yet — Sonatina string samples run
   quiet vs VPO brass; per-family defaults could be added to
   `instruments.py`/gcs2sfz if a piece still buries melody after articulation.
3. The staccato/sustain choice is duration-based only; the CSV carries no
   legato-vs-staccato marking, so slurred (`l`) and marked-staccato short notes
   render the same. A future `--sfzpipecsv` note-atttack column could carry
   per-note articulation intent.
4. **NEW BUG (reported end-of-day:** b/01 v1-1, FIRST measure: the pizzicato
   sounds funny — the 2nd pizz note is too loud. Not yet investigated. Likely
   candidates: per-note velocity/CC11 handling for the pizz part, imscomp
   accent/stress on that note, or the VPO pizzicato mapping velocity layers.
   Trigger: `make sfz` in b/01 (or `--sfzpipecsv --measures 1` slice) and
   A/B that pizz phrase against the GM render.