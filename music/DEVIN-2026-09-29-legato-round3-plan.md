# Session state: legato work, 2026-09-29 (saved before macOS/reinstall reboot)

This file is a handoff for the NEXT session. Context: the two compilers and
gcs2sfz work is mid-flight on making slurred (`l`) notes legato. Rounds 1+2 are
done and measured; round 3 was designed but NOT coded when the machine went
down for a macOS reinstall (second day in a row). Work dir:
`/Users/m4/newtmp/saved-m4-stuff/src/github.com/m4/music`.

After the reboot, sanity-check the toolchain first:
`cd music && make setup` (expect `0 missing`) and confirm `~/bin/fluidsynth`,
`~/bin/ffmpeg`, `~/bin/ps2pdf` still exist and are on PATH.

## Objective
Round 3: fix user feedback on `ims/test-legato` — (a) violin in
`test-legato-sfz.mp4`: the LAST note of the slur "chops and stops abruptly";
(b) slurred-in notes are too quiet on BOTH the fluidsynth (.fs/GM) and SFZ
paths: "The attack reduction has reduced the notes volumes."

Root cause already established (do NOT redo the analysis): hammering the
slur-in *velocity* down was the wrong lever.
- GM/fluidsynth: velocity drives attack AND sustain/body — cutting it to 45%
  starves the note body, not just the onset.
- VPO/sfizz: the slur-in velocity reduction drops the sfizz velocity LAYER
  and with it the sample level (loudness is CC11, but velocity picks the
  layer), which is also what makes the violin's last slur note weak/choppy.
- Perceived level = attack + intensity + volume. Attack reduction must come
  from the CONNECTION (pedal + short overlap on GM, slow-onset swell on VPO),
  not from note loudness.

## Where things stand (end of round 2, all built & measured 2026-09-28 14:37)
- Compilers (both, byte-identical; `diff ims/test-legato.fs ims/test-legato_m.fs`
  = IDENTICAL): `slur_velocity` default **45.0**; `pedal_overlap` **30** ticks;
  slur-in overlap `_ovp = MIDICLICKSPERQUARTER//32` (31.25ms here); CC64 seam
  pedal press `note_time - pedal_overlap_ticks`, release
  `note_time + pedal_overlap_ticks + _ovp`, melodic channels only.
- gcs2sfz: `GCS2SFZ_LEGATO_ATTACK_MS` 150 (VPO slur-in slow-onset split),
  `GCS2SFZ_LEGATO_ATTACK_GAIN_DB` 3.0 (+3dB on the slow slur stem),
  `GCS2SFZ_LEGATO_PEDAL_MS` 60 (CC64 seam, GM fallback only),
  `GCS2SFZ_LEGATO_VEL_FACTOR` default 1.0 (off).
- Measured on the 14:37 mix: piano plain -37.5..-38.6 dB, slur-ins -49.3/-50.6,
  body after pedal lift -52.8; violin slur-ins -34.7/-40.5 with +3dB.
- Docs already updated to round-2 numbers: `DEVIN-2026-09-28-legato-gm-mono.md`,
  `SFZ.md`.

## ROUND-3 IMPLEMENTATION PLAN (designed, NOT coded)
1. Compiler, BOTH files mirrored (`ims/imscomp`, `musicomp2abc/musicomp2abc`):
   - `createglobalvar('slur_velocity', ...)` (~imscomp line 1519): default
     45.0 -> **68.0** (healthy GM body; the pedal+overlap now carry the
     attack-masking, so a milder cut. Vel ~53 from 78).
   - Slur velocity softening (~lines 7053-7066): gate OFF for --sfzpipecsv:
     `if prev_note_slurred and not args.sfzpipecsv:` (args is in scope; see the
     `args.fluidsynth` gates at ~5966/5974). Update the comment: the SFZ CSV
     must carry FULL slur-in velocity so VPO hits full layers; gcs2sfz handles
     attack per-path. Keep 68% on the .fs / midi1csv (GM) output.
   - Do NOT touch the overlap/pedal block (7118-7140); its `Control_c` pedal
     lines are dropped from the CSV by print_out_sfzpipecsv (verified: no CC64
     in test-legato violin/piano CSVs).
