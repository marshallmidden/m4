# SFZ pipeline session state (session 1, 2026-09-03)

Plan doc: `DEVIN-2026-09-03-sfz.md`. Original notes: `SFZ.md`.

## Objective
Add `--sfzpipecsv` output mode to `imscomp` emitting per-instrument CSVs
(`start_sec,dur_sec,midi_note,velocity`) so symphonies can be rendered with real
orchestral SFZ libraries (VPO via `~/bin/sfizz_render`), replacing GeneralUser.sf2 GM.

## Session 1 COMPLETED WORK

### Environment (done, carried over)
- `~/bin/sfizz_render` installed (statically linked, from GitHub releases 1.2.3).
- VPO library extracted to `music/sfz/library/VPO/Virtual-Playing-Orchestra3/`.
- Minimal test render verified: `~/bin/sfizz_render --sfz <VPO String s/1st-violin-SEC-sustain.sfz> --midi /tmp/test-violin.mid --wav /tmp/test-violin.wav`.

### `--sfzpipecsv` in `ims/imscomp` (this session)
All changes are in `music/ims/imscomp` ONLY (NOT yet mirrored to musicomp2abc).

Files/diffs: `git diff --stat ims/imscomp` => 262 insertions, 98 deletions.

#### Completed plumbing (carried into session from prior work)
- Arg added: `--sfzpipecsv` (aliases `--sfzpipe`, `--sfzpi`, `--sfz`).
- All 65 `args.midi1csv or args.fluidsynth` -> + `or args.sfzpipecsv`.
- Mutual-exclusion branches, `play` var, `print_output()` in-memory capture,
  init, end-of-song `print_out_sfzpipecsv()`, program/control format, tempo
  handling, key-signature, save/restore tuples.

#### NEW THIS SESSION — `print_out_sfzpipecsv()` (near line ~7714) rewritten
The function parses `array_of_lines` (in-memory MIDI CSV captured by
`print_output()`), extracts per-instrument note events, converts ticks->seconds,
and writes one `<instrument_name>.csv` per instrument.

Signature: `print_output()` sends ALL lines to `array_of_lines` when
`args.sfzpipecsv`. The array is globally time-sorted (comes from sorted `tm`).

Two bugs found & fixed:
1. **Velocity/note pairing**: old code hardcoded velocity 100 and used a single
   dict value per (port,chan,pitch) so overlapping same-pitch notes (trills)
   paired wrong. Now uses a FIFO list per (port,chan,pitch):
   `pending_notes[key] = [(start_tick, velocity, spt), ...]` popped `.pop(0)`.
   Note_on velocity 0 handled as note-off too.
2. **Timing conversion**: The tempo event in fluidsynth/sfz mode is
   `Tempo, {bpm}, {note_len}` (NOT microseconds!). The correct conversion
   (mirrors `print_out_midi1csv_tempo` else-branch): 
   `sec_per_quarter = 60 / (4 * note_len * bpm)`; `sec_per_tick = that / MIDICLICKSPERQUARTER`.
   Helper `sfz_sec_per_tick(cur_s, cur_l)` defined inside the function.
   `seconds_per_tick` is captured at each note-ON time and stored with the
   pending note, so mid-piece tempo changes are honored per-note.

Notes now stored in `notes[(port,chan)]` as `(start_sec, dur_sec, pitch, velocity)`.
There is still ONE global tempo issue (see Next Move / Known issues): tempo is
parsed lazily, and a note that starts before the first Tempo event / straddles a
tempo change uses the tempo captured at its start — acceptable for now.

#### Verification (session 1)
- Simple `ims/m4h.gcs`: `--sfzpipecsv` writes `acoustic_grand_piano.csv`,
  one note 0.447500,1.202500,89,97 -> correct.
- Beethoven 3 (`b/03/v3-1.E`): 34 instrument CSVs,
  starts ~0.04s, ends ~841s (correct ~14 min), real durations (0.29s, 1.66s),
  varied velocities. Canonical instrument names matched: bassoon, cello,
  clarinet, contrabass, flute, french_horn, oboe, pizzicato_strings, timpani,
  trumpet, viola, violin.
- DOALL regression: 0 bare ARGHs. 22 named ARGHs are ALL pre-existing
  (confirmed: run `cd music/b && ./AAA.diff._2` with changes stashed gives the
  same b2m3/b2m4/new-g3 execution failures). NO regression from my changes.

