# DEVIN-gcs.md - imscomp Project Reference

Background reference for AI assistants working on the `ims/imscomp` music compiler.
Load this file at the start of a session: "read DEVIN-gcs.md for background".

## Project Overview

`ims/imscomp` is a single-file (~20,112 lines) Python music compiler that reads `.gcs`
(and `.E`) input files and produces multiple output formats: ABC notation, horizontal,
vertical, staves, LilyPond, MIDI CSV, and FluidSynth.

The file lives at `/home/m4/LEARNING/music/ims/imscomp`.

## How to Run Tests

### DOALL (full regression test)
```bash
cd /home/m4/LEARNING/music/ims && bash DOALL
```
Expected result: `ALL OKAY!` at the end. CSV and FS diffs (ARGH markers) showing
Control_c ordering differences are pre-existing and acceptable.

DOALL runs test suites in: `songs/`, `musicomp2abc/`, `b/`, `t/e/`, `tc-testing/gershwin/`.
Each directory has `AAA.diff._2` script and a `Makefile`.

### Testing with b9m2 (Beethoven 9th, Mvt 2)
**IMPORTANT**: b9m2.gcs uses `#include` directives with relative paths. You MUST
run from its directory:
```bash
cd /home/m4/LEARNING/music/b/09
python3 ../../ims/imscomp --staves b9m2.gcs 2>/dev/null | head -80
python3 ../../ims/imscomp --lilypond b9m2.gcs 2>/dev/null | head -80
python3 ../../ims/imscomp --horizontal b9m2.gcs 2>/dev/null | head -80
```
b9m2.gcs has 35 voices, 19 staves, and transposing instruments (horns in D,
trumpets in D, clarinets in C). It includes `b9m2.starting` which includes
`instruments.include`. The `.E` file is a generated intermediate, not a source file.

**Known issue**: b9m2.gcs currently produces ~13,421 errors about unrecognized
staff-label variables in lilypond/horizontal modes. The staff-label syntax
(`FluteV1: r2d`) is parsed correctly in staves mode but not fully in other modes.

### Testing with Gershwin g3 (transposing instruments)
```bash
cd /home/m4/LEARNING/music/ims/tc-testing/gershwin
python3 ../../../ims/imscomp --lilypond g3.gcs 2>/dev/null | head -80
```
g3.gcs has transposing instruments:
- Bb clarinet (xpose -2), written key G when concert key is F
- F horn (xpose -7), written key C when concert key is F
- Bb trumpet (xpose -2), written key G when concert key is F
- Flute (no xpose), key F

### Testing with theme.gcs (simple, 2 staves)
```bash
cd /home/m4/LEARNING/music
python3 ims/imscomp --lilypond ims/tc-testing/theme.gcs 2>/dev/null
python3 ims/imscomp --staves ims/tc-testing/theme.gcs 2>/dev/null
```

## Key Function Locations (line numbers as of 2026-03-22)

