# Music Project -- Directory and File Guide

## Overview

This project contains orchestral and piano music transcribed in GCS notation
(era 1975-1978), compiled to ABC, MIDI CSV, FluidSynth, and YouTube-uploadable
MP4 formats.  Two compilers exist: the original `musicomp2abc` and the newer
`imscomp` (IMS chord format, late 1978).

---

## Top-Level Makefile

The root `Makefile` drives builds across all subdirectories.

| Target    | What it does                                                     |
|-----------|------------------------------------------------------------------|
| `all`     | Build everything (ABC, MIDI, FluidSynth) in all subdirectories.  |
| `mp4`     | Create `.mp4` YouTube videos in `songs/`, `t/`, and `b/`.       |
| `mp4_2`   | Create `_2.mp4` videos (imscomp variants) in the same dirs.     |
| `clean`   | Remove intermediate files (`.csv`, `.mid`, `.abc`, `.ps`, `.fs`, `.wav`). |

Subdirectories built: `musicomp2abc/`, `songs/`, `ims/`, `ims/tc-testing/gershwin/`, `t/`, `b/`.

---

## Directories

### `b/` -- Beethoven Symphonies and Sonata
    Contains subdirectories for individual Beethoven works, each with its own
    Makefile, `.gcs` source files, `.starting` setup files, and `*-Text` YouTube
    metadata files.

    | Subdir      | Piece                                   | Movements / Files              |
    |-------------|-----------------------------------------|--------------------------------|
    | `b/01/`     | Symphony No. 1                          | v1-1, v1-2, v1-3, v1-4        |
    | `b/02/`     | Symphony No. 2                          | b2m1, b2m2, b2m3, b2m4        |
    | `b/03/`     | Symphony No. 3 (Eroica)                 | v3-1, v3-2, v3-3, v3-4        |
    | `b/04/`     | Symphony No. 4                          | v4-1, v4-2, v4-3, v4-4        |
    | `b/06/`     | Symphony No. 6 (Pastoral)               | b-6 (single file)             |
    | `b/09/`     | Symphony No. 9                          | b9m1, b9m2, b9m3              |
    | `b/sonata14/` | Piano Sonata No. 14 (Moonlight)       | s14                            |

    Also contains `instruments.include` (shared instrument/macro definitions)
    and `instrument-placement/` (pan/seating reference).

    Symphonies 1, 2, and 6 use the older `putd`/`=macro` voice system
    (`musicomp2abc`).  Symphonies 3, 4, and 9 use the newer `staff` command
    system (`imscomp`).  Sonata 14 is piano only.

### `songs/` -- Short Pieces and Songs
Contains 111 `.gcs` source files for shorter pieces, including
    `promenade.gcs` (Mussorgsky, piano).  Has its own Makefile with targets
    for `.fs`, `.mp4`, ABC, MIDI, and PDF output.

### `ims/` -- IMS Compiler and Test Files
Contains:
    - **`imscomp`** -- The IMS music compiler (Python3, ~20,600 lines).
      Converts `.gcs` notation to ABC, MIDI CSV, FluidSynth, and other formats.
      Supports the `staff` command, `ensemble` duplication, `pitchbend`,
      crescendo/decrescendo, and many other features.
    - **`A-Flat-Prelude-Chopin.gcs`** -- Chopin A-flat Prelude (piano, 3 staves).
    - **Test files** -- Numerous `.gcs` test files (A0-A7, B3-B6, P1-P9, etc.)
      used by `RUN-Tests` / `AAA.run.tests` for regression testing.
    - **`tc-testing/gershwin/`** -- Gershwin Piano Concerto in F, 3rd Prelude
      (`new-g3.gcs`, full orchestra + piano, 75 voices).
    - **`calculate.py`** -- Pitch/frequency calculation utility.

### `musicomp2abc/` -- Original Music Compiler
    Contains:
    - **`musicomp2abc`** -- The original compiler (Python3, ~20,500 lines).
      NOTE: this an imscomp should be identical. :)
      Uses the older `putd`/`=macro`/`voice` system.  Predates `imscomp`.
      And the horizonal or vertical voice system, or the staff system.

### `t/` -- Tchaikovsky
    Contains:
    - **`t/e/`** -- 1812 Overture (`e.gcs`, 63 voices, full orchestra
      including brass band, cannon, and bells).  Uses the `putd`/`=macro`
      system (`musicomp2abc`).

---

## Scripts

### `create-mp4`
    Convenience script that runs `make mp4` in parallel across all piece
    directories:
    - `b/` (all Beethoven symphonies and sonata)
    - `songs/` (promenade)
    - `ims/` (Chopin prelude)
    - `ims/tc-testing/gershwin/` (Gershwin concerto)
    - `t/e/` (1812 Overture)

