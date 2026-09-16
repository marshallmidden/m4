# DEVIN — CC11 expression predistortion (sfizz vs GM loudness) — 2026-09-15

Worktree: `/Users/m4/newtmp/saved-m4-stuff/src/github.com/m4/music/music/sfz/gcs2sfz`
(gcs2sfz === gcs2sfz; also spellings `saved-m4-stuff`/`gcs2sfz` exist on disk
and several of them DID NOT track the renderer edits this session — the
tool layer reported different states per spelling, which is why this dragged
on. Before trusting any edit, `git show HEAD:music/.../gcs2sfz` + a fresh
per-spelling grep of the *emission* lines.)

## Root cause (measured, controlled)

sfizz (via the VPO wrapper `amplitude_oncc11=100`) applies CC11 expression
LINEARLY in amplitude (`amplitude ∝ CC11`). GM/fluidsynth (GeneralUser)
applies CC11 expression exponentially. So the same `vol_start..vol_end` ramp
renders:

  CC11 90->60  =  -2.5 dB (sfizz)   vs   -8..-9 dB (GM)
  b/01 horn fp->p (91->61) = 0.4 dB (sfizz)  vs  ~9 dB (GM)

Pizz velocity layers DO respond (that part of the contrast is velocity, not
expression) — only expression/C11 is flat in the sfizz path.

## Fix (shipped a helper + env knob)

Predistort the emitted CC11 value as `127*(v/127)^E` before write, with
`GCS2SFZ_EXPR_EXP` (float, DEFAULT 4.0; 1.0 = raw linear). Measured on the
b/01 horn CSV `vol 90->60` (20*log10 RMS of the horn stem beat1 vs beat5):

  E=1.0 -> 2.6 dB     E=2.0 -> 4.0 dB
  E=2.5 -> 4.7 dB     E=4.0 -> 6.7 dB     E=6.0 -> 8.4 dB

GM sits ~-9 dB, so E in {4..6} fully closes the gap. Verified on the b/01
`v` slice (2026-09-16): full-mix beat1-vs-beat5 GM `v_2.fs` = 12.52 dB,
SFZ E=2.0 = 6.78 dB, SFZ E=4.0 = 12.19 dB — so the default was raised to
4.0 to match GM. Only CC11 is reshaped — velocity, CC10 pan, CC91 reverb
untouched (VPO velocity-layer patches keep timbral response).

SCOPING (2026-09-16): `write_midi` is shared by the sfizz path AND the GM
fallback (`render_gm`, piano/`bell_tower`). GM/fluidsynth applies CC11
exponentially too, so predistorting the GM path would double-compress it.
The fix adds `pred_expr=True` to `write_midi` (default = sfizz path);
`render_gm` passes `pred_expr=False` so GM-fallback instruments keep their
native exponential response. Full suite re-rendered at E=4.0 after this.

## Wiring checklist (verify on the exact disk file before shipping)

1. helper once: `def _pred(v)` (or `_expr`) predistorting `127*(v/127)^exp`
   with `GCS2SFZ_EXPR_EXP`, `from math import pow as _pow`.  (currently
   DUPLICATED + some copies never defined — dedupe.)
2. note-on CC11: `append((11, _pred(vol_s)))`  (+ `last_vol = vol_s`)
3. in-note ramp: `interp_ramp(on, off, _pred(vol_s), _pred(vol_e), 11)`

Sites that were found RAW on the last verified reading of the canonical
spelling (line ~300): `interp_ramp(on, off, vol_s, vol_e, 11)` then
`last_vol = vol_e`.

## GM-fallback stutter fix (2026-09-16) — CC11 cross-voice pumping

Follow-up: even with pred_expr scoping, the GM fallback still "stuttered"
audibly ~26s in on Chopin piano. Root cause is channel merging, not
predistortion: the `.fs` uses a SEPARATE MIDI channel per voice (0-4), so each
voice's CC11 is independent; the sfz CSV merges every voice of one instrument
onto a single MIDI channel (0). `write_midi` emitted CC11 (vol_start) before
EVERY note-on, and with alternating voices (accompaniment ~60 vs melody ~100)
CC11 toggled every ~90ms, pumping ALL sustained notes on channel 0.

Verified clean (2026-09-16): the mp4 audio has no clipping and no dropped
frames — the "stutter" is purely this volume pumping. HEAD and current
`gcs2sfz` produce byte-identical GM audio (max sample diff 0), so it predates
all expression work.

Fix: on the GM path (`pred_expr=False`) skip per-note CC11 emission entirely —
per-note dynamics already ride on note velocity; only CC10 pan sweeps, CC91
reverb, and in-note cresc/dim ramps are retained. Sfizz path unchanged.
Measured after fix: Chopin 25.5-30s envelope goes from rapid ±8dB toggles to a
smooth -38->-48dB decay (old `chopin_piano.wav` vs `fix.wav`; the .fs
reference rides ~-28..-31 dB, a separate pre-existing level offset).

Re-rendered the full suite (all 130 mixes + 130 sfz-mp4s, 0 errors); DOALL
regression clean (0 bare ARGHs; 2 gershwin new-g3 named ARGHs, pre-existing).

## Memo

All 130 sfz-mix WAVs are re-rendered whenever `gcs2sfz` changes, but the
Makefile rule `sfz-mix/%-sfz-mix.wav: sfz-csv/%/.done` does NOT depend on
`../music/sfz/gcs2sfz` (unlike the mp4 rule which lists GCS2YOUTUBE etc.).
Force a re-render by deleting the sfz-mix/*.wav files first; otherwise
`make sfz` reports "Nothing to be done".
