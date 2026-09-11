# Devin Session - 2026-09-11 - sfz lead-in/tail timing + waveform loudness scale

## Status: COMPLETED (this session)

Two issues raised by the user while comparing `v1-1-sfz.mp4` (SFZ/VPO path) with
`v1-1_2.mp4` (GM/fluidsynth path) in `b/01` using `MC_ARGS="--measures 0,3"`:

1. **The sfz mp4 had a ~2s pause before the music** and a trailing pause at the end.
2. **Measure 3's last half looked "excessively loud"** in the sfz waveform vs GM —
   user asked for loudness marks (e.g. cyan lines) on the waveform to quantify it.

## 1) The pauses — root cause & fix (`music/music/sfz/gcs2sfz`)

The GM path and the sfz path were NOT time-aligned:

- `gcs2sfz` `mixdown()` prepended `LEAD_IN` (default **1.0s**) of silence via
  `adelay` before the first note, so `*-sfz-mix.wav` started ~1s after `t=0`.
  Measured in `sfz-mix/...-sfz-mix.wav`: `0.0-0.9s` were digital zeros, first
  sound at `1.0s`. The `.fs` overlay timeline (measure labels) starts at `t=0`,
  so in the sfz mp4 the measure text ran **1s ahead of the audio**.
- The observed "2s pause" = 1.0s `LEAD_IN` + the piece's first notes being soft
  `vol(p)` (audible peak ~0.005 until the loud `vol(sfff)` note peaked ~2.0s).
- The end pause = the `TAIL` padding appended after the longest stem
  (`target = LEAD_IN + piece_end + TAIL`, TAIL default 6.0s). For short slices
  (e.g. 8.5s mix whose music ends at ~6s) that leaves 2-2.5s of near-silence.

### Fix
- `LEAD_IN` default changed `"1.0"` → `"0.0"`. The env knob `GCS2SFZ_LEAD_IN`
  remains for opt-in; default **must** stay 0 so the sfz-mix stays time-aligned
  with the `.fs` overlay (a positive lead-in misplaces the scrolling measure text).
- `TAIL` is now env-overridable: `GCS2SFZ_TAIL` (default still `"6.0"`).
- Result: `v1-1-sfz-mix.wav` = 8.499s, first audible sample at 0.00s; GM is 6.82s,
  both begin at t=0 → measure labels and audio align in both.

## 2) Waveform loudness scale (`music/wav2waveform`)

New feature: cyan dBFS reference scale on the scrolling waveform (used by
`gcs2youtube` for BOTH GM and sfz mp4s).

- CLI: `--db-markers "…"` (comma list of dBFS levels; default `-6,-12,-18,-24`;
  empty string disables).
- Renders static dashed cyan horizontal lines symmetric about the centre at
  amplitude `10^(db/20)`, 2 px thick so they survive H.264 4:2:0 chroma
  subsampling (1 px lines come out heavily diluted), plus small `-6dB`-style
  labels drawn with a built-in 5x7 bitmap font (no PIL dependency; script already
  has numpy via `uv run --script`).
- Implementation notes: the dB mask is a `(H,W)` float built ONCE before the
  frame loop (not per frame — the per-column x-loop would be too slow), applied
  additively per frame as `G=max(g,cyan)`, `B=cyan`. Dashes are 3-on/9-off.
- Verified in `b/01/v1-1-sfz.mp4`: all 8 line rows (135/203/236/253 top,
  405/337/304/287 bottom rel. to the 540-row quadrant centred at 270) + labels
  present in the final 1920x1080 encode.

## 3) "Is measure 3's last half really louder in sfz?" — measurement

Slice `MC_ARGS="--measures 0,3"` (note: in the slice, measure 3 is re-based to
start at t=0; measure 0 is the empty `for yaps -k` preroll). Measure 3 spans
0-5.45s; its last audible half is ~2.7-4.1s.

RMS/peak (after the LEAD_IN alignment fix, both files start at t=0):

- Last half of m3 (2.70-4.20s): **sfz RMS 0.051 / peak 0.259** vs
  **GM RMS 0.046 / peak 0.199** ≈ +1.1x RMS (~+1 dB), +1.3x peak.
- Full measure 3: sfz peak 0.259 vs GM peak 0.260 — essentially identical.

Conclusion: NOT "excessively loud". Most of the visible difference was the
+1s label-vs-audio misalignment (fixed in #1) plus soundfont character
(VPO samples vs GeneralUser.sf2 GM), i.e. ~1-2 dB — not a loudness bug.

## Files changed this session

- `music/music/sfz/gcs2sfz` — LEAD_IN default 0; `GCS2SFZ_TAIL` env knob.
- `music/wav2waveform` — cyan dBFS loudness scale (`--db-markers`).
- `AGENTS.md` (repo root) — SFZ pipeline section updated with the env knobs and
  waveform marker note.
- Build artifacts regenerated in `b/01` only: `v1-1_2.mp4` (GM 6.82s),
  `v1-1-sfz.mp4` + `sfz-mix/v1-1-sfz-mix.wav` (8.499s, aligned).

No compiler changes — the `ims/imscomp` ↔ `musicomp2abc/musicomp2abc` DOALL
mirror constraint is untouched (gcs2sfz and wav2waveform are pipeline scripts,
not compilers).

## To resume

- If the end-silence is still objectionable on a FULL piece, shrink the tail per
  piece: `GCS2SFZ_TAIL=2.5 make v1-1-sfz-mix.wav` (or similar) and re-check the
  ring-out isn't chopped (`trim_wav` targets `LEAD_IN + piece_end + TAIL`).
- Loudness-mark style tweaks (more/other dB levels, brightness, dash period) are
  all in `wav2waveform`'s dB-mask block / `--db-markers`.