### `gcs2youtube`
    Converts a `.fs` FluidSynth command file to a YouTube-uploadable `.mp4`.
    This is the core rendering pipeline called by every Makefile's `%.mp4` rule.

    Pipeline:
    1. `.fs` -> `.wav` -- FluidSynth renders audio using a SoundFont (96 kHz).
    2. `.wav` -> waveform video -- `wav2waveform` generates a scrolling
       waveform visualisation (optional, 960x540, lower-right overlay).
    3. `.fs` -> `.ass` -- `fs2ass` generates scrolling echo-text subtitles
       (measure numbers, rehearsal marks, etc.).
    4. `.wav` + `.ass` + waveform -> `.mp4` -- `ffmpeg` composites everything
       into a 1920x1080 H.264/AAC video.

### `fs2ass`
    Generates an ASS (Advanced SubStation Alpha) subtitle file from a `.fs` file.
    Extracts timed `echo` lines and creates a scrolling-text overlay where the
    scroll rate adapts so that each echo reaches screen centre at the correct
    timestamp.  Persistent title text is displayed at the top of the frame.

    Called automatically by `gcs2youtube`.

### `fs2echolist`
    Extracts timed echo events from a `.fs` file and outputs a tab-separated
    list of `<seconds>\t<text>`.  A simpler alternative to `fs2ass` for
    debugging or other tools.

### `wav2waveform`
    Generates a centred-playhead waveform visualisation video from a `.wav`
    file.  The current playback position is bright green; past and future fade
    to black.  Output is a 960x540 H.264 video designed to overlay the
    lower-right quarter of a 1920x1080 frame.  Requires `numpy`.

    Called automatically by `gcs2youtube` when available.

### `mp4cat`
    Concatenates multiple `.mp4` files into a single video with configurable
    silent black-screen gaps between them.  Useful for combining multi-movement
    symphonies into a single upload.

    Usage: `mp4cat [-o output.mp4] [-g seconds] file1.mp4 file2.mp4 ...`

### `youtube-upload`
    Uploads `.mp4` files to YouTube using the YouTube Data API v3.  Reads
    metadata from companion `*-Text` files (title, description, playlist,
    privacy, license, category).

    Options: `--all` (upload everything), `--replace` (delete existing video
    by title match before uploading), `--dry-run`, `--notify`.

    Requires OAuth2 credentials in `~/.config/youtube/client_secrets.json`.

### `update-mp4-on-youtube`
    Convenience script that re-uploads all 25 pieces to YouTube using
    `youtube-upload --replace`.  Runs each upload sequentially.

---

## YouTube Workflow -- Order of Operations

### 1. Write/Edit the Music

Edit the `.gcs` source file and its `.starting` setup (staff definitions,
instrument assignments, ensemble declarations, clefs, keys, tempo, meter).

### 2. Compile to FluidSynth Format

The Makefile handles this automatically.  The chain is:

```
.gcs  -->  cpp  -->  .E  (C preprocessor resolves #include, #ifdef)
.E    -->  imscomp --fluidsynth  -->  .fs     (or musicomp2abc --fluidsynth)
```

Run `make fs` in the piece's directory, or just `make mp4` (which
depends on `.fs`).

### 3. Create the `.mp4` Video

```
make mp4          (in the piece's directory)
```

Or run `./create-mp4` from the project root to build all pieces in
parallel.

Under the hood, the Makefile rule calls `gcs2youtube <file.fs>` which:
1. Renders `.fs` to `.wav` via FluidSynth (96 kHz, SoundFont synthesis).
2. Generates scrolling echo text overlay (`.ass`) via `fs2ass`.
3. Generates waveform visualisation video via `wav2waveform`.
4. Composites audio + overlays into 1920x1080 `.mp4` via `ffmpeg`.

### 4. Write YouTube Metadata

Each `.mp4` needs a companion `*-Text` file in the same directory
(e.g., `b/09/b9m2-Text` for `b9m2.mp4`).  The format is:

```
Title:
<title line>
------------------------------------------------------------------------------
Description:
<description text>
------------------------------------------------------------------------------
Playlists: <playlist names>
Audience: Make for kids.
Visibility: Public
License: Creative Commons - Attribute
Category: Music
------------------------------------------------------------------------------
```

### 5. Upload to YouTube

Upload a single piece:
```
uv run --script ./youtube-upload <file.mp4>
```

Replace an existing upload (deletes old video by title match):
```
uv run --script ./youtube-upload --replace <file.mp4>
```

Upload/replace all 25 pieces:
```
./update-mp4-on-youtube
```

### 6. (Optional) Concatenate Movements

To combine multi-movement works into a single video:
```
./mp4cat -o symphony9.mp4 -g 5 b/09/b9m1.mp4 b/09/b9m2.mp4 b/09/b9m3.mp4
```

---

## Quick Reference -- Full Rebuild and Upload

```bash
# 1. Build all .mp4 files (parallel)
./create-mp4

# 2. Upload/replace all to YouTube
./update-mp4-on-youtube
```
