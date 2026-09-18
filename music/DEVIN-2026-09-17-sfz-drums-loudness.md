# DEVIN 2026-09-17 — SFZ drums separation + GM CC11 bug + loudness calibration

Session record + **TODO list** for the SFZ drum / loudness work. Kept in-repo so
progress survives tooling failures. Companion: `SFZ.md` (NEXT FIXES), `AGENTS.md`.

## TODO (current)

1. [x] Separate drums into per-instrument CSVs in `imscomp --sfzpipecsv`
   (channel-9 notes keyed by pitch). DONE, verified.
2. [x] Voice each drum CSV on its VPO patch key (`KIT_KEY` in
   `music/sfz/instruments.py`, applied in `music/sfz/gcs2sfz`
   `render_instrument`). DONE, all 5 test drums audible.
3. [x] Confirm GM drum path (channel 9 / bank 128) already correct in both
   compilers — no change needed.
4. [x] Fix GM-fallback CC11 in-note ramp stale-state bug (see below). DONE.
5. [x] DOALL regression: 0 bare ARGHs, 2 pre-existing named (gershwin new-g3).
6. [x] **Loudness: per-instrument, per-level normalization to the piano.**
   Implemented as `LEVEL_GAIN` + `level_gain_db()` / `base_gain_db()` in
   `music/sfz/instruments.py`, split into a flat stem gain + CC11 residual in
   `music/sfz/gcs2sfz` (see "Loudness table" below). Interpolates linearly
   between measured levels (crescendos glide).
7. [x] Re-render `ims/test-volume-levels`, re-measure. mp..fffff now within
   ~0–2 dB for essentially all in-range instruments. Remaining shortfalls
   (iterated below) mostly ppp/pppp ~3–8 dB (soft VPO attack vs the 0.25 s
   measurement window) and a few one-level outliers.
8. [ ] (Deferred by user — "just the gain table" for now) Silent VPO range
   mappings: **piccolo fully silent** (its sustain patch covers only d5–d#5;
   written concert 60–69 misses it), **timpani high notes silent**
   (`timpani-hit.sfz` tops out ~f#3), **tuba high notes**. These stay OUT of
   `LEVEL_GAIN` (a flat base gain would make their audible notes too loud).

## Completed this session

### 1. SFZ drums: separate + correct key (files: `ims/imscomp`,
`music/sfz/instruments.py`, `music/sfz/gcs2sfz`)
- All drums share GM channel 9, so `print_out_sfzpipecsv` previously merged them
  into one `(voiceon, chan)` bucket. Now channel-9 notes are attributed by their
  fixed GM key (`drum_key_to_inst`: 35→acoustic bass drum, 38→snare, 49→crash 1,
  54→tambourine, 81→open triangle, …), giving one CSV per drum.
- VPO drum patches are voiced on their own keys, not GM keys (bassdrum.sfz has
  regions only at c2/d2 and **no lokey/hikey**, so GM key 35 hit nothing →
  silent). `instruments.KIT_KEY` maps each drum name to a fixed patch key
  (bass drum 36, snare 48, crash 1/2 66, ride 1/2 68, tambourine 54, triangle
  45); `gcs2sfz.render_instrument` rewrites the CSV pitches to that key into a
  temp CSV before rendering (`orig_csv_path` retained for the reverb lookup).
- Timpani is **melodic** (keep written pitch), not a kit key.
- Verified with one isolated note each: bass drum −23.4, snare −24.3,
  triangle −25.8, tambourine −28.0, crash −22.2 dB max (all audible; previously
  silent for the GM-keyed ones).

### 2. GM-fallback CC11 ramp bug (file: `music/sfz/gcs2sfz`)
- Regression from `4d7d486c`: the GM path (`pred_expr=False`) skipped per-note
  CC11 but still emitted the in-note cresc/dim ramp. The ramp left the channel's
  CC11 at the note's `vol_end`; since no CC11 was re-emitted at the next
  note-on, every later note whose `vol_start==vol_end` inherited it. In
  `ims/test-volume-levels` every non-first instrument's note 1 fades
  (`vol 71→0`), so **`piano` was silent after note 1** (`default` worked only
  because all its endpoints were equal, so no ramp was ever emitted).