### Reproduce the CSV generation
```
cd music/b/03
gcc -E -x c -undef -Wundef -Werror -nostdinc -C -CC -traditional-cpp v3-1.gcs -o v3-1.E
rm -f *.csv
python3 ../../ims/imscomp --sfzpipecsv v3-1.E   # writes *.csv in CWD
```

## NEXT MOVES
1. **Create `music/sfz/instruments.py`**: map the instrument names that imscomp
   emits (from `b/instruments.include`, `wt/instruments.include`) to VPO SFZ
   files (e.g. `'violin'` -> `String s/1st-violin-SEC-sustain.sfz`,
   `'french horn'` -> `Brass/french-horn-SEC-sustain.sfz`,
   `'pizzicato strings'` -> pizzicato variant, etc.). Need to enumerate the
   actual names emitted and the VPO directory structure first.
2. **Create `gcs2sfz` renderer script**: parse each instrument CSV -> build
   per-instrument .mid -> `~/bin/sfizz_render` -> per-instrument .wav ->
   `ffmpeg amix` mixdown.
3. **Mirror `--sfzpipecsv` changes to `musicomp2abc/musicomp2abc`** (must stay
   byte-identical to imscomp; DOALL treats divergence as regression).
4. **Full end-to-end SFZ render of a real piece.**

## Known issues / TODOs
- Note that VPO instruments span up to midi note ~108; 1812-specific instruments
  (Gunshot, Explosion, Church Bells) need custom one-shots (not in VPO).
- Some instrument names (e.g. `french_horn`, `pizzicato_strings`, `contrabass`)
  need exact mapping to VPO filenames; verify against actual library layout.
- The `sfz_sec_per_tick` capture at note-on ignores tempo changes that occur
  strictly between note-on and note-off of a single note; fine for now.

## Artifacts to clean up after each experiment
- `music/b/03/*.csv`, `music/b/03/v3-1.E`, `music/ims/*.csv`, `music/ims/*.E`,
  `.fs`, `.abc` in the test dirs.

## RESOLVED: MIDI timeline bug (varint byte order)
- **Symptom**: dense MIDI (1786+ notes, 745s) rendered via `sfizz_render --use-eot`
  to a huge 8723s / 9087-tick file. Minimal MIDI rendered fine.
- **Root cause**: `gcs2sfz._varint()` (MIDI variable-length-quantity encoder)
  emitted the 7-bit groups **LSB-first** (reverse of standard MIDI VLQ which is
  MSB-first). Every delta > 127 ticks was corrupted — e.g. `_varint(128)` produced
  `0x80 0x01`, which a standard VLQ decoder reads as `1`. All multi-byte deltas
  were wrong, inflating the timeline to 8.7M ticks.
- **Fix**: rewrote `_varint` to collect 7-bit groups then emit them
  most-significant-first, setting the continuation bit (0x80) on every byte but
  the last. Verified byte-exact round-trip through a VLQ decoder for 0..~830k.
- **Validation**: contrabass MIDI now ends at EOT tick 715,058 (744.85s) with max
  single delta jump 33,418 (34.8s, legitimate phrase rests); balanced ons/offs
  (1793/1793). `sfizz_render` on the fixed MIDI yields a **744.85s** WAV (was
  9087s). `_varint` round-trip unit-check: 0 bad.

## CURRENT STATUS
- `gcs2sfz` MIDI writer timeline bug FIXED and validated end-to-end.
- Parallel rendering (ThreadPoolExecutor across instruments) NOT yet added.
- `musicomp2abc` mirror of `--sfzpipecsv` NOT yet done.

## DONE: parallel rendering in gcs2sfz
- Added `concurrent.futures.ThreadPoolExecutor` (threads release GIL during the
  sfizz/ffmpeg subprocess calls). `WORKERS = GCS2SFZ_WORKERS or 8`.
- `main()` now submits all `render_instrument` jobs up front and collects results
  in submission order; skips un-rendered instruments as before.
- Validated end-to-end on /tmp/sfztest (violin+cello CSVs -> parallel renders ->
  amix mixdown): both instruments rendered, output `sfztest-sfz-mix.wav` written.

## REMAINING
- Mirror `--sfzpipecsv` (and today's fixes conceptually) to `musicomp2abc/musicomp2abc`
  so the two compilers stay byte-identical (DOALL divergence check).
- Full real-piece end-to-end SFZ render (e.g. b/09/b9m1) with parallel render.
