# Sustained-instrument slur-in level (violin probe)

Status: **shipped** in both compilers (mirrored).  Follows on from
`DEVIN-2026-10-05-legato-piano-velocity.md`.

## Complaint

`ims/test-legato.gcs` was switched to a **violin** (`instrument 1 violin`) over
all ten dynamics (ffff..pppp); every legato note read as *way too quiet* next to
the freshly struck note in the same measure.

## Measurement (GeneralUser violin, per-note RMS, slur-in body vs fresh)

Offsets: each dynamic is 2 measures, fresh measure then slur measure; body window
= onset+0.25..onset+0.85 s.

| dynamic | 55/0.65 (piano default) | 76/0.9 | **100/1.0 (shipped)** |
|---|---|---|---|
| FFFF | −14.6 / −16.4 dB | −5.7 / −6.9 | ~0 / ~0 |
| MP   | −14.5 / −16.4 | −5.5 / −6.7 | ~0 / ~0 |
| PP   | −15.0 / −15.7 | −6.7 / −6.4 | ~0 / ~0 |
| PPP  | −14.3 / −16.5 | −3.4 / −6.5 | ~0 / ~0 |
| PPPP | −9.7 / −11.8  | −5.4 / −4.4 | ~0 / ~0 |

Velocity calibration on the GeneralUser violin: 55/0.65 (≈34% effective) →
−14.3/−15.0 dB; 75/0.9 (≈66%) → −5.9/−6.7; 90/0.9 (≈79%) → −3.4/−4.2.

## Root cause

`slur_velocity=55` + `slur_velocity_decay=0.65` were calibrated on the **piano**,
a struck string that decays, so a slur-in lands on the source's decayed seam.  A
bowed string (and a wind/brass note) *holds* its level while sounding, so its
slur-in is a **continuation**, not a seam — dipping it ~15 dB is wrong.

## Fix (both compilers, identical)

- New globalvars `sustained_slur_velocity` (default **100**) and
  `sustained_slur_decay` (default **1.0**), read alongside `_slur_velocity`.
- New `sustained_slur_instruments` frozenset (bowed strings, winds, brass,
  voices — real `instruments` dict keys; piano/plucked/perc excluded).
- The slur block picks the sustained profile when the voice's instrument is in
  that set, then applies the adaptive decay with the sustained decay value.
  `100` + `1.0` = a full-velocity slur-in that holds at any source length.
- Piano/default keeps `55` + `0.65` **byte-identical** (verified: ffff 41/42,
  mf 30/27, mp 24/25, pp 20/22, ppp/pppp floor 20).
- imscomp `--sfzpipecsv` dispatch additionally pins
  `sustained_slur_velocity=45` / `sustained_slur_decay=1.0`, so the shared SFZ
  CSV is unchanged (verified: `sfz-csv/test-legato/violin.csv` slur-ins 52/53,
  flat 45) — **never change that pin without re-verifying `gcs2sfz`**.

## Verification

- Mirror: `test-legato.E`, `b/01/v1-1.E`, `b/01/v1-2.E` produce byte-identical
  `.fs` under imscomp and musicomp2abc.
- DOALL: **0 bare / 2 named** (both named = the pre-existing gershwin
  `new-g3.E`, both compilers) — baseline, no regression.
- `make test-legato.mp4` (GM) rebuilt for listening.

## Open / next

- **SFZ/VPO path** still renders the violin slur-in ~10.5..12.5 dB under fresh
  (VPO slow-attack swell, `GCS2SFZ_LEGATO_ATTACK_MS=150` + slow stem amix).  Next
  job: a per-instrument slur-in velocity boost on the slow slur stem in
  `gcs2sfz` (sustained ≈1.55-1.6 → 45%→~70%), calibrated to ~−5 dB, keeping the
  +3 dB slow-stem gain and the pinned 45% CSV.  The GM fix above must land first
  (this note).
- Winds/brass share the sustained profile but have only been validated on the
  violin — split them out if they sound wrong.
