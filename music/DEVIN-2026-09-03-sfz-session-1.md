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

# 2026-09-05 session 2: full b9m2 render — the --use-eot bug (CRITICAL, fixed)

## What happened
- Launched the first full 13-instrument b9m2 render (all 8 workers in parallel,
  detached via `screen -dmS b9m2sfz`). Parent ran ~1h35m, then the disk hit
  **100% full (116 MiB free)** — `/var/folders/.../T/gcs2sfz-*/` contained
  **532 GB** of per-instrument WAVs (cello.wav alone 121 GB = ~635,000 s of audio!).
  Renders were NOT stuck on compute — they were pathologically over-long.

## Root cause
- `sfizz_render` WITHOUT `--use-eot` freewheels and, for **dense** parts, never
  detects the end of the song — it keeps generating audio indefinitely.
  - Sparse parts (contrabass/pizzicato/timpani/trombone, ≤1485 notes) terminate on
    their own at the correct length (~1173 s / 225 MB).
  - Dense parts (all 8 with >1447 notes, e.g. trumpet 1447, cello 1804, horn 5900)
    produced 30–121 GB WAVs (encoded as 474k–635k s and still growing).
  - Not a `write_midi`/`_varint` problem: MIDIs are correct (max tick ~1121k =
    ~1168.7 s); CSVs are clean (max end 1171.6 s).
- With `--use-eot`: trumpet renders exactly **1171.56 s** in <60 s. That's the fix.

## Fix
- `gcs2sfz.render_instrument` cmd now: `[SFIZZ, "--use-eot", "--sfz", ..., "--midi", ..., "--wav", ..., "--samplerate", ...]`

## Validation
- 3-instrument test (trumpet + french_horn + cello CSVs): finished in 16.3 s,
  mix `b9m2-test-sfz-mix.wav` = 1177.6 s estimated (1171.56 actual) — correct.
- **Full b9m2 render (all 13 instruments): DONE in ~1 min**, mix =
  `~/b9m2-sfz-render/out/b9m2-sfz-render-sfz-mix.wav`, duration **1171.56 s**,
  224.9 MB @1536 kbps, mean_volume -14.3 dB / max 0.0 dB. No orphan processes.

## Lesson
- `sfizz_render` ALWAYS needs `--use-eot` for dense music, or freewheeling runs
  away and fills the disk. If a render "never finishes": check `du -sh` of the
  gcs2sfz `tempfile.mkdtemp` dir; kill it, add `--use-eot`, rerun.

## REMAINING
- `musicomp2abc` mirror of `--sfzpipecsv` + `--use-eot` (keep byte-identical).
- Note: `make sfz` / `make sfz-mp4` b/09 Makefile targets still untested end-to-end.

# 2026-09-05 session 3: lead-in, limiter, audio levels (user listened with afplay)

## What the user found
- Playing the mix with `afplay` sounded like the first note(s) were chopped off.
  - **Not a render bug**: audio began at t=0 (ramp to -37 dB by frame ~10), nothing
    lost at sample level. `afplay` has startup latency; with zero lead-in the
    attack plays during afplay's startup. 319 full-scale pins of 112M samples.
  - `ffplay -nodisp -autoexit wav` and `open wav` (QuickTime/VLC) work fine.
- Instrument timbres differ from fluidsynth GeneralUser — EXPECTED: VPO is a real
  orchestral sample library; that is the point of the SFZ pipeline.

## Fixes in gcs2sfz
1. **Lead-in**: `LEAD_IN = float(os.environ.get("GCS2SFZ_LEAD_IN", "1.0"))`
   seconds of silence before the first note. `mixdown(..., lead_in)` appends
   `adelay=<lead_ms>ms:all=1` to the filter chain; `target = LEAD_IN + piece_end + TAIL`.
2. **Limiter**: mixdown chain now `amix=...:normalize=0:duration=longest,
   alimiter=limit=0.95:level=false`. `level=false` is REQUIRED — default auto
   makeup gain pushes output back toward full scale (max stayed 0.0 dB / 372 clips).

## test-pipeline (new)
- `music/music/sfz/test-pipeline` — self-contained smoke test: writes 3 synthetic
  instrument CSVs (cello/violin/french_horn, piece ends 3.0s), runs gcs2sfz with
  `GCS2SFZ_LEAD_IN=1.0`, asserts: mix exists, duration ≈ piece_end + lead_in,
  48000Hz/2ch, audio present (peak > 3000/32767), NO clipped samples, first note
  onsets after lead_in (silencedetect). Run: `cd music/music/sfz && python3 test-pipeline`.
  Note: mix output filename uses the input dir's basename — test finds `*-sfz-mix.wav`.
  Use `-v info` (not error) when parsing silencedetect output (info-level).
  Currently PASS.

## Verified final b9m2 render (~/b9m2-sfz-render/out/b9m2-sfz-render-sfz-mix.wav)
- duration 1172.56s (1171.6 piece + 1.0 lead-in), 225132110 bytes
- 0 clipped samples of 112.5M, max 0.950 FS (limiter working), mean -14.3 dB
- first sound at 1.031s (lead-in intact)

## REMAINING (for next session)
- **Mirror `--sfzpipecsv` (+ lead-in/limiter conceptually) to `musicomp2abc/`** so
  imscomp/musicomp2abc stay byte-identical — DOALL regression gate. NOT done.
- b/09 `make sfz` / `make sfz-mp4` / `make b9m2-sfz.mp4` targets added in e76ff628
  still NOT validated end-to-end (render pipeline verified manually instead).
- Clean up/ignore b/09 build artifacts (b9m2.E, *.csv, b9m2_2.fs) — not committed.
- Optional: 1812 one-shots (gun/explosion/bells) still TODO from earlier projects;
  GM drum-channel remap in imscomp still TODO.
- Render notes: keep on macOS, run under `screen -dmS` (detached) to survive
  workspace interruptions; ALWAYS `--use-eot`; watch `GCS2SFZ_WORKERS`/disk.
