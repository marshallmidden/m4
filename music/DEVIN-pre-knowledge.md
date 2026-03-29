# DEVIN-pre-knowledge.md - Project Reference

Background reference for AI assistants working on the music compilation and playback system.
Load this file at the start of a session: "read DEVIN-pre-knowledge.md for background".

---

## Project Overview

This workspace contains a music compilation pipeline that converts `.gcs` source files
through multiple intermediate formats to produce audio output.

**Pipeline**: `.gcs` -> `.E` (CPP preprocessor) -> `.abc` / `.csv` / `.fs` -> `.mid` / `.wav` / `.pdf`

Two compilers produce output from `.E` files:
- **musicomp2abc** (`musicomp2abc/musicomp2abc`) - the older compiler
- **imscomp** (`ims/imscomp`) - the newer compiler (actively developed)

Files with `_2` suffix (e.g. `s14_2.fs`) are imscomp output; without suffix are musicomp2abc.

### Key Directories

| Directory | Contents |
|-----------|----------|
| `ims/imscomp` | The compiler (single ~20,280-line Python file) |
| `ims/DOALL` | Full regression test script |
| `ims/calculate.py` | Expression parser module used by imscomp |
| `ims/tc-testing/theme.gcs` | Simple 2-staff test file |
| `ims/tc-testing/gershwin/` | Gershwin orchestral test files (g3.gcs, g21.gcs, g22.gcs) |
| `ims/csv2fs/` | CSV to FluidSynth converter |
| `b/` | Beethoven compositions (01-09 = symphonies, sonata14 = Moonlight) |
| `b/09/b9m2.gcs` | Beethoven 9th Mvt 2 (35 voices, 19 staves, transposing) |
| `b/sonata14/s14.gcs` | Beethoven Moonlight Sonata Op.27 No.2 (6 voices, 3 staves) |
| `songs/` | Collection of song test files |
| `musicomp2abc/` | The older compiler and ABC-related test files |
| `abcm2ps/` | ABC to PostScript converter |
| `abcmidi-m4-64/` | ABC to MIDI converter |

### Build System

Each composition directory has a `Makefile`. Key rules:
```
%.fs: %.E          # musicomp2abc --fluidsynth
%_2.fs: %.E        # imscomp --fluidsynth
%.csv: %.E         # musicomp2abc --midi1csv
%_2.csv: %.E       # imscomp --midi1csv
%.mid: %.csv       # csvmidi
%.wav: %.mid       # fluidsynth -F
%.abc: %.E         # imscomp --abc  (or musicomp2abc --abc)
%.ps: %.abc        # abcm2ps
%.pdf: %.ps        # ps2pdf
%.E: %.gcs         # cpp -P (C preprocessor)
```

Rebuild a specific output: `cd b/sonata14 && make -B s14_2.fs`
Rebuild everything: `cd b/sonata14 && make -B`

---

## imscomp Details

### How to Run

```bash
# FluidSynth output (.fs file)
cd /home/m4/LEARNING/music/b/sonata14
python3 ../../ims/imscomp --fluidsynth s14.E s14_2.fs

# Other formats
python3 ../../ims/imscomp --abc s14.E s14.abc
python3 ../../ims/imscomp --midi1csv s14.E s14_2.csv
python3 ../../ims/imscomp --staves s14.gcs 2>/dev/null | head -80
python3 ../../ims/imscomp --lilypond s14.gcs 2>/dev/null | head -80
```

**IMPORTANT**: .gcs files use `#include` with relative paths. Always `cd` to the
composition's directory first.

### How to Run Tests (DOALL)
```bash
cd /home/m4/LEARNING/music/ims && bash DOALL
```

DOALL runs 5 test suites in parallel: `songs/`, `musicomp2abc/`, `b/`, `t/e/`, `tc-testing/gershwin/`.
Each suite runs `AAA.diff._2` which compiles `.gcs` files with both imscomp and musicomp2abc,
then diffs the outputs across all format flags (abc, h, v, csv, fs).

**Expected baseline (as of 2026-03-28)**: 0 bare ARGHs, 2 named ARGHs.
- Bare ARGHs = imscomp-vs-musicomp2abc diffs. Currently 0 (files are synced).
- Named ARGHs = execution failures. Only `new-g3` fails (known issue in gershwin suite).
- Exit code 0 if named ARGHs <= 2 (PASS with known diffs). Exit code 1 if more (FAIL).

**When verifying changes**: Always run full DOALL and confirm 0 bare / 2 named.
Changes that increase named ARGHs are regressions.

### Output Format Flags

| Flag | Output |
|------|--------|
| `--fluidsynth` (or `--fs`, `-f`) | FluidSynth .fs command file |
| `--midi1csv` | MIDI CSV format |
| `--abc` | ABC notation |
| `--staves` | Staff-grouped display |
| `--lilypond` | LilyPond notation |
| `--horizontal` | Horizontal display |
| `--vertical` | Vertical display |
| `--nohumanize` (or `--nohum`..`--nohumaniz`) | Disable timing humanization |
| `--noarpeggio` (or `--noarp`..`--noarpegg`) | Disable arpeggio chord staggering |

### Sound Quality Features (MIDI/FluidSynth output, 2026-03-27)

Six features that improve realism of MIDI playback. All implemented in
`print_out_midi1csv_notes()` (line ~5889). On by default; controllable via
GCS variables and command-line flags.

