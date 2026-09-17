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

## Shipped (commit 4d7d486c, pushed)

The fix is now in `gcs2sfz` on the canonical disk file only (worktree
`/Users/m4/newtmp/saved-m4-stuff/...`); the old duplicated-copy confusion is
resolved — there is ONE gcs2sfz and it is the renderer the piece Makefiles
reference. Implementation in `write_midi`:

- `pred_expr` flag (default True = sfizz path): predistort note-on CC11 with
  `_pred(vol_s)` and the in-note ramp with `_pred(vol_s)`/`_pred(vol_e)` where
  `_pred(v) = 127*(v/127)^GCS2SFZ_EXPR_EXP` (default 4.0), defined once near
  the `interp_ramp` helper.
- `pred_expr=False` (render_gm): skip per-note CC11 (`vol_s != last_vol`
  emission AND `last_vol` tracking) — velocity carries note dynamics; keep CC10
  pan sweeps, CC91 reverb, and in-note cresc/dim ramps.

All 130 sfz-mix WAVs + 130 sfz-mp4s re-rendered with the default at E=4.0;
DOALL regression clean (0 bare ARGHs; 2 gershwin new-g3 named ARGHs,
pre-existing).

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

## GM-fallback loudness sync + expression-in-velocity (2026-09-16) — a834d9ee

Two follow-up fixes on the GM fallback, both calibrated on a new
`ims/test-volume-levels.gcs` (12 eighth notes: mp..pppppp then mf..fffff,
pitches 60/62/64/65/67/69; `Makefile`: `SONGS += test-volume-levels`; committed
with `git add -f` because the ims/ `.gitignore`'s bare `*` would ignore it).

### 1. fluidsynth master gain was 0.2, not 0.5

`render_gm()` ran `fluidsynth -q -F out.wav sf2 midi` with no `-g`. The `.fs`
render emits `synth.gain 0.5` (imscomp), so every GM-fallback stem (default
piano, bell tower, ...) came out 20*log10(0.2/0.5) = **-7.96 dB** quiet — this
WAS the "~10 dB below the .fs" item in SFZ.md NEXT FIXES (its "-37 vs -28..-31
dBFS" numbers). Fix: `-g` from `GCS2SFZ_GM_GAIN` (default 0.5), documented in
`gcs2sfz`'s env-knob docstring.

### 2. Skipped per-note CC11 also skipped the expression

On the fallback, dynamics ride on velocity alone (CC11 stays 127). vol(mp) is
vel 65 + expr 68 in the `.fs`, but the fallback played it at full expression →
mp far LOUDER than the .fs; conversely vol(ppppp) (expr 0) stayed clearly
audible. Fix: fold the expression into the note-on velocity as
`vel * (vol/127)^GCS2SFZ_GM_EXPR` (default **1.3**), enforced per-note in
`write_midi(pred_expr=False)` (no cross-voice pumping risk — velocity is
per-note). Calibration notes:

- GCS2SFZ_GM_EXPR=1.3 on test-volume-levels: per-note RMS vs the .fs render
  (sustained 0.37 s window) — mp -1.0, p -0.8, pp -1.1, ppp -2.5, pppp +1.5,
  mf -3.6, f -2.7, ff -3.6, fff -4.8, ffff -2.2, fffff -2.1 dB (sfz - GM).
  The ~-2..-5 dB residual is the .fs's global synth reverb, which the CSV path
  intentionally omits. ppppp floors at velocity 1 (silent) regardless of E.
- E=1.0-1.4 sweep (mean abs err 4.05 dB at 1.4, 6.32 at 1.7); user ears picked
  1.3 so pppp stays audible (+1.5 dB).
- GeneralUser velocity calibration quirks: v=100 renders SILENT (a crossfade
  dead zone; v95=-28, v104=-27), and v105/v110 read near-silent in one first
  pass (window artifact, re-verified loud). Keep low velocities well below 104.
- Resulting mp4 levels: test-volume-levels-sfz.mp4 -32.4/-13.9 vs test-volume-
  levels.mp4 -31.4/-13.6 dB (mean/max). Nothing else re-rendered — the fallback
  only affects GM-mapped instruments, and notes at vol=127 keep their verbatim
  velocity, so loud passages are unchanged.
Makefile rule `sfz-mix/%-sfz-mix.wav: sfz-csv/%/.done` does NOT depend on
`../music/sfz/gcs2sfz` (unlike the mp4 rule which lists GCS2YOUTUBE etc.).
Force a re-render by deleting the sfz-mix/*.wav files first; otherwise
`make sfz` reports "Nothing to be done".
