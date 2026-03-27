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
| `ims/imscomp` | The compiler (single ~20,112-line Python file) |
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
Expected result: `ALL OKAY!` at the end. CSV and FS diffs (ARGH markers) showing
Control_c ordering differences are pre-existing and acceptable.

DOALL runs test suites in: `songs/`, `musicomp2abc/`, `b/`, `t/e/`, `tc-testing/gershwin/`.

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

### Key Function Locations (line numbers approximate, as of 2026-03-23)

| Line  | Function / Section |
|-------|-------------------|
| 3917  | `def print_measure_vh_header(` - per-measure metadata for vertical/horizontal |
| 4295  | `def print_measure_staves_header(` - per-measure metadata for staves |
| 4472  | `def print_out_staves(` - staves output |
| 4745  | `def _ly_note_name(` - convert internal note format to LilyPond notation |
| 4804  | `def _ly_duration(` - convert internal duration to LilyPond duration |
| 4852  | `def print_out_lilypond(` - LilyPond output function |
| 5404  | `def print_header(` - header output for all formats |
| 5743  | `def do_midi_vpi(` - CC 11/10/7 emission during crescendo/diminuendo |
| 5891  | `last_cc11 = -1` - CC tracker initialization per voice |
| 6086  | CC emission before Note_on (bug fix area -- see below) |
| 6280  | Second `do_midi_vpi()` call site (staccato handling) |
| 6462  | `change_name` dict - maps internal MIDI CSV names to FluidSynth commands |
| 6480  | `def print_out_fluidsynth()` - FluidSynth output generator |
| 14778 | `middle_c = None` - middle_c default initialization |
| 14863 | `def getnote(` - main note-processing function (~600 lines) |
| 15203 | Key conversion section ("Convert for key") |
| 15317 | Xpose condition |
| 15412 | Note format conversion to `o+n+a` format |
| 19252 | `def main(` |

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
volume[voice][measure]      - volume value (0-127)
pan[voice][measure]         - pan value (0-126)
intensity[voice][measure]   - expression/intensity value
reverb[voice][measure]      - reverb value
xpose[voice]                - semitone transposition (via staff array)
key_voice[voice]            - current key for each voice
staff_name[label]           - maps staff label to list of voice numbers
clef[voice]                 - clef for each voice
vinstrument[voice]          - instrument for each voice
middle_c                    - None default, set to 39+12=51 for staff format
meas[]                      - ordered list of measure identifiers
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