| Feature | Default | GCS Variable | CLI Flag | Description |
|---------|---------|-------------|----------|-------------|
| Default duration | 0.90 (90%) | `default_duration` | — | Unmarked notes shortened to create natural gap. Skips tied/legato/staccato/marcato/tenuto/cresc notes. Looks ahead for ties to avoid breaking them. |
| Velocity dynamics | on | — | — | Note velocity scales with CC 11 expression: `vel = max(40, min(127, int(vel * cc11 / 100)))`. Gives timbral variation. |
| Timing humanization | ±5 ticks | `humanize` | `--nohumanize` | Random jitter on Note_on and CC emissions. Deterministic via `random.seed(42)`. Note_off and measure boundaries stay exact. |
| Arpeggio | 1/32 note (60 ticks) | `arp` | `--noarpeggio` | Chord notes staggered across voices on same staff. Voice position within staff determines delay: `arp_ticks * position`. |
| Sustain pedal | off (0) | `sustain_pedal` | — | CC 64 legato pedaling. Set `sustain_pedal -1` in .gcs to enable. Pedal up at note_time + overlap/2, down at note_time + overlap. Not applied during staccato. |
| Pedal overlap | 60 ticks | `pedal_overlap` | — | Controls sustain pedal timing (ticks after note start). |

**Key implementation details**:
- `note_time` variable holds humanized time offset; used for Note_on and CC emissions
- `_voice_staff_pos` dict (line ~5920) maps voices to their position within a staff for arpeggio
- Tie lookahead (default duration): checks both next note in same measure AND first note of next measure
- `random.seed(42)` at line ~5917 ensures reproducible humanization across runs

### Key Function Locations (line numbers approximate, as of 2026-03-27)

| Line  | Function / Section |
|-------|-------------------|
| 1476  | `createglobalvar('default_duration', ...)` - default note duration (0.90) |
| 1477  | `createglobalvar('humanize', ...)` - max humanization jitter (5 ticks) |
| 1478  | `createglobalvar('sustain_pedal', ...)` - CC 64 control (0=off, -1=on) |
| 1479  | `createglobalvar('pedal_overlap', ...)` - pedal timing (60 ticks) |
| 1516  | `def is_float(` - parse/evaluate string as numeric expression (has fast-path for plain numbers) |
| 1962  | `--nohumanize` argument definition |
| 1965  | `--noarpeggio` argument definition |
| 2231  | `_frac_cache` / `_cached_frac256()` - Fraction cache for limit_denominator(256) |
| 3221  | `def get_time_stak(` - convert note duration to ABC/MIDI time values |
| 3404  | `def new_voice_initialize(` - init per-voice data structures |
| 3977  | `def print_measure_vh_header(` - per-measure metadata for vertical/horizontal |
| 4355  | `def print_measure_staves_header(` - per-measure metadata for staves |
| 4532  | `def print_out_staves(` - staves output |
| 4805  | `def _ly_note_name(` - convert internal note format to LilyPond notation |
| 4864  | `def _ly_duration(` - convert internal duration to LilyPond duration |
| 4912  | `def print_out_lilypond(` - LilyPond output function |
| 5464  | `def print_header(` - header output for all formats |
| 5803  | `def do_midi_vpi(` - CC 11/10/7 emission during crescendo/diminuendo |
| 5889  | `def print_out_midi1csv_notes(` - **main MIDI CSV note output** (sound quality features live here) |
| 5917  | `random.seed(42)` - deterministic humanization seed |
| 5920  | `_voice_staff_pos` - arpeggio voice position mapping |
| 5982  | `last_cc11 = -1` - CC tracker initialization per voice |
| 6620  | `change_name` dict - maps internal MIDI CSV names to FluidSynth commands |
| 6638  | `def print_out_fluidsynth()` - FluidSynth output generator |
| 8398  | `def put_on_bufs(` - add note to output buffers; updates mlth |
| 8615  | `def instak(` - insert note into time-sorted stack |
| 8700  | `def fill_voice_mlth(` - pad all voices with rests to match longest (has skip-set optimization) |
| 13930 | `def do_xpose(` - transposition command |
| 14836 | `voice` command parsing (vargs handling) |
| 14945 | `middle_c = None` - middle_c default initialization |
| 15030 | `def getnote(` - main note-processing function (~600 lines) |
| 15369 | Key conversion section ("Convert for key") |
| 15444 | Xpose condition |
| 15397 | Note format conversion to `o+n+a` format |
| 19420 | `def main(` |

### FluidSynth Output Format (.fs files)

imscomp converts internal MIDI CSV events to FluidSynth shell commands:
```python
change_name = {
    'Note_on_c': 'noteon',      # noteon <chan> <note> <vel>
    'Note_off_c': 'noteoff',    # noteoff <chan> <note> <vel>
    'Control_c': 'cc',          # cc <chan> <cc#> <val>
    'Pitch_bend_c': 'pitch_bend',
    'Text_t': 'echo',
    'Tempo': 'tempo',
    ...
}
```

Time gaps between events become `sleep <ms>` commands (fractional ms supported).

Example .fs file structure:
```
set audio.driver coreaudio      # Settings (ignored when piped)
set synth.gain 0.5
set synth.midi-channels 128
reset
set synth.reverb.active 1
...
prog 00 000                     # Program changes
cc 0 11 40                      # CC: expression=40 (pp)
noteon 0 56 75                  # Note on: chan 0, G#3, vel 75
sleep 258.62                    # Wait 258.62 ms
noteoff 0 56 0                  # Note off
...
quit                            # End of file
```

### Internal Note Format

