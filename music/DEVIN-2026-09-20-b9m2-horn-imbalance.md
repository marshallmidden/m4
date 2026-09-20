# b/09 b9m2: horns overwhelm measures 51-58 (RESOLVED 2026-09-20)

Date: 2026-09-20. Status: RESOLVED. The SFZ feature (`gcs2sfz` +
`imscomp --sfzpipecsv`) is the current render path.

## Resolution (b9m2.gcs m45 edits, committed + pushed this session)

The imbalance was a score/dynamics mistake, confirmed by CSV analysis rather
than the initial suspicion's simplest form. The fix (7 changed lines in the
`measure 45` block):

- m45 lead-ins: moved `vol<` to the HEAD of each line (`vol< r4 r4 ...`) so the
  crescendo registers at the measure boundary instead of 2 beats in.
- `CClarinetV8` + `HorninDV14`: `vol(p)<` -> `vol<` (match the pp base everyone
  else crescs from).
- `HorninBV16`: bare `r2d` -> `vol< r2d` (give the 2nd horn its own swell).
- `TimpaniV20`: bare `r2d` -> `vol(pp)< r2d`.
- m46: dropped the redundant `vol<` on `HorninBV16`.

Result (verified in rendered CSVs + `make sfz-mp4`): the sustained horn/clarinet
swell now arrives AT m57's `ff` together with the orchestra instead of floating
at ~ff from ~m49; horns no longer overwhelm mm 51-58. Music "sounds exactly
fine" per the user.

## The report (from the user, after listening to `b/09/b9m2-sfz.mp4`)

> "In measures 51 to 58 the horns overwhelm the other instruments. Otherwise,
> sounding quite nice! Please commit, push, and then see if the horns are
> understandable. Perhaps the b9m2.gcs has a mistake in the volume levels?"

Commit/push was a no-op (already clean at 18fdeb69). Everything below is the
horn diagnosis.

## Score facts (b9m2.gcs, first pass, scherzo; measure markers via `grep -n "^measure"`)

- m45 block: lines 710-724. **The suspicion: `vol(p)<` on horns + clarinet only.**
  - 711 FluteV1:  r4 r4 `vol<` [4a4,3a4]
  - 712 OboeV5:  `vol<` 4c+4 4d4 4e4
  - 713 CClarinetV8: `vol(p)<` [3a2dt 2a2dt]      <-- one of two `vol(p)<` lines
  - 714 BassoonV10: r4 r4 `vol<` [2g4,2e4]
  - 715 HorninDV14: `vol(p)<` [2g2dt 3g2dt]       <-- one of two `vol(p)<` lines
  - 716 HorninBV16: r2d
  - 717 DTrumpetV18: `vol<` [2g4 3g4] r4 r4
  - 719-723 strings: all bare `vol<`.
- m46-m56 (lines 725-890): Flute/Oboe/Bassoon/DTrumpet/Violin/Bass rebound the
  short motif every bar; **HorninDV14 sustains the same [2g2dt 3g2dt] every bar
  from m45 straight through m56** (continuous held G); CClarinet sustains
  likewise; HorninBV16 alternates.
- m57 (line 892): EVERYONE `vol(ff)` — full-tutti arrival (HorninDV14/BV16 go to
  repeated quarter figure [3g4 4g4]).
- So the horns/clarinet carry ONE long sustained swell from `vol(p)<` at m45
  while the rest accelerate the short figure from pp.

## Rendered-CSV evidence (sfz-csv/b9m2/*.csv, 11 columns:
start_sec,dur_sec,midi_note,velocity,pan,reverb,vol_start,vol_end,pan_end,artic,staff)

- `french_horn.csv` (staffs 9-12 = HorninDV14 x2 + HorninBV16 x2):
  - m45-48-ish: the long sustained note starts t=96.21, dur 6.21, vol 59->101
    (a 3-bar crescendo to ~ff, then it STAYS loud).
  - t~88-102: horn vol mostly 88-102 continuously; continues ~88-102 through
    t~128. i.e. horns are effectively at ff for the whole m46-56 build-up.
- Contrast instruments in the same t window (94-120s):
  - bassoon.csv: vol ~58-62 (pp-ish) except one ff-ish 79-82 right at t~115.34 (m57 arrival).
  - flute.csv: vol ~58-62, brief ~81 during the ff.
  - violin/trumpet: printed NO sustained (dur>=0.45) events in 94-120s window
    (busy 8ths/16ths). **The per-event comparison there was never completed —
    rerun without the dur filter before deciding.**
- Interpretation: horns ramp `vol(p)<` -> ~101 by ~m49 and float there, while
  the orchestra stays ~58-62 until the m57 `ff`. That is exactly the "horns
  overwhelm measures 51-58" imbalance.

## Working hypothesis (the b9m2.gcs "mistake")

m45 marked `vol(p)<` ONLY on HorninDV14 (line 715) and CClarinetV8 (line 713);
every other instrument got bare `vol<`. The pp-based `vol<` stayed quiet
(58-62) through the build, but the horns/clarinet started from p and on their
long sustained notes the crescendo ran all the way to ~ff and persisted.

Diagnosis (partly candidate, partly confirmed): change the two `vol(p)<` ->
`vol<`, AND place each m45 `vol<` at the head of the line (before the rest) so
the swell starts at the measure boundary, AND give HorninBV16 + Timpani their
own lead-ins. Net effect: the sustained swell arrives AT m57's `ff` together
with the orchestra instead of being already-ff from ~m49. Verified in rendered
sfz CSVs + `make sfz-mp4`; user confirms it now sounds right.

## Re-verify commands

- `PATH="$HOME/bin:$PATH" make sfz-mp4` in `music/b/09` (renders b9m1..b9m3).
- Inspect CSVs with `importlib.machinery.SourceFileLoader('g','music/sfz/gcs2sfz')`
  + `g.read_csv_notes(...)`; or plain grep/awk of `sfz-csv/b9m2/french_horn.csv`.
- HORN voicing: HorninDV14 pair = staffs 9,10 (CSV); HorninBV16 pair = 11,12.

## Open items / caveats (don't get distracted)

- fs2ass `time_ms` accounting: fs2ass parse_fs_echoes does `time_ms += sleep`
  then yields `/1000`, i.e. it treats fs `sleep` values as ms — but the fs
  `sleep 0.718` etc. look like real seconds (quarter ~0.69s), so fs2ass-derived
  measure->audio-time mapping is suspect. Also summing `.fs` `sleep`s linearly
  gave 25346s by first-pass m51 (fs is NOT a simply cumulative timeline; it has
  goto/repeat passes). Do NOT rely on fs2ass times for measure mapping; the
  rendered timeline (sfz-mix = 856.755s) + score measure-marker lines are the
  safe anchors.
- violin/trumpet dynamics comparison in the 94-120s window still unquantified
  (need all durations, not just <0.45s filter).
- Minor untracked blemish (not the report): b/01 v1-1 pizzicato_strings at 9
  desks gets pan duplicates [-9,-6,-4,-4,+0,+4,+4,+6,+11]; tunings all distinct.
- If the `vol(p)<`->`vol<` theory is wrong, next suspects: svz per-instrument
  gain (GCS2SFZ_GAIN) or french_horn desk count doubling the section.