- Fix: GM path now emits **no CC11 at all** (velocity-only, as the design
  comment always claimed). CC11 stays at 127. `piano` now renders all 12
  dynamics (mp −45.5 … fffff −23.2 dB).

### 3. Loudness table: per-instrument, per-level match to the piano
(files: `music/sfz/instruments.py`, `music/sfz/gcs2sfz`)
- `instruments.LEVEL_GAIN` is a `name -> [(vol, gain_db), ...]` table measured
  from `test-volume-levels` (gain = dB needed to bring the instrument up to the
  piano at that level). `level_gain_db(name, vol)` linearly interpolates in
  vol/dB and holds the endpoints outside the measured range; capped by
  `LEVEL_GAIN_MIN_DB=-6` / `LEVEL_GAIN_MAX_DB=30`.
- **Why split flat + CC11**: the correction is level-dependent, but the
  predistorted CC11 (`GCS2SFZ_EXPR_EXP=4`) is already ~127 at the top dynamics,
  so a large boost there cannot be delivered through CC11. So each instrument
  gets (a) a **flat stem gain** = `base_gain_db(name) = level_gain_db(name,127)`
  (the part with no CC11 headroom), applied in `gcs2sfz.main()` alongside
  `GCS2SFZ_GAIN` (ffmpeg `volume=..dB` on the stem, before mixdown), and (b) a
  **CC11 residual** = `level_gain_db(name,v) - base_gain_db(name)` applied in
  `write_midi._pred()`.
- Out-of-range instruments (`piccolo`, `timpani`, `tuba`) and one-shots
  (`canon`, `church_bells`) are absent from the table → gain 0, unchanged.
  Do NOT add them until their range mappings are fixed (a flat base gain on a
  mostly-silent patch makes the audible notes far too loud — observed at +29 dB
  for `timpani`).
- Mixdown (`gcs2sfz.mixdown`) already has `alimiter=limit=0.95:level=false`, so
  the boosts cannot clip; `level=false` preserves dynamics. Test mix peaks
  −0.4 dBFS, mean −20.8 dBFS.