Notes stored in `bufs[voice][measure]` as strings:
- `'4c'` = octave 4, C natural
- `'3d+'` = octave 3, D sharp
- `'5b-'` = octave 5, B flat
- Prefixes: `vol(90)`, `p(64)`, `in(100)`, `<`, `>` (cresc/dim markers)

### Key Data Structures

```
bufs[voice][measure]        - list of note strings
ba.length[voice][measure]   - list of note durations
ba.suffixes[voice][measure] - list of suffix strings ('t' for tie, 'c' for cresc, etc.)
ba.volume[voice][measure]   - list of volumes
volume[voice][measure]      - volume value (0-127) (dict of dicts)
pan[voice][measure]         - pan value (0-126)
intensity[voice][measure]   - expression/intensity value
reverb[voice][measure]      - reverb value
mlth[voice][measure]        - accumulated note duration in each measure per voice
tlth[voice]                 - total duration per voice
xpose[voice]                - semitone transposition (via staff array)
key_voice[voice]            - current key for each voice
staff_name[label]           - maps staff label to list of voice numbers
clef[voice]                 - clef for each voice
vinstrument[voice]          - instrument for each voice
middle_c                    - None default, set to 39+12=51 for staff format
meas[]                      - ordered list of measure identifiers
_fvmlth_done                - set of measures already padded by fill_voice_mlth
_fvmlth_nv                  - voice count at last fill_voice_mlth run (resets _fvmlth_done on change)
_frac_cache                 - dict caching str(Fraction(v).limit_denominator(256)) results
_voice_staff_pos            - dict mapping voice to its position (0-based) within its staff (for arpeggio)
note_time                   - per-note humanized time offset (lthworkingmeasures + random jitter)
```

### MIDI CC Tracking Variables (per voice, in FluidSynth/CSV output)

```
last_note_on    - string: currently playing note ('' = no note / rest)
last_cc11       - int: last emitted CC 11 (expression/volume), init -1
last_cc10       - int: last emitted CC 10 (pan), init -1
last_cc7        - int: last emitted CC 7 (intensity), init -1
```

These track what CC values have actually been sent to MIDI, to avoid
redundant emissions. The CC bug fix (see below) was about keeping these
in sync with actual emissions.

---

## FluidSynth (Custom Build)

### Location and Version
- **Source**: `/home/m4/music/fluidsynth` (git clone, v2.5.3-64-g30380a25)
- **Binary**: `/home/m4/bin/fluidsynth` (version 2.5.3)
- **Library**: `/home/m4/lib/libfluidsynth.so.3`
- **Soundfont**: `/home/m4/share/soundfonts/default.sf2` (symlink to `/home/m4/src/GeneralUser_GS/GeneralUser.sf2`)
- **Config**: `/home/m4/.fluidsynth` (WSL2 audio buffer settings)

### How to Build
```bash
cd /home/m4/music/fluidsynth/build
make -j$(nproc) && make install
```
Installs to `~/bin`, `~/lib`, `~/include` (CMAKE_INSTALL_PREFIX=/home/m4).

### Custom Patches Applied (3 patches on top of upstream v2.5.3)

#### Patch 1: Fractional sleep support
- **File**: `src/bindings/fluid_cmd.c` line ~1125
- **Change**: `atoi` -> `atof` in `fluid_handle_sleep()` to support fractional ms sleep values
- **Why**: imscomp generates `sleep 258.62` etc.

#### Patch 2: Stdin echo suppression
- **File**: `src/utils/fluid_sys.c` line ~1169
  - Added `&& isatty(in)` to readline check -- only uses readline for interactive TTY, not pipes
- **File**: `src/fluidsynth.c` lines ~1113-1128
  - Added `int is_tty = isatty(fileno(stdin))` -- suppresses "Type 'help'" banner and sets prompt to "" when stdin is a pipe
- **Why**: When piping .fs files via `cat s14_2.fs | fluidsynth`, every command was echoed with `> ` prompt

#### Patch 3: WAV rendering from stdin-piped .fs commands (2026-03-23)
- **Problem**: `-F file.wav` only worked with MIDI files. No way to render `.fs` stdin commands to WAV.
- **Files changed**:
  - `src/bindings/fluid_cmd.c`: Added `renderer` and `pending_samples` fields to `_fluid_cmd_handler_t`. Added `fluid_cmd_handler_set_renderer()`. Modified `fluid_handle_sleep()` to render audio frames (via `fluid_file_renderer_process_block()`) instead of sleeping when renderer is set. Accumulates fractional samples across sleep calls.
  - `src/fluidsynth.c`: Added `#include "fluid_cmd.h"`. Removed `interactive = 0` from fast_render block. Added stdin rendering path: when `-F` is used without a MIDI file, creates file renderer, attaches to command handler, runs shell, cleans up.
  - `include/fluidsynth/shell.h` + `src/bindings/fluid_cmd.h`: Declared `fluid_cmd_handler_set_renderer()` with `FLUIDSYNTH_API` (needed because CMake sets hidden visibility).
- **Key insight**: `sleep` commands in .fs files advance time. For file rendering, each `sleep N` computes `N * sample_rate / 1000` samples and calls `fluid_file_renderer_process_block()` in a loop (one call per `period_size` samples). Fractional samples carry over.

### Usage

```bash
# Play .fs file through audio driver (real-time)
cat s14_2.fs | fluidsynth -q /path/to/soundfont.sf2

# Render .fs file to WAV (fast, non-realtime) -- CUSTOM PATCH
cat s14_2.fs | fluidsynth -q -F output.wav /path/to/soundfont.sf2

# Render MIDI file to WAV (standard fluidsynth feature)
fluidsynth -q -F output.wav /path/to/soundfont.sf2 input.mid
```

