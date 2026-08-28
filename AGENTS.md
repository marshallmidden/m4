# AGENTS.md

Single git repo (`m4`). All active work lives in `music/` — a music transcription/compilation pipeline. Everything else at this level (`kernel/`, `m4-clfs/`, `testing/`, `misc/`, ...) is unrelated personal/Linux experiments; ignore unless asked.

## Pipeline

`.gcs` (score source) → `.E` (GCC C preprocessor) → `.abc`/`.csv`/`.fs`/`.v`/`.h` (compiler output) → `.mid` (csvmidi) / `.ps` (abcm2ps) / `.pdf` (ps2pdf) / `.wav`+`.mp4` (fluidsynth + ffmpeg).

- `.gcs` files use `#include`/`#ifdef`/macros; the `.E` step is real CPP: `gcc -E -x c -nostdinc -C -CC -traditional-cpp x.gcs -o x.E` (flags vary slightly per Makefile).
- Always run compilers from the piece's own directory — `.gcs` `#include`s are relative.
- `.E`, `.fs`, `.mp4`, etc. are build artifacts; `make clean` removes them.

## Two compilers (single Python files — must stay in sync)

- `music/ims/imscomp` — actively developed compiler (~20.9k lines).
- `music/musicomp2abc/musicomp2abc` — older name, currently a byte-identical copy of imscomp. **Mirror any imscomp change here** (and `ims/calculate.py` ↔ `musicomp2abc/calculate.py`); DOALL treats divergence as a regression.
- `ims/calculate.py` — expression parser used by the compilers.

Suffix convention: `foo.fs/.abc/.csv` = musicomp2abc output; `foo_2.fs/_2.abc/_2.csv` = imscomp output. Piece Makefiles build both.

## Commands

- Per-piece dirs (`music/b/01`, `music/b/09`, `music/songs`, `music/t/e`, `music/ims`, ...): `make fs` / `make abc` / `make pdf` / `make mp4` / `make -B b9m2_2.fs` (force rebuild one imscomp output). Makefiles enumerate `SONGS`/`TESTS` names.
- Top-level `music/`: `make all | mp4 | mp4_2 | clean` recurses into `musicomp2abc songs ims t b`.
- `./create-mp4` — builds all 25 YouTube `.mp4`s in parallel.

## Tests (differential regression — the real test suite)

- `cd music/ims && ./DOALL` runs 5 suites in parallel (songs/, musicomp2abc/, b/, t/e/, tc-testing/gershwin/). Each `AAA.diff._2` compiles every listed `.gcs` with BOTH compilers across 5 formats (`--vert --hori --csv --fs --abc`) and diffs outputs.
- **bare ARGH** = imscomp-vs-musicomp2abc output difference; **named ARGH** = execution failure. Baseline: 0 bare / 0 named. Any increase is a regression.
- One suite: `cd music/b && ./AAA.diff._2`. Single compiler check: `cd music/ims && make tests`. (`ims/RUN-Tests` is older and references a stale `~/musicomp2abc/...` path — prefer DOALL.)

## Audio/video needs a patched fluidsynth

- `.fs → .wav → .mp4` goes through `gcs2youtube`, which pipes the `.fs` into fluidsynth: `cat foo.fs | fluidsynth -q -F out.wav <soundfont>`. That stdin-render mode is a **custom patch** (documented in `music/DEVIN-2026-03-23.md`); stock fluidsynth fails with "No midi file specified!".
- Scripts default to `~/bin/fluidsynth` (patched build). The current macOS box lacks it, so `make mp4`/`make wav` won't run here; the text pipeline (abc/pdf/midi/fs) works with stock tools.
- SoundFont: `/Users/m4/src/GeneralUser/GeneralUser.sf2` (macOS) or `/home/m4/src/GeneralUser_GS/GeneralUser.sf2` (Linux).
- `mp4cat` concatenates `.mp4`s (multi-movement symphonies).

## YouTube uploads

- Each `.mp4` needs a sibling `*-Text` file (`Title:`/`Description:`/`Playlists:`/`Visibility:`/`License:`/`Category:` header format, e.g. `b/09/b9m2-Text`).
- Upload: `uv run --script ./youtube-upload [--replace] <file.mp4>` — needs OAuth2 at `~/.config/youtube/client_secrets.json`. `./update-mp4-on-youtube` re-uploads all 25.

## Score format conventions

- `.gcs` pieces `#include` a `.starting` setup file plus shared `b/instruments.include` (symlinked at `music/` root and `wt/`).
- Old format = `putd`/`=macro`/`voice`; new format = `staff`. Beethoven 1/2/6 and 1812 (`t/e`) are old-format; 3/4/9 and Gershwin are new-format. `wt/` shows auto-converting old→staff via `imscomp --staves --auto-staves`.

## Working in the compilers

- Each compiler is a single ~21k-line Python file; `python3 imscomp --help` lists flags. Key formats: `--abc --midi1csv --fluidsynth --vert --hori --staves --lilypond`; `--nohumanize`/`--noarpeggio` toggle realism features.
- MIDI/fs output uses `random.seed(42)` for deterministic humanization; CC 11/10/7 trackers (`last_cc11` etc.) must stay in sync with actually-emitted CCs (was a real bug — dynamics went silent).
- `music/DEVIN-pre-knowledge.md` is the detailed internal reference (data structures, function locations, bug history) — read it before editing the compilers. Its file paths (`/home/m4/...`) are from the original Linux box and stale here.
- **old putd/= → staff conversion:** read `music/DEVIN-2026-08-27-staves.md` first. TL;DR: the working generator is `imscomp --auto-staves --staves` run on the CPP-preprocessed `.E` file (NOT the raw `.gcs`); `convert_to_staff.py` is dead broken — it emits a per-voice suffix format imscomp rejects and should be deleted. A `do_xpose` KeyError in imscomp was fixed there (must stay mirrored in `musicomp2abc`); two merge bugs (wind -1 semitone transposition offset; legato-`l` recompile errors) were still open as of that note.