- Result (piano-relative, + = instrument still quiet): in-range instruments
  within ~0–2 dB across mp..fffff; ppp/pppp residuals up to ~8 dB for some
  (english_horn, crash, pizzicato, bassoon, contrabass, viola, oboe, flute —
  likely the slow VPO soft attack landing outside/at the edge of the 0.25 s
  window, not a real loudness error); a few one-level outliers (contrabass
  fffff +10.9 — its table's 127 point is a resonance outlier, held at 3.9).
- `ppppp` is meaningless as a target: at vol 0 the piano is silent (CC11=0), so
  every SFZ instrument shows a large negative "gain". Ignore it.

## RESOLVED: v1-1-sfz.mp4 "instruments start before they should" (tempo bug)
User report (after the loudness change): in `b/01/v1-1-sfz.mp4` an unexpected
note ~13 s and several instruments by ~18 s. Root cause was a **pre-existing
`print_out_sfzpipecsv` bug**, exposed (not caused) by the fact that we re-rendered
with it. Rescue snapshot history: `DEVIN-2026-09-17-sfz-WIP/STATE.md`.

Cause: `release_sfz_note` computed note times by multiplying the ABSOLUTE tick by
the tempo in effect at note-on (`start_sec = start_tick * spt`). v1-1 has
`Tempo, 88, 0.125` at tick 0 and `Tempo, 112, 0.5` at tick 23040; after the
change `spt` drops, so any later note collapses backwards. E.g. tick 23044 →
`23044*0.0005581 = 12.859 s` instead of the correct ≈65.456 s. That is exactly
the "note ~13 s" (and the "extra instruments ~18 s"); the strings that should
enter at measure 4 were emitted at 12.86–13.93 s. Notes before the change (e.g.
the pizz part, all measures 1–3) were correct, which is why it looked
score-like at first. Proof (instrumented): `voice14 off_chan14 pitch60
tick24336 -> start23044 start_sec12.859375`; `--midi1csv` had that same note at
tick 23044 (correct ≈65.45 s), and the GM `.fs` was correct all along.

Fix (in `ims/imscomp`, SFZ-only; NOT mirrored to musicomp2abc):
- Added a tempo timeline just below `sfz_sec_per_tick`:
  `tempo_points = [(tick, sec_per_tick, seconds_at_tick), ...]`,
  `add_tempo(tick, spt)` (replace the initial point at tick 0; append when
  `tick > last` and `spt` changed; ignore duplicates/out-of-order) and
  `tick_to_sec(t)` (`psec + (t-pt)*pspt` for the last point ≤ t).
- Tempo handler now calls `add_tempo(current_tick, sfz_sec_per_tick(...))`.
- `release_sfz_note` uses `start_sec = tick_to_sec(start_tick)` and
  `dur_sec = tick_to_sec(tick) - start_sec`; the per-note `spt` tuple field was
  dropped from the pending-note key.

Verified on b/01/v1-1 (fresh `--sfzpipecsv`):
- violin.csv first note now **16.349432 s** (measure 4, key 79 = matches the
  `.fs` ch14 first noteon at 16351 ms); zero string/brass/percussion notes
  before 16.3 s.
- Notes across the tempo change are correct and monotonic: tick 23044 →
  65.456778 s; violin.csv spans 16.3–473.7 s with 0 backwards steps.
- SFZ max time 473.7 s == GM `.fs` last noteon 473.7 s (`.fs` total render
  476.1 s). Strings/cello/contrabass/timpani/trumpet/viola now all first-note
  ≈16.35–16.37 s; only winds + pizzicato start at 0 (as notated).
- Rebuilt `v1-1-sfz-mix.wav` (483.0 s) and `v1-1-sfz.mp4` (476.958 s) — the
  video now matches the GM `v1-1_2.mp4` timeline.
- DOALL after the fix: 0 bare / 2 named (same pre-existing gershwin new-g3 in
  both compilers) → no regression. Note DOALL does NOT exercise `--sfzpipecsv`.

Blast radius: this affects EVERY multi-tempo piece's SFZ render (the bug needed
only two distinct tempos). Affected `SONGS`/`ims`/`t`/`b` pieces include
b/01 v1-1, b/02 b2m1, b/03 v3-3/4, b/04 v4-1/3, b/06, b/09 b9m2/m3, t/e,
all Gershwin, and songs gnome/m1a/macros-fugue-in-c/kleine-suite/pete/etc.
Their `*-sfz.mp4`s are wrong until re-rendered (`make sfz-mp4`); the GM
`*_2.mp4`s are unaffected. Full SFZ re-render pending user go-ahead.

Removed the temporary `SFZ_DBG` debug dumps (per-note `.raw`/`.rel`) and the
now-duplicate second `sfz_sec_per_tick` def after verification.

## Measurement notes (for the loudness table)
- Method: parse each `ims/sfz-csv/test-volume-levels/*.csv` for note starts;
  RMS over `[start+0.04, +min(dur,0.25)]` of the stem.
- **Use each file's own sample rate** (`.fs`/GM/fluidsynth = 44.1 kHz,
  sfizz/VPO = 48 kHz) — indexing 48 kHz stems with 44.1 kHz silently corrupts
  every window.
- Reference = the **piano stem** (`acoustic_grand_piano`, GM fallback, already
  calibrated to the `.fs`), per level — NOT each instrument's own GM program
  (that gave implausible brass offsets because the GM brass programs render
  very quietly, e.g. trombone mp ref max −66.8 dB).
- `.fs` reference render command:
  `cat ims/test-volume-levels.fs | ~/bin/fluidsynth -q -F /tmp/tvl-gm.wav /Users/m4/src/GeneralUser/GeneralUser.sf2`
  (the `.fs` itself sets `synth.gain 0.5`).
- Scratch scripts used this session: `/tmp/measure_levels.py`,
  `/tmp/calibrate.py`, `/tmp/calibrate2.py` (calibrate2 = against the piano
  stem; prints the per-level gain table).
- First-order per-level spread vs the piano (before implementing): sustained
  VPO strings/winds run ~7–16 dB quiet at mid levels but only ~13 dB at
  fff–fffff; so a flat gain is insufficient — the correction is level-dependent.