| Line  | Function / Section |
|-------|-------------------|
| 3917  | `def print_measure_vh_header(` - per-measure metadata for vertical/horizontal |
| 4295  | `def print_measure_staves_header(` - per-measure metadata for staves (Feature #3) |
| 4472  | `def print_out_staves(` - staves output (Feature #3) |
| 4745  | `def _ly_note_name(` - convert internal note format to LilyPond notation |
| 4804  | `def _ly_duration(` - convert internal duration to LilyPond duration |
| 4852  | `def print_out_lilypond(` - LilyPond output function |
| 5404  | `def print_header(` - header output for all formats |
| 14778 | `middle_c = None` - middle_c default initialization |
| 14863 | `def getnote(` - main note-processing function (~600 lines) |
| 15203 | Key conversion section ("Convert for key") |
| 15253 | ABC else/pass path for key conversion |
| 15317 | Xpose condition (recently modified for Feature #4) |
| 15407 | ABC else/pass path for xpose |
| 15412 | Note format conversion to `o+n+a` format |
| 15419 | `o = o - 1` octave adjustment when `middle_c == 51` |
| 19252 | `def main(` |

## Output Format Flags

The code uses `args.*` flags to branch behavior. There are ~84 occurrences of
`args.lilypond` in the file. Many locations group lilypond with other display formats:

```python
args.horizontal or args.vertical or args.staves or args.lilypond
```

The key groupings at format-selection points:
- **Buffer init, save/restore, crescendo formatting**: lilypond correctly grouped with vert/horiz/staves
- **Xpose (line 15317)**: lilypond REMOVED (Feature #4) - now falls through to ABC `pass` path
- **Key conversion (line 15203)**: lilypond still grouped - uses written key (correct since xpose is skipped)
- **Note format conversion (line 15412)**: lilypond still grouped - converts to `o+n+a` format needed by `_ly_note_name()`

## Internal Note Format

Notes stored in `bufs[voice][measure]` as strings like:
- `'4c'` = octave 4, C natural
- `'3d+'` = octave 3, D sharp
- `'5b-'` = octave 5, B flat
- Can have prefixes: `vol(90)`, `p(64)`, `in(100)`, `<`, `>`
  - `vol(N)` = per-note volume change (stays in output, not metadata to strip)
  - `p(N)` = pan change on note
  - `in(N)` = intensity
  - `<` / `>` = crescendo/diminuendo markers

## Key Data Structures

```
bufs[voice][measure]        - list of note strings
ba.length[voice][measure]   - list of note durations
ba.suffixes[voice][measure] - list of suffix strings ('t' for tie, etc.)
ba.volume[voice][measure]   - list of volumes
volume[voice][measure]      - volume value (0-127)
pan[voice][measure]         - pan value (0-126)
intensity[voice][measure]   - expression/intensity value
reverb[voice][measure]      - reverb value
xpose[voice]                - semitone transposition (via staff array)
xpose_new_key[voice]        - new key after transposition
key_voice[voice]            - current key for each voice
staff_name[label]           - maps staff label to list of voice numbers
clef[voice]                 - clef for each voice
vinstrument[voice]          - instrument for each voice
middle_c                    - None default, set to 39+12=51 for staff format
meas[]                      - ordered list of measure identifiers
```

## Octave Handling

- `middle_c = None` (default): standard, middle C = MIDI 60 = internal `3c`
- `middle_c = 51` (staff format, set by `middlec` directive): middle C = internal `4c`
- When `middle_c == 51`, `getnote()` applies `o = o - 1` at line 15419 for display formats
- Staves output adds `+1` back at ~line 4600: `octave = int(n[0]) + 1`
- LilyPond `_ly_note_name()` maps: internal octave 4 = LilyPond `c'` (middle C)

## Feature History

### Feature #3: --staves output (COMPLETE)
- Per-staff metadata via `print_measure_staves_header()` (line 4295)
- Staff-based output via `print_out_staves()` (line 4472)
- Header: per-staff declarations (staff, clef, key, volumes, instrument, pan, intensity, reverb)
- `middlec` directive when `middle_c != 51`
- Round-trip tested: theme.gcs near-perfect, b9m2 acceptable diffs
- DOALL: ALL OKAY!

### Feature #4: --lilypond written pitch (COMPLETE)
- Removed `args.lilypond` from xpose condition at line 15317
- Lilypond now falls through to ABC `else: pass` path, skipping transposition
- Notes stored as written pitch (not concert pitch)
- Keys preserved as written keys
- Verified with Gershwin g3.gcs transposing instruments
- DOALL: ALL OKAY!

### Known Remaining Issues
- LilyPond: `v,,,` artifacts may appear in output (mentioned in earlier sessions, needs investigation)
- b9m2.gcs: staff-label variables not recognized in lilypond/horizontal modes (~13,421 errors)
- LilyPond multi-voice: only assigns `\voiceOne` and `\voiceTwo` direction; voices 3+ get no direction

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
#include "instruments.include"      $$ file inclusion
FluteV1: vol(ff) 5d4d 4d8 4d4      $$ staff-label: notes
1: vol(ff) 5d4d 4d8 4d4            $$ voice-number: notes
bars    10                          $$ bar lines per system
page    45                          $$ page number annotation
pitchbend                           $$ enable pitch bend
%%staves [ ( 1 2 ) ( 3 4 ) ]       $$ ABC stave grouping
* comment                           $$ comment line
$$ inline comment                   $$ inline comment
```

## File Layout Quick Reference

| Directory | Contents |
|-----------|----------|
| `ims/imscomp` | The compiler (single Python file) |
| `ims/DOALL` | Full regression test script |
| `ims/tc-testing/theme.gcs` | Simple 2-staff test file |
| `ims/tc-testing/gershwin/` | Gershwin orchestral test files (g3.gcs, g21.gcs, g22.gcs) |
| `b/09/b9m2.gcs` | Beethoven 9th Mvt 2 (35 voices, 19 staves, transposing instruments) |
| `b/09/instruments.include` | Instrument definitions for Beethoven |
| `songs/` | Collection of song test files |
| `musicomp2abc/` | ABC-related test files |
