===============================================================================
DEVIN-2026-09-19-sfz-ensemble.md -- ensemble / stage simulation (detune + pan)
===============================================================================

Status: DONE. Committed with the SFZ.md recreation (badgering none). Resolves
the "orchestra on stage" request: each staff duplicated inside an ensemble is
a DESK. Each desk is internally in tune (its own channel), tuned slightly
differently from its neighbors (never two desks on the same tuning), and
panned slightly differently -- so a violin section of 7 desks reads as seven
players in different spots, not one voice in one seat, and two violin desks
sound like two violins rather than one louder one.

Design rule (the user's physics): a single violin is precisely tuned -- all
its strings against each other. A second violin tunes its strings against
each other too, but its reference is slightly different from the first
violin's; so each DESK of a section carries its own small tuning offset.
Pan must also differ per desk -- the desks can't stand on top of each other.

Context
-------
- Pre-feature the sfz CSV merges EVERY voice of an instrument onto one MIDI
  channel (print_out_sfzpipecsv by_instrument roll-up) and every voice is
  tuned identically, so a 12-violin section sounds like one voice. Pan was
  already per-part from the score (CC10), but all desks of an instrument's
  section carried the SAME pan (e.g. every violin desk 36).
- Pitch bend is CHANNEL-wide (fluidsynth/sfizz), range via RPN 0 default
  +/-2 semitones (200 cents, 8192 = center). It also persists per channel if
  not re-zeroed. This shapes the design: every desk stream renders on its
  OWN file/channel, so a bend applies to that whole stream (the desk), which
  is exactly "strings tuned against each other, reference slightly off".

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
   - Per-desk detune: the deterministic permutation of -R..+R (R =
     GCS2SFZ_ENSEMBLE_DETUNE, default 4) sorted by crc32(name|v), assigned
     to the staffs in order -- guarantees distinct desk tunings up to
     2R+1 = 9 desks (b/01's pizz section is 9). One desk lands on 0 (the
     tuning reference). No two desks share a tuning (the "why have two
     violins" trap).
   - pan_off per staff = even fan across +/-GCS2SFZ_ENSEMBLE_PAN (default
     10) around the score pans, + a small crc32 jitter per staff.
5. render_instrument(): if the plan has >=2 staffs, each staff is written to
   its own sub-CSV and rendered by the SAME articulation-split renderer
   (_render_instrument_csv, extracted from the old inline block), then amixed
   into the instrument stem; reverb bake unchanged (uses the original CSV).
   Single-staff and kit/one-shot instruments take the old path entirely.
6. render_gm(): the fluidsynth fallback gets the same treatment via
   _render_gm_csv (pred_expr=False, program change, GCS2SFZ_GM_GAIN intact).

Verification
------------
- Plan output (b/01 v1-1): violin 7 desks detune [+4,+0,-3,-2,+1,+3,-4] cent,
  pan fan [-9,-8,-3,+1,+2,+8,+11] around base 36; viola 3; winds 2 each;
  pizzicato_strings 9 desks all distinct; cello/contrabass/timpani -> None
  (single-staff, score pan verbatim). Distinction guaranteed by permutation.
- Full `make sfz-mp4` chain with the new code (CSV -> WAV -> overlayed mp4):
  b/01 v1-1..v1-4 and b/09 b9m1..b9m3, all EXIT 0. Re-rendered b/01 after the
  per-desk change (the first pass had the earlier shared-detune behavior).
- Legacy 9-col and 10-col (no staff) CSVs render as before; GCS2SFZ_ENSEMBLE=0
  reproduces the pre-feature single-stream output.
- DOALL: 0 bare / 2 pre-existing named (new-g3 both compilers). WAVs get
  wiped by DOALL's b-suite `make clean` (sfz-clean) -- expected; re-run
  `make sfz`/`make sfz-mp4` for fresh audio.

Notes
-----
- History: the FIRST implementation gave every desk of an ensemble the SAME
  detune (crc32(name)%17-8), which the user correctly rejected -- with
  identical tuning two violin desks are just one louder violin. The spec was
  amended in-session (this note supersedes the original "shared detune"
  description the first commit used).
- Env knobs now: GCS2SFZ_ENSEMBLE (on/off), GCS2SFZ_ENSEMBLE_DETUNE (default
  4, +/- cents), GCS2SFZ_ENSEMBLE_PAN (default 10, +/- pan units).

Follow-ups (open, in SFZ.md status)
-----------------------------------
- A/B-listen detune/pan choices across the full suite (130 mixes + 130
  sfz-mp4s) and tune to taste via the two knobs.
- Calibrate instruments.LEVEL_VELOCITY (velocity-map named dynamics).
- ppp/pppp residuals from the 0.25s measurement window.