### WSL2 Audio Configuration (`/home/m4/.fluidsynth`)
```
set audio.period-size 1024
set audio.periods 4
set audio.driver pulseaudio
```
Default period-size=64 causes crackling on WSL2 (FluidSynth -> PipeWire-pulse -> WSLg RDP -> Windows).

### FluidSynth Source Layout

| Directory | Contents |
|-----------|----------|
| `src/fluidsynth.c` | Main application (CLI, fast_render_loop, shell startup) |
| `src/bindings/fluid_cmd.c` | Command handler, shell, all `fluid_handle_*` functions |
| `src/bindings/fluid_cmd.h` | Private header for command handler |
| `src/bindings/fluid_filerenderer.c` | File renderer (`fluid_file_renderer_process_block`) |
| `src/utils/fluid_sys.c` | System utilities (readline, sleep, isatty) |
| `src/synth/` | Synthesizer core |
| `src/drivers/` | Audio/MIDI drivers |
| `src/midi/` | MIDI processing |
| `src/sfloader/` | SoundFont loading |
| `include/fluidsynth/shell.h` | Public API for command handler and shell |

---

## Bug Fix History

### Subagent-Discovered Bugs and Improvement Passes (2026-03-28) -- COMPLETE

**Subagent bug hunt (4 real fixes, 2 false alarms):**
1. Line 18116: `for ls in last_voice_staff:` → `for ls in save_last_voice_staff:` in `set_process_begin()` (CRITICAL: loop iterated over empty list, never executing — process quit logic was dead)
2. Line 3531: `mmm in clef and j in clef[mmm]` → `j in clef and mmm in clef[j]` in `new_voice_initialize()` (swapped operands: `clef` is keyed by voice, not measure — condition never matched, defaulted to treble)
3. Line 3675: Added `len(staff_name[st]) > 0` check in `firstvoiceinstaff()` (crash prevention: `staff_name[sn] = []` at line 13341 can create empty lists)
4. Line 3720: `clef [voiceon]` → `clef[voiceon]` (cosmetic: extraneous space before bracket)
- Lines 3529, 12340 off-by-one: **false alarm** — `-1` correctly skips current measure (just appended to `meas`)
- Line 5179 bounds check: **false alarm** — voices are 1-indexed, `voiceon == len(volume)` correctly identifies last voice

**Improvement passes 1-4 (completed earlier in session):**
- Crash bugs: Line 11403 `me3asure_on` → `measure_on` typo + removed undefined `{command}`; Line 14446 added missing `f` prefix
- Error messages: Line 12548 `"legal note"` → `"not a legal note"`; Line 12545 fixed 5→4 space indent; Line 14432 fixed return type `-> None` → `-> tuple`
- Redundant conditions: Lines 16552, 16852, 17421 removed duplicate leading conditions
- Dead code: Line 2326 removed unreachable `break` after `return`
- Semicolons: Lines 5150, 8073, 8800, 11311 removed trailing semicolons
- calculate.py: Line 1124 `arrays + local_arrays` → `variable_index.get(arg1[1], [])` (O(n)→O(1) lookup)

**.gcs tempo conversions (completed earlier in session):**
- `musicomp2abc/Aflatmajor.gcs`: Removed 10 intermediate tempos, added `tempo 116,4,rit` at m41
- `songs/promenade2.gcs`: Removed 9 intermediate tempos, added `tempo 90,4,rit` at unit 12.5
- `ims/tc-testing/gershwin/g3.gcs`: Converted 4 sections to use rit/acc syntax

**DOALL after all fixes**: 0 bare ARGHs, 2 named ARGHs (new-g3 only, pre-existing).

**Session reference**: `history_9e6ba5d844d34ba0.md`

### Volume Table Change, Batch 6 Bug Fixes, and calculate.py Audit (2026-03-28) -- COMPLETE

**Volume table change**: Shifted all dynamics in `vlprint` table up by 10 (pp:40→50, p:50→60,
mp:60→70, mf:70→80, f:80→90, ff:90→100, fff:100→110, ffff:110→120, pppp:20→30, ppp:30→40).
Updated stale comments in volume number lookup to match. Only modified ims/imscomp.

**Musicomp2abc baseline reset**: Copied `ims/imscomp` → `musicomp2abc/musicomp2abc` and
`ims/calculate.py` → `musicomp2abc/calculate.py` so DOALL shows 0 bare ARGHs.
Originals saved as `musicomp2abc/musicomp2abc.SAVE` and `musicomp2abc/calculate.py.SAVE`.

**Pitchbend audit**: Verified pitchbend implementation is correct for fluidsynth output
(single 14-bit value, not split LSB/MSB). Autopitchbend range 7292-9092 is safe within 0-16383.

**Xpose investigation**: Verified note-on/note-off balance (0 unmatched). Horn sustain in b9m2_2.fs
is from legitimately long tied notes (6-17 seconds), not a bug.

