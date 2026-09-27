# Legato / slur: compilers + SFZ path (2026-09-27)

Session note for the uncommitted-at-the-time legato work in `imscomp` /
`musicomp2abc` / `gcs2sfz`. Records the semantics that were settled on, why,
and the open questions.

## The `l` suffix, finally pinned down

`imscomp` line 17279 states it: `suffix_legato = 16  # glide to next note - no
note attack on start of next note.` So `l` on note N means **N glides into N+1
with no re-attack at N+1's onset** — it is "slurred *from*".

The practical convention that falls out, and the one the test piece uses, is
**mark every note of the slur with `l`, including the last**:

```
v W: NT0 NT1 NT2 NT3   r4        NT0..NT2 carry ;l,1
```

That makes the slurred group = the maximal run of *consecutive* `l` notes, and
the first non-`l` note after it is struck fresh. `ims/test-legato.gcs` is the
canonical example: `NT0..NT2` are `l`, `NT3` is bare, and the `.fs` shows
CC68 down across the three notes and released at `NT3`.

Note that much of `b/01/v1-1.gcs` uses the two-note shorthand
(`5e2l, 5f8` — `l` on the first note only). Under the convention above that is
*not* a two-note slur, it is a legato marking on `5e` with `5f` re-struck. See
"Open questions".

## Compiler changes (both compilers, kept in sync)

### 1. CC68 footswitch ordering — the audible bug

Before: the release (CC68 = 0) was emitted at `note_time + lastlegatolth`, i.e.
*after* the following un-slurred note-on. A synth with the pedal still down
reads that note-on as a legato continuation (no hammer strike), and the
note's own note-off lands a tick later and damps it at once — **the note is
inaudible.**

Now the release is emitted at `note_time`, ahead of the `HERE16`/`HERE20`
note-on inserts so it sorts first at the same tick. It is also no longer a
toggle: one CC68=127 at the start of a slurred group, one CC68=0 at its end.

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

- contiguous (`|cur.start - prev.end| <= 5 ticks`);
- every continuation note marked `legato` by imscomp (CSV column 10);
- predecessor is not `staccato`/`marcato` (a detached note is re-struck, so
  fusing it would swallow its character);
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

| `GCS2SFZ_LEGATO_MAX_SEMI` | re-strikes eliminated |
|---|---|
| 2 (default) | 226 (8%) |
| 4 | 358 (13%) |
| 7 | 371 (13%) |
| 12 | 408 (15%) |

Raising the bend range barely helps, so the range is **not** the bottleneck —
the bottleneck is the score's marking convention. Of the ~16660 run starts,
most are a single `l` note followed by a non-`l` note (no chain to fuse), and
`not-contiguous` / `staff-change` account for only ~1900 breaks between them.
See "Open questions".

## Verification done

- `ims/DOALL`: **0 bare / 2 named**, the 2 being the known pre-existing
  `gershwin new-g3` failure that both compilers share. No regression.
- `imscomp` and `musicomp2abc` diff is exactly the expected `--sfzpipecsv`
  delta (756 lines, all SFZ-only); the legato edits are mirrored in both.
- `ims/test-legato.fs` inspected directly: `sleep 50.0` first line (priming),
  CC68 down across the slurred notes, `cc 0 68 0` immediately *before*
  `noteon 0 77 79` (the un-slurred note) — the ordering fix, visible.
- `gcs2sfz` on `ims/sfz-csv/test-legato/piano.csv`: emits `NOTEON pitch=72` once
  for the run plus a 19-step bend ramp to +2.00 semitones, restores `8192` at
  the run's note-off.
- VPO render of `b/01` violin (7 desks, ensemble detune + pan spread):
  -16.1 LUFS / true peak -0.2 dBFS with fusion on, -16.0 / -0.2 with it off
  (`GCS2SFZ_LEGATO=0`) — the fusion does not change level, and sfizz accepts
  the bend stream.
- `musicomp2abc` `make mp4` target: `Aflatmajor.mp4` builds in ~4 s.

## Open questions

1. **The two-note shorthand.** `b/01` writes `5e2l, 5f8` where a listener
   would hear a two-note slur. Under the convention settled on here that
   re-strikes the `5f`. Either the scores should mark the last note of every
   slur too, or the compilers should extend the pedal across the note
   following a chain of `l` notes. The latter is a one-line change to the
   release condition but it re-introduces exactly the inaudible-note bug
   described in §1 unless the note-off timing is also revisited. **This is a
   musical call, not a mechanical one — it needs the user's ear.**
2. **Is 8% worth it?** The fusion is correct but currently a small effect on
   real repertoire. Fixing (1) is what would make it large; that is why (1)
   comes first.
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