2. gcs2sfz `write_midi` (lines ~568-602): the slur-in soften block currently
   multiplies slur-in velocity by GCS2SFZ_LEGATO_VEL_FACTOR (default 1.0=off)
   ON TOP of the CSV's. Once the CSV carries full velocity, the GM fallback
   (pred_expr=False) must soften ITSELF or GM-fallback piano re-attacks; VPO
   (pred_expr=True) must stay untouched (gone is the double-softening trap).
   - Gate the velocity multiplier to `not pred_expr`, rename knob to
     `GCS2SFZ_LEGATO_GM_VEL_FACTOR` (default **0.68**, keep a legacy alias for
     the old name). Apply only in reattack mode, only when <1.0. Note the sfizz
     path (pred_expr=True) slur-ins keep full CSV velocity because the CSV is
     now full; the slow-attack swell is their attack control.
   - Keep the `pedal_ticks and not pred_expr` CC64 seam logic as-is.
3. Rebuild + re-measure:
   ```
   cd music/ims && export PATH="$HOME/bin:$PATH"
   make -B test-legato.mp4 test-legato-sfz.mp4
   diff test-legato.fs test-legato_m.fs   # must stay IDENTICAL
   ```
   Measure (same recipe as before, astats RMS): piano slur-in onsets should
   rise from -49.3/-50.6 toward ~-42..-47 (plain -37.5..-38.6); violin last
   slur note 76 (t~16.0s) should hold a full body instead of chopping; GM
   fallback piano via `/tmp/legato-piano` re-render if needed.
4. Update docs to round-3 numbers (DEVIN-2026-09-28 file + SFZ.md): slur_velocity
   68, sfzpiepecsv full slur-in velocity, `GCS2SFZ_LEGATO_GM_VEL_FACTOR` 0.68.
5. Hand back listening commands:
   `cd music/ims && cat test-legato.fs | fluidsynth -a coreaudio -n -q -K 256 -r96000 /Users/m4/src/GeneralUser/GeneralUser.sf2`
   `open test-legato.mp4 && open test-legato-sfz.mp4`
   Tuning levers if still off: slur_velocity, GCS2SFZ_LEGATO_GM_VEL_FACTOR,
   GCS2SFZ_LEGATO_ATTACK_MS / _ATTACK_GAIN_DB, pedal_overlap,
   GCS2SFZ_LEGATO_PEDAL_MS.

## Key file locations
- `ims/imscomp` (and `musicomp2abc/musicomp2abc` mirror): slur velocity var
  ~1519; soften block 7053-7066; overlap + CC64 seam 7118-7140.
- `music/sfz/gcs2sfz`: `_slur_target_marks` 376, `_slur_slow_attack` 397,
  `_slur_slow_gain_db` 410, write_midi reattack join 548-566 + slur soft/pedal
  block 568-602, `gain_db` in `_render_sfz` ~1000, slur split in
  `_render_instrument_csv` ~1215-1400.
- Docs: `DEVIN-2026-09-27-legato-slur.md` (l semantics, CC68/priming, slur
  fusion), `DEVIN-2026-09-28-legato-gm-mono.md` (mono verdict + rounds 1-2),
  `SFZ.md` (legato/slur section + knobs).

## Regression guardrails
- Baseline: 0 bare ARGH / 2 named (gershwin new-g3, both compilers identically
  failing — not a mirror problem). Full `music/ims/DOALL` takes >10 min and
  exits non-zero on gershwin by design; prefer the fast smoke until the end:
  `b/01/v1-1`, `b/01/v1-2`, `ims/test-legato` x `--vert --hori --midi1csv
  --fluidsynth --abc`, diff per format imscomp vs musicomp2abc (must be 0
  diffs).
- The `pedal_overlap` default also drives the legacy `.gcs` `sustain_pedal`
  feature (its timing shifts if the default is changed — be aware, not a bug).
- SFZ renders use `random.seed`-based humanize/detune only on their own paths;
  compare the SAME bar twice, and never judge ensemble/velocity changes by
  sample-domain A-B diffing (saturated metric, see AGENTS.md).