**Batch 6 — Bug fixes in imscomp (16 fixes):**
1. Line ~1979: Dead code comment fix (`len(l) < 0` impossible)
2. Line ~2008: `args.intensity != ''` → `args.intensity = ''` (comparison→assignment)
3. Line 3315: Added `f` prefix to f-string in `set_all_staff_arr`
4. Line 3316: `print_die(here)` → `print_die("here")` (undefined variable)
5. Line 3339: `.` → `, ` separator AND `print_die("here")`
6. Line 3340: `print_die(here)` → `print_die("here")`
7. Line 3362: `.` → `, ` separator AND `print_die("here")`
8. Lines 3386-3387: `.` → `, ` separator AND `print_die("here")`
9. Line 3712: Missing `f` prefix on f-string in `print_measure_abc`
10. Line 9396: Missing `f` prefix on f-string for default note error
11. Line 13190: Typo `"0to 127"` → `"0 to 127"` in reverb error
12. Line 15989: `{strpvi}` → `{str_pvi}` (undefined variable typo)
13. Lines 16447-16454: `which_intensity_now` → `which_intensity` (undefined variable)
14. Lines 16609-16610: Added `return None` after trill error (was falling through)
15. Line 17472: `w + 1 > len(wargs)` → `w + 1 >= len(wargs)` (off-by-one)
16. Line 18172: Removed dead `, False` from `pop_process()` call (unused tuple)

**calculate.py audit (2 fixes):**
17. Line 871: `raise RuntimeError(msg, file=sys.stderr, flush=True)` — removed invalid kwargs
    (RuntimeError doesn't accept `file=`/`flush=`; would TypeError instead of raising properly)
18. Line 639: Removed dead variable `f = a1 / a2` (return on next line recalculated it)

**DOALL after all fixes**: 0 bare ARGHs, 2 named ARGHs (new-g3 only, pre-existing).

**Session reference**: `history_585210cdd61843d8.md`

### Comprehensive Code Audit & Bug Fixes (2026-03-27) -- COMPLETE

Full 9-subagent code audit of imscomp (~20,280 lines) and calculate.py (~2,109 lines).
Fixed 21 verified bugs across 4 batches. DOALL baseline maintained: 214 bare, 2 named ARGHs.

**Batch 1 — Crash/NameError bugs (13 fixes):**
1. `args.intensity != ''` → `args.intensity = ''` (comparison→assignment, 3 locations)
2. Added `original_line = ''` at top of `parse_args()` (5 NameError sites)
3. `{i}` → `{name}` in `get_volume_number()` error message
4. Added `, original_line` to `print_error()` calls in `handle_volume_both()` (3 locations)
5. Added `f` prefix to f-string in `getvar_checkokay()`
6. Added `f` prefix to f-string in `set_all_staff_arr()`
7. `theynote` → `note_to_decode` in `do_middle_c()`
8. `nc = None` → `nc = 'treble'` in `new_voice_initialize()` (prevents crash)
9. `next_a` → `next_tokens` in `keyword_transcribe()`
10. Added `return False` after exception in `do_line()` try/except
11. calculate.py `f_m()`: removed extra `txt` in format string
12. calculate.py `f_m()`: range check `> 50` → `> 100`
13. calculate.py `quotes_eval()`: undefined `value` → error return

**Batch 2 — Logic bugs (6 fixes):**
14. `rs.intensity[1]` → `rs.intensity[voiceon]` (wrong voice for intensity clamping)
15. `not lengthl or not lengthl` → `not lengthl or not lengthf` (duplicate condition)
16. Removed duplicate intensity initialization block in `new_voice_initialize()`
17. Malformed MIDI string `0"` → `0'` (mismatched quote in CC 68 control)
18-19. xpose `or` → `and` validation (3 locations) — `or` always true for values in one dict but not the other

**Batch 3 — Error handling bugs (2 fixes, 1 false alarm):**
20. `do_copy()`: missing `return` after error (fell through to overwrite command)
21. Staccato/legato conflict: `replace('s','')` → `replace('l','')` (comment said "remove legato" but code removed staccato)
22. `do_delay()` inverted conditional — **false alarm**: `tempo_note_length_now['']` is a valid key

**Batch 4 — calculate.py logic bugs (2 fixes):**
23. `f_not()`: returned -1 for non-zero (should be 0) and 0 for zero (should be -1) — was normalizing, not inverting
24. `f_frac()`: `int(round(a)) - a` → `a - int(a)` (wrong sign and rounding instead of truncation)

**Session reference**: `history_a5733283be694314.md`

### Sound Quality Refinements (2026-03-27) -- COMPLETE

**Batch 5 — Two improvements, two false alarms:**
25. Sustain pedal (CC 64) now releases on rest — previously pedal stayed down through rests,
    causing notes to ring when they should be silent.
26. Velocity minimum now configurable via `velocity_min` variable (default 20, was hardcoded 40).
    Old value of 40 made ppp (vel 27) and pp (vel 36) sound identical. Now ppp=27, pp=36 are distinct.
    New GCS variable: `velocity_min` (0-127).
27. Arpeggio offset lost after first measure — **false alarm**: offset is applied to `lthworkingmeasures`
    which accumulates across measures, not reset per-measure.
28. Legato minimum overlap hardcoded — **not a bug**: computed from `MIDICLICKSPERQUARTER // 8`, not arbitrary.

### Sound Quality Features for MIDI Output (2026-03-27) -- COMPLETE

**Six features** added to `print_out_midi1csv_notes()` in imscomp to improve MIDI/FluidSynth
playback realism: default duration gap, velocity dynamics, timing humanization, arpeggio
chord staggering, sustain pedal (CC 64), and pedal overlap control.

**Command-line flags**: `--nohumanize`, `--noarpeggio`
**GCS variables**: `default_duration`, `humanize`, `sustain_pedal`, `pedal_overlap`, `arp`, `velocity_min`

**Key bug fixed during implementation**: Default duration initially broke cross-measure ties.
The tie lookahead now checks both the next note within the current measure AND the first note
of the next measure (if not going through a goto). This eliminated "tied note is not same"
errors in 5-dollar-fuga and similar tests.

**Verification**: DOALL passes with baseline counts (214 bare, 2 named ARGHs).
See "Sound Quality Features" section above for full details.

### Voice Command Parsing Bug Fix (2026-03-27) -- COMPLETE

**Symptom**: `voice 4 19,40,56,0` caused "ERROR - voice error - bad input to calculator#3"

**Root cause**: `jkl = str(vargs[1:])` converts a Python list to its repr (e.g., `"['19,40,56,0']"`),
injecting brackets and quotes that corrupt downstream parsing.

**Fix**: Changed to `jkl = ','.join(vargs[1:])` in both compilers:
- `ims/imscomp` line ~13640 (now ~13650)
- `musicomp2abc/musicomp2abc` line 12434

### Two Minor Bug Fixes in imscomp (2026-03-27) -- COMPLETE

1. **Line ~9003**: `{a}` -> `{o}` in debug message for `to_abc_note()` bad octave (NameError crash fix)
2. **Line ~19170**: Added `numcharvar = 0` before while loop in `readthefile()` (UnboundLocalError crash fix during macro argument substitution)

### Performance Optimizations in imscomp (2026-03-27) -- COMPLETE

Three optimizations applied, together providing ~2x speedup on large files:

**1. `fill_voice_mlth` skip-set** (line ~8562):
- Tracks processed measures in `_fvmlth_done` set; skips them on subsequent calls.
- Clears the set when voice count (`len(volume)+1`) changes (new voices need all measures re-processed).
- Never marks the current `measure_on` as done (it may still receive notes).
- Eliminated O(n^2) rescan: was 0.43s, now negligible.
- **CAUTION**: This optimization is sensitive. The first attempt (tracking by index) caused 43 regressions because it didn't handle new voices appearing mid-song. The current set-based approach with voice-count reset is correct.

**2. `_cached_frac256()` cache** (line ~2183):
- Memoizes `str(Fraction(v).limit_denominator(256))` results.
- Used in `get_time_stak` (lines ~3205, ~3215) and `note_text_fraction` (line ~3247).
- Avoids ~39K redundant Fraction constructions.

**3. `is_float` fast path** (line ~1488):
- When `type_float == is_float_number`, tries `float(strg)` first.
- If it succeeds (plain numeric string), returns immediately without calling `calculate.parse()`.
- Falls through to full parse on `ValueError` (expressions, variables, etc.).
- Cut parse calls roughly in half (70K -> 33K).

**Performance results** (b-6.gcs, largest test file, 21 voices):
| Metric | Before | After |
|--------|--------|-------|
| Wall-clock | ~1.3s | ~0.66s |
| Profile total | 2.8s | 1.3s |
| Function calls | 6.8M | 3.2M |

### ABC "Bad tie" Issue (2026-03-27) -- ANALYZED, NOT FIXED

**Symptom**: abcm2ps reports "Bad tie" errors on chord-to-single-note ties.

**Pattern**: When a chord like `(c1 G1)` is tied and only one note continues in the next
measure (e.g., just `G2`), the tie `-` is appended to the entire chord notation, not per-note.

**Code location**: `bufs[voiceon][m] = bufs[voiceon][m] + '-'` at line ~15652 in imscomp.
This blindly appends tie to the whole chord without per-pitch validation.

**Status**: Analysis complete; no fix applied yet. Would require matching individual chord
pitches between measures to determine which notes are truly tied.

### CC (Expression/Volume) Bug Fix in imscomp (2026-03-22) -- COMPLETE

**Symptoms**: Instruments disappeared or went very quiet during playback.
Channel 23 in b9m2 had only 3 CC 11 events instead of 41.
Moonlight Sonata dynamics were flat (no crescendo in m25-27).

**Root cause**: Two bugs where CC 11/10/7 trackers (`last_cc11` etc.) fell out of
sync with actual MIDI state.

**Fix 1** (line ~6086-6097): Removed `if last_note_on != ''` guard around CC emission
before note-on. Since rests already `continue` earlier (line ~6044), this code only
runs when about to play a pitched note, so CC should always be emitted. The guard
was suppressing CC updates after rests/staccato gaps.

**Fix 2** (`do_midi_vpi` function, lines ~5768-5801): Moved `last_cc11 = ivol` (and
cc10/cc7 tracker updates) inside the `if last_note_on != ''` block so the tracker
only updates when the CC is actually sent to MIDI. Previously the tracker updated
even when the CC was suppressed, causing it to think the value was already sent.

**Verification**:
- b9m2_2.fs CC counts: 4,033 -> 10,658 (matching musicomp2abc's 10,572)
- s14_2.fs CC counts: ~1,300 -> 1,945 (matching musicomp2abc's 1,921)
- Note counts identical in both: 38,795 for b9m2, 1,169 for s14

### Feature #3: --staves output (COMPLETE, 2026-03-21)
- Per-staff metadata via `print_measure_staves_header()` (line 4295)
- Staff-based output via `print_out_staves()` (line 4472)
- DOALL: ALL OKAY!

### Feature #4: --lilypond written pitch (COMPLETE, 2026-03-21)
- Removed `args.lilypond` from xpose condition at line 15317
- LilyPond now outputs written pitch (not concert pitch)
- DOALL: ALL OKAY!

### Known Remaining Issues
- LilyPond: `v,,,` artifacts may appear in output
- b9m2.gcs: staff-label variables not recognized in lilypond/horizontal modes (~13,421 errors)
- LilyPond multi-voice: only assigns `\voiceOne` and `\voiceTwo` direction; voices 3+ get no direction
- ABC "Bad tie" on chord-to-single-note ties (see Bug Fix History above)
- `make all` in top-level Makefile hits pre-existing `abcm2ps` segfault on `b/09/b9m1.abc` (unrelated to imscomp)

### Top-Level Makefile (`/home/m4/LEARNING/music/Makefile`)
The top-level Makefile has `all` and `clean` targets that recurse into all 5 DIRS:
`musicomp2abc songs ims t b`. Uses `@for d in ${DIRS}; do $(MAKE) -C $$d <target>; done`.

### Key Test Files
| Path | Description |
|------|-------------|
| `b/06/b-6.gcs` | Beethoven 6th (largest test file, 21 voices, 12698 lines, good for profiling) |
| `b/02/21/v2-1.gcs` | Voice command test (uses `voice` directive) |
| `b/02/21/v3-4.gcs`, `v4-1.gcs`, `v4-2.gcs`, `v4-3.gcs` | Additional voice tests |
| `b/15/inv15-example.gcs` | Inversion example test |

---

## Beethoven Compositions Reference

### Moonlight Sonata (`b/sonata14/s14.gcs`)
- Op.27 No.2, 1st movement (Adagio sostenuto)
- 6 voices, 3 staves (treble triplets, treble melody, bass)
- Score PDF: `b/sonata14/beethoven_-_piano_sonata_no.14_op.27_no.2.pdf`
- Key dynamics: `vol(pp)` at beginning (CC 11=40), `cresc(mf,2.0)` at m25, `dimin(p,1.0)` at m27
- Total duration: ~139.5 seconds (2.3 minutes)
- .fs file size: ~6,758 lines

### Symphony No. 9 Mvt 2 (`b/09/b9m2.gcs`)
- 35 voices, 19 staves, transposing instruments (horns in D, trumpets in D, clarinets in C)
- Includes `b9m2.starting` which includes `instruments.include`
- Total duration: ~854.5 seconds (~14 min 14 sec)
- .fs file size: ~98,962 lines

---

## .gcs Input Format Basics

```
title   Symphony No. 9 Movement 2
meter   3/4
measure 1
key     f                           $$ global key
key     HorninDV14 c                $$ per-staff key
tempo   116,2d                      $$ tempo: speed, note-length
clef    FluteV1 treble
staff   FluteV1: 1,2                $$ staff label: voice numbers
xpose   HorninDV14 -7               $$ transposition in semitones
#include "instruments.include"      $$ file inclusion (via CPP)
FluteV1: vol(ff) 5d4d 4d8 4d4      $$ staff-label: notes
1: vol(ff) 5d4d 4d8 4d4            $$ voice-number: notes
vol(pp)                             $$ volume change
cresc(mf,2.0)                       $$ crescendo to mf over 2 beats
dimin(p,1.0)                        $$ diminuendo to p over 1 beat
bars    10                          $$ bar lines per system
page    45                          $$ page number annotation
* comment                           $$ comment line
$$ inline comment                   $$ inline comment
```

## Octave Handling

- `middle_c = None` (default): standard, middle C = MIDI 60 = internal `3c`
- `middle_c = 51` (staff format, set by `middlec` directive): middle C = internal `4c`
- When `middle_c == 51`, `getnote()` applies `o = o - 1` at line 15419 for display formats
- Staves output adds `+1` back at ~line 4600: `octave = int(n[0]) + 1`
- LilyPond `_ly_note_name()` maps: internal octave 4 = LilyPond `c'` (middle C)

---

## Session Logs

Detailed session transcripts are saved in `/home/m4/LEARNING/`:
- `DEVIN-2026-03-23.md` - FluidSynth WAV rendering from stdin implementation

Devin CLI session summaries are saved in `/home/m4/.local/share/devin/cli/summaries/`:
- `history_c0fe88301fb046f3.md` - Voice bug fix, Makefile, Bad tie analysis, minor fixes, profiling & optimization
- `history_004649c9295f4037.md` - Sound quality features (default duration, velocity, humanization, arpeggio, sustain pedal)
- `history_a5733283be694314.md` - Comprehensive code audit & 21 bug fixes (Batches 1-4)
- `history_6c6ac476eb4b4356.md` - Subagent bug hunts, velocity/volume humanization, 20+ bug fixes, calculate.py fixes

### Session 5 Changes (history_6c6ac476eb4b4356, first part)

#### Subagent Bug Hunt Round 1 (4 real fixes, 2 false alarms)
1. **Line 18116** (CRITICAL): `for ls in last_voice_staff:` → `for ls in save_last_voice_staff:` — loop iterated over empty list (just reset at line 18115), so `set_process_begin()` quit logic was dead
2. **Line 3531** (HIGH): `mmm in clef and j in clef[mmm]` → `j in clef and mmm in clef[j]` — operands swapped; `clef` keyed by voice not measure, so condition never matched
3. **Line 3675** (MEDIUM): Added `len(staff_name[st]) > 0` check in `firstvoiceinstaff()` — empty lists possible from `staff_name[sn] = []` at line 13341
4. **Line 3720** (cosmetic): `clef [voiceon]` → `clef[voiceon]` — extraneous space

#### Velocity & Volume Humanization (NEW FEATURE)
- `humanize_velocity` GCS variable (default 3, range 0-127): max random velocity variation per note
- `humanize_dynamics` GCS variable (default 2, range 0-127): max random CC11/CC7 variation per note
- Cached at line ~5938: `_humanize_vel_max` and `_humanize_dyn_max`
- Applied in MIDI output (line ~6163+): velocity gets `random.randint(-max, max)` (clamped velocity_min..127); CC11/CC7 similar
- Controlled by existing `--nohumanize` flag (disables all: timing, velocity, dynamics)

#### Additional imscomp Bug Fixes
- **Lines ~16401, ~16475**: `crd` undefined in pan/intensity cresc/dimin error messages → replaced with `'cresc'`/`'dimin'`
- **print_die()**: Called 23 times but never defined → added `def print_die(msg)` at ~line 1265
- **Line 14621**: Bare `return` in `handle_vars_parens()` → `return wary, None` for tuple consistency

#### calculate.py Fixes & Refactoring
- **Line 1226**: `return [[ "ERROR..." ]]` → `return [ "ERROR..." ]` — double-nested list
- **Line 1514**: Error said "two arguments" but `f_in()` requires three → fixed
- **Line 981**: `parse_to()` bare string error return → wrapped in `[ ..., None ]` list
- **m1-m100**: Replaced ~210 lines of repetitive `arrays.append(['mN'...])` with 4-line loop

### Session 6 Changes (history_6c6ac476eb4b4356, second part)

#### Subagent Bug Hunt Round 2 (9 imscomp fixes, 6 calculate.py fixes)

**imscomp fixes:**
1. **Line 6901** (CRITICAL): `pre_move_note_before(prev_m, m, voice, s, vel, n, a, v, p)` missing `intens` param — 9 args but function expects 10, would TypeError at runtime
2. **Line 6630** (CRITICAL): `error_now = error_now + 1` — variable never defined anywhere, would UnboundLocalError
3. **Line 9433** (HIGH): `{wnote}` → `{w_note}` — wrong variable name in error message, would NameError
4. **Line 9442** (MEDIUM): `therest and therest` → `therest` — redundant duplicate condition
5. **Lines 16423/16426** (HIGH): Pan time calculation had vertical/midi assignments SWAPPED vs crescendo/intensity pattern — vertical should use `new_second + 0.0`, midi should use `round(new_second * MIDICLICKSPERQUARTER * 4)`
6. **Lines 16604/16638** (HIGH): `therest[0]` accessed after `therest = therest[1:]` without checking empty — would IndexError in trill parsing
7. **Line 13141** (HIGH): `wargs[1]` accessed after only checking `len(wargs) < 1` — should check `< 2`, would IndexError with single argument
8. **Line 14392** (MEDIUM): `vn = get_volume_number(...)` could return None, then used in `set_volume_now` without check — added `if vn is None: return`
9. **Lines 4095/4453/4614/5579** (LOW): `val in instruments.values()` compared string to list-of-lists (always False); simplified to `val in instruments` (key lookup)

**calculate.py fixes:**
- **Lines 1138/1140/1148/1152/1158/1166**: Added missing `, None` to 6 error returns in `result_functions()` — single-element error lists weren't caught by `len(args) > 1` check in `parse_to()`

**IMPORTANT: calculate.py return convention** — The Pratt parser uses `args = evaluate_handle(args)` where eval functions must return `[[type, value]]` (single-element list wrapping `[type, value]`). Error returns are `['ERROR...', None]` (two-element list). Do NOT remove the double-nesting — it is intentional.

---

## Profiling imscomp

### How to Profile
```bash
# Best approach: use subprocess wrapper to separate profile from program output
cat > /tmp/profile_b6.py << 'PYEOF'
import cProfile, pstats, io, sys, os, subprocess
os.chdir('/home/m4/LEARNING/music/b/06')
result = subprocess.run(
    ['python3', '-m', 'cProfile', '-s', 'tottime',
     '/home/m4/LEARNING/music/ims/imscomp', '--abc', 'b-6.gcs'],
    capture_output=True, text=True, timeout=120)
for line in result.stdout.split('\n'):
    if 'function calls' in line:
        # Print from this line onward (the profile table)
        idx = result.stdout.index(line)
        print(result.stdout[idx:idx+3000])
        break
PYEOF
python3 /tmp/profile_b6.py
```

### Wall-clock Timing
```bash
cd /home/m4/LEARNING/music/b/06
time python3 /home/m4/LEARNING/music/ims/imscomp --abc b-6.gcs > /dev/null 2>&1
```
Note: exit code 11 is normal for `--abc` (it's the warning count).

### Current Profile Hotspots (after optimization, 2026-03-27)
Top functions by tottime on b-6.gcs (1.3s total, 3.2M calls):
1. `do_notes_oldway` 0.09s (10763 calls)
2. `do_measure` 0.09s (514 calls)
3. `re.Pattern.sub` 0.08s (147K calls)
4. `getnote` 0.07s (21K calls)
5. `put_on_bufs` 0.06s (18K calls)

### Future Optimization Opportunities
- `do_notes_oldway` / `do_measure` are the main parsing loops; hard to optimize without restructuring
- 147K regex `sub` calls — could pre-compile or batch string replacements
- `find_terminator_in_string` (18K calls, 0.05s) — string scanning; potential for optimization
- `separate_args` (11K calls, 0.03s) — argument parsing
- `len()` still called 575K times — some could be cached in hot loops
