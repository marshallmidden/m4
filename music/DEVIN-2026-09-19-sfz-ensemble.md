===============================================================================
DEVIN-2026-09-19-sfz-ensemble.md -- ensemble / stage simulation (detune + pan)
===============================================================================

Status: DONE. Committed with the SFZ.md recreation (badgering none). Resolves
the "orchestra on stage" request: each staff duplicated inside an ensemble is
slightly detuned (all staffs of the ensemble share ONE detune) and panned
slightly differently, so a violin section of 7 desks no longer sits in one
seat at one tuning.

Context
-------
- Pre-feature the sfz CSV merges EVERY voice of an instrument onto one MIDI
  channel (print_out_sfzpipecsv by_instrument roll-up) and every voice is
  tuned identically, so a 12-violin section sounds like one voice. Pan was
  already per-part from the score (CC10), but all desks of an instrument's
  section carried the SAME pan (e.g. every violin desk 36).
- Pitch bend is CHANNEL-wide (fluidsynth/sfizz), range via RPN 0 default
  +/-2 semitones (200 cents, 8192 = center). It also persists per channel if
  not re-zeroed. This shapes the design: every bank/staff stream renders on
  its OWN file/channel, so a bend applies to that whole stream safely.

Design (implemented)
--------------------
1. imscomp --sfzpipecsv appends an 11th CSV column = the 0-based voice
   number (staff id). Gated: print_out_sfzpipecsv runs only under
   --sfzpipecsv; the 5 DOALL formats are untouched (0 bare).
   - pending_notes tuples unchanged; release_sfz_note() adds `voiceon` to the
     notes tuple; the writer emits `...,artic,staff`.
2. gcs2sfz read_csv_notes() now returns 11-tuples (staff '' fallback for
   legacy 9/10-col CSVs -> single-stream render, byte-compatible).
3. write_midi() gains `detune_cents` and `pan_off`:
   - detune: raw = clamp(round(8192 + cents*8192/200)) bend event at tick 0
     (before any note), plus a trailing re-zero 8192 at the very end.
   - pan_off: shifts every note's pan AND pan_end (clamped 0..127).
4. _ensemble_plan(name, notes): deterministically decides the split.
   - Disabled by GCS2SFZ_ENSEMBLE=0 (or any '' staff, or <2 staffs).
   - detune = crc32(name) % 17 - 8   (+/-8 cents, shared by all staffs).
   - pan_off per staff = even fan across +/-10 around the score pans, + a
     small crc32 jitter per staff.
5. render_instrument(): if the plan has >=2 staffs, each staff is written to
   its own sub-CSV and rendered by the SAME articulation-split renderer
   (_render_instrument_csv, extracted from the old inline block), then amixed
   into the instrument stem; reverb bake unchanged (uses the original CSV).
   Single-staff and kit/one-shot instruments take the old path entirely.
6. render_gm(): the fluidsynth fallback gets the same ensemble treatment via
   _render_gm_csv (pred_expr=False, program change, GCS2SFZ_GM_GAIN intact).

Verification
------------
- 11-col CSVs: b/01 v1-1 violin spans 7 staffs (14..20), all pan 36.
- write_midi byte check: plain file has NO 0xE0 bend; detuned (-4 cent / pan
  off -9) has `E0 5C 3E` (=8028) at tick 0, re-zero `E0 00 40` at the end,
  and CC10 pan bytes 36 -> 27.
- Full b/01 `make -B sfz`: every movement rendered (EXIT 0, 12/12/11/11
  instruments). 32 ensemble actions, e.g. violin 7 staffs -4 cent fan
  [-9,-8,-3,+1,+2,+8,+11]; viola 3 staffs -7; winds 2 staffs each; pizz
  strings 9 staffs +3. Detune constant per name across all 4 pieces.
  cello/contrabass single-staff -> untouched (score pan verbatim).
- Legacy 9-col and 10-col (no staff) CSVs render as before; GCS2SFZ_ENSEMBLE=0
  reproduces the pre-feature single-stream output (1135+4067 on v1-1 violin).
- DOALL: 0 bare / 2 pre-existing named (new-g3 both compilers). The mixed WAVs
  got wiped by DOALL's b-suite `make clean` (sfz-clean) -- expected; re-run
  `make sfz`/`make sfz-mp4` for fresh audio.

Follow-ups (open, in SFZ.md status)
-----------------------------------
- A/B-listen detune magnitude / pan spread across the full suite (130 mixes +
  130 sfz-mp4s) and tune to taste.
- Calibrate instruments.LEVEL_VELOCITY (velocity-map named dynamics).
- ppp/pppp residuals from the 0.25s measurement window.