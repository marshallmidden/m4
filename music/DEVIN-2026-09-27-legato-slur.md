# Legato / slur: compilers + SFZ path (2026-09-27)

Session note for the uncommitted-at-the-time legato work in `imscomp` /
`musicomp2abc` / `gcs2sfz`. Records the semantics that were settled on, why,
and the open questions.

## The `l` suffix, finally pinned down

`imscomp` line 17279 states it: `suffix_legato = 16  # glide to next note - no
note attack on start of next note.` So `l` on note N means **N glides into N+1
with no re-attack at N+1's onset** — it is "slurred *from*". The mark therefore
goes on the notes that *slur*, and **not** on the last note of the run:

```
c4 e4l f4l g4
      ^  ^
```

`e` and `f` slur; `g` is slurred *into* by `f`'s `l`, and `c` is not slurred into
at all (nothing before it has `l`). So the group is `e f g`, `c` and nothing
else is struck fresh at the far end, and `g` is played with no attack.

This is what the pre-2026-09 compilers already did, and it is the semantics to
keep. An earlier draft of this note got it backwards and told you to mark every
note including the last; that variant re-attacks the final note of every slur,
and it also makes `gcs2sfz` fuse the wrong notes (see below).

`ims/test-legato.gcs` is the canonical example: `NT0` and `NT1` carry `l`, so
`NT0..NT2` is the three-note slur and `NT3` is struck fresh.

Much of `b/01/v1-1.gcs` uses the two-note shorthand (`5e2l, 5f8` — `l` on the
first note only), which under this convention *is* a correct two-note slur.

## Compiler changes (both compilers, kept in sync)

### 1. CC68 footswitch span — the audible bug

`imscomp` drives CC68 (MIDI 0x44 `LEGATO_SWITCH`) directly. fluidsynth treats
CC68 as **monophonic** (`fluid_channel_is_playing_mono()`), which dictates the
whole shape of the pedal trace:

- The pedal must be **down at the onset** of every note that follows an `l`
  note. That is the semantics. Releasing it *before* the note-on — as one
  intermediate attempt did, on the theory that a late release leaves the note
  inaudible — silently re-attacks the last note of every slur.
- The pedal must be **re-armed a short way into that note**, not held across it.
  Within one unbroken pedal-down span, fluidsynth routes note-ons to
  `fluid_synth_noteon_monopoly_legato()`, so every note-on after the first
  continues the same decaying voice rather than starting a new one, and the
  notes die away one by one. Measured per-note RMS on a 3-note test:

  | pedal behaviour                | note 1 | note 2 | note 3 |
  |--------------------------------|--------|--------|--------|
  | up (all struck)                | -26.5  | -25.9  | -24.6  |
  | down at onset, up 2 ms in      | -26.6  | -26.1  | -24.9  |
  | held to the note-off           | -26.5  | -29.2  | -50.2  |
  | held across the whole run      | -41.3  | -41.8  | -58.1  |

  So the trace is a short ON/OFF pair per slurred note, which is exactly what
  the pre-2026-09 compiler emitted (its `ON / OFF / ON / OFF` toggle was noisy
  but harmless, and each slurred note-on was the first of its span). The old
  stretch-derived `lastlegatolth` release point is replaced by
  `1 .. MIDICLICKSPERQUARTER/16` ticks (2–31 ms), capped at a quarter of the
  note so it always rings.
- The **previous note's note-off must land before the slurred note's onset.**
  In monophonic mode a note-off for the old key that arrives even one tick
  after the new note-on goes through `fluid_synth_noteoff_mono_LOCAL()` and
  damps the note just started. This is not hypothetical: humanize jitters
  note-ons by up to ±`humanize` ticks while note-offs were measured from the
  un-jittered grid, so the ordering flipped at random — `NT2` came out at
  -56.7 dB in measure 1 of the mini test and -34.0 dB in measure 2 of the
  identical measure 2. Slur note-offs are now pinned to `note_time - 1`
  (≈2 ms, an exact legato join) and measured from the jittered `note_time`
  rather than the grid.

### 2. `l` is a slur, not a duration stretch

- `note_playing = int(note_playing * last_legato)` is **gone**. A slurred note is
  held for exactly its written length; the *next* note's onset supplies the
  sustain. Stretching both sides is what produced overlapping/gappy slurs.
- Legato notes snap to the grid for humanization
  (`... and 'l' not in last_suffixes` on the `note_time` random offset). The
  previous note is held to exactly that tick, so a random offset would make the
  pair overlap or leave a gap.
- Expression snaps at the seam instead of ramping: `_do_vol_ramp` gained
  `and 'l' not in last_suffixes`. A ramp would otherwise be a fake swell inside
  the still-sounding previous note.

### 3. Measure/EOF text lands after the last note

`Text_t, "measure N"` / `"goto m"` moved from `lthworkingmeasures` to
`lthworkingmeasures + last_note_length`, so the overlay caption appears when
the measure's last note *ends* rather than while it is still sounding.

### 4. `--priming` (new flag, both compilers)

`--priming MS` (default 50.0, `0` disables) emits `sleep <ms>` as the very
first `.fs` line. This replaces the old hardcoded priming hack, which did
`noteon 0 60 0 / sleep 2.0 / noteoff 0 60 0` after setup — a 2-second
audible-ish C4 nudge at the head of every render. Rationale for the 50 ms
default is in the flag help: FluidSynth's default output ring is
`audio.period-size 64 * audio.periods 16 = 1024` frames = 23 ms @ 44100, so
50 ms covers two dropped buffers.

### 5. SFZ CSV: `vol_end` no longer reads a legato-seam snap

`print_out_sfzpipecsv` keeps a per-`(voiceon, chan)` CC11 history
(`chan_vol_hist`) and takes `vol_end` from the last CC11 change *strictly
before* the note-off tick. Previously it read `chan_vol` at the note-off tick,
which at a slur seam is the *next* note's dynamic — a fake swell at the end of
every slurred note.

## gcs2sfz: slur emulation by pitch-bend fusion

A sample player cannot re-pitch a sustaining voice, so a slur in the SFZ path
would restart the sample on every note — which is exactly the re-attack that
makes a slur sound detached. `_collapse_legato_groups` instead plays each
slurred run as **one** note-on at the run's first pitch and produces the later
pitches with channel pitch bend gliding into each notated onset, restoring
`base_bend` at the run's note-off. A 20 ms ramp (`GCS2SFZ_LEGATO_GLIDE_MS`)
is laid down just *before* each onset, so the new pitch has arrived by the time
the note "starts".

Guards, all conservative:

- the run **starts** on a note marked `legato` by imscomp (CSV column 10) and
  **extends to the note after it** — `l` is "no attack on the *next* note", so
  the slurred-in target carries no marker. Requiring the target to be marked
  instead runs the group backwards: it would absorb the note *before* the slur
  and drop the slur's last note;
- contiguous (`|cur.start - prev.end| <= 5 ticks`);
- the note the run extends from is not `staccato`/`marcato` (a detached note is
  re-struck, so fusing it would swallow its character). A *slurred-in* target
  may be staccato — `l` governs the attack, the staccato only its length;
- same `staff` column (else it is a different desk, not a continuation);
- every member within `GCS2SFZ_LEGATO_MAX_SEMI` (default 2) of the run's
  **first** pitch — the sample is loaded at that pitch, so the whole run must
  stay inside the sampler's bend range. Measured from the first note, not the
  immediate predecessor, or a stepwise 66→65→63 chain would drift 3 semitones
  and bend out of range;
- **monophonic**: no other note of the instrument sounding for more than
  `GCS2SFZ_LEGATO_OVERLAP_MS` during the run. Pitch bend is channel-wide, so a
  second simultaneous voice would be dragged along with it.

`GCS2SFZ_LEGATO=0` disables the whole thing. It only runs on the sfizz path
(`pred_expr=True`); the GM fallback merges every voice of an instrument onto
one channel, where a channel-wide bend would detune concurrent notes.

## Measured payoff, and why it is small

`b/01/v1-1.gcs`, all 12 instrument CSVs, 2766 legato notes:

`b/01/v1-1.gcs`, all 12 instrument CSVs — 17277 notes, 2766 of them marked
`legato`:

| `GCS2SFZ_LEGATO_MAX_SEMI` | re-strikes eliminated | fused runs | mean run |
|---|---|---|---|
| 2 (default) | 505 (2.9%) | 391 | 2.29 notes |
| 4 | 727 (4.2%) | 453 | 2.60 notes |
| 7 | 830 (4.8%) | 481 | 2.73 notes |
| 12 | 895 (5.2%) | 495 | 2.81 notes |

The bottleneck **is** the bend range. Of the 2478 runs that never got past their
first note, 1745 were refused for exceeding `max_semi`, against 571 for a staff
change, 107 for polyphony and 55 for non-contiguity. Looking only at the 914
pairs that clear contiguity/staff/monophony, the first slurred interval is

| semitones | 1 | 2 | 3 | 4 | 5 | 6–9 | 10 | 12 | 13–14 |
|---|---|---|---|---|---|---|---|---|---|
| pairs | 373 | 262 | 58 | 110 | 32 | 12 | 11 | 50 | 6 |

so 635 of them sit inside the default 2-semitone bend and the rest are mostly
leaps of a fourth (110) and an octave (50). Bumping the default to 4 is the
cheapest real gain (+44%), but the bend is plain resampling, so 4 vs 2 is a
listening call rather than a correctness one — the default is left at 2.

## Verification done

- `ims/DOALL`: **0 bare / 2 named**, the 2 being the known pre-existing
  `gershwin new-g3` failure that both compilers share. No regression.
- `imscomp` and `musicomp2abc` diff is exactly the expected `--sfzpipecsv`
  delta; the legato edits are mirrored in both, and the two compilers emit
  byte-identical CC68/note traces for the same input.
- Minimal repro (`/tmp/mini/m.gcs`, `4c4, 4e4l, 4f4l, 4g4` on acoustic grand):
  per-note RMS -36.2/-35.5/-34.9/-34.0 dB in m1 and -34.9/-36.6/-34.9/-34.0 in
  m2 — no silent notes, in either measure. Before the note-off fix the same
  measure read -56.7 dB on its last note.
- Pedal-trace shape, same piece, measure 1 (ticks):
  `noteoff 76 @503.5` → `cc 68 127` + `noteon 77 @504` → `cc 68 0 @519` →
  `noteoff 77 @741.5` → `cc 68 127` + `noteon 79 @742` → `cc 68 0 @757` →
  `noteoff 79 @960`.
- `ims/test-legato.fs` measure 6 inspected directly: `cc 2 68 127` immediately
  before `noteon 2 74` and before `noteon 2 76` (the two slurred-in notes), and
  **no** CC68 before `noteon 2 77` — `NT3` struck fresh, as it should be.
- `gcs2sfz._collapse_legato_groups` unit-checked directly against
  `c e4l f4l g4`, `a4l b4`, `c d4l e4`, gapped / wide-leap / polyphonic /
  two-separate-slur cases, and fractional-second CSVs (which used to raise
  `TypeError: 'float' object cannot be interpreted as an integer` in the bend
  ramp). All as expected.
- VPO render of `b/01` violin (7 desks, ensemble detune + pan spread):
  -16.1 LUFS / true peak -0.2 dBFS with fusion on, -16.0 / -0.2 with it off
  (`GCS2SFZ_LEGATO=0`) — the fusion does not change level, and sfizz accepts
  the bend stream.
- `musicomp2abc` `make mp4` target: `Aflatmajor.mp4` builds in ~4 s.

## Open questions

1. ~~**The two-note shorthand.**~~ **Resolved.** `5e2l, 5f8` *is* a two-note
   slur under the real semantics, which is what both compilers have always done
   and what the current pedal trace reproduces. Nothing in the scores needs
   changing, and the note-off/pedal interaction that used to make it sound
   broken is fixed (§1).
2. **Is 2.9% worth it?** The fusion is correct but a small effect on this
   repertoire, and the ceiling is set by the bend range rather than by the score.
   `GCS2SFZ_LEGATO_MAX_SEMI=4` roughly doubles it if the samples hold up under a
   4-semitone bend — worth an A/B listen on the `b/01` violin.
3. ~~`test-legato.gcs` had a stray `cstop*`~~ — **fixed, see below.**

## `ims/test-legato.gcs`: the per-instrument audition was silently broken

Commit 07405f4a added this piece as a "per-instrument legato audition", and the
version in the working tree did not do that. Lines 6-37 had been wrapped in
`cstop` … `cstart`, which suspended the whole 28-voice `instrument N <name>`
block, and a `cstop*` at line 86 then halted compilation right after `piano`.
Two consequences:

- only 4 of the expected 56 measures were emitted;
- **every remaining voice fell back to program 0.** All 28 `* violin`,
  `* viola`, … lines are `echo` overlay labels — they do *not* change the
  instrument. The only instrument bindings that survived were voice 1
  (`default`) and voice 2 (`piano`).

So the reel silently auditioned piano 25 times under different labels.

The compression was presumably an attempt to stay under 16 MIDI channels. That
turns out to be unfounded: `set synth.midi-channels 128` is already in the
header, and the 29-voice `.fs` renders fine through fluidsynth (169 s of audio,
exit 0). Verified before changing anything, because "it renders" and "it
renders the right instruments" are different claims.

Fixed by restoring the live `voice 28` / `instrument 1..28` / `volumes` block
and dropping both `cstop*` lines. The header comment now spells out that
`legato_notes` uses the voice number `W` for both the voice and its measure
pair `(2W-1, 2W)`, and that the `* <name>` lines are labels only.

Verified after the fix:

- 56 measures; `select` programs now match the instrument list
  (`2→40` violin, `3→41` viola, `4→42` cello, `5→43` contrabass,
  `6→45` pizzicato, `7→72` piccolo, `8→73` flute, `10→68` oboe,
  `11→71` clarinet, `12→70` bassoon, `13→69` english horn, `14→60` horn,
  `15→56` trumpet, `16→57` trombone, `17→58` tuba, `18→61` brass section,
  `19→47` timpani, drums → 127/14 as percussion).
- violin, measure 6: `noteon 2 72` / `cc 2 68 127` / `noteon 2 74` /
  `noteon 2 76` / `cc 2 68 0` / `noteon 2 77` — three-note slur, then a fresh
  strike. Measure 5 (no `l`) has no CC68 at all.
- `--sfzpipecsv` now writes **28** instrument CSVs (was 2), each with the same
  4-sustain / 3-legato / 1-sustain shape.
- All six output formats still build: `--vert --hori --csv --fluidsynth --abc
  --sfzpipecsv`.
- GM render: 169 s, exit 0, -36.2 LUFS (quiet, as expected for 28 single
  mf bars spread over three minutes).

