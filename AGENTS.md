# AGENTS.md

Single git repo (`m4`). All active work lives in `music/` — a music transcription/compilation pipeline. Everything else at this level (`kernel/`, `m4-clfs/`, `testing/`, `misc/`, ...) is unrelated personal/Linux experiments; ignore unless asked.

## Pipeline

`.gcs` (score source) → `.E` (GCC C preprocessor) → `.abc`/`.csv`/`.fs`/`.v`/`.h` (compiler output) → `.mid` (csvmidi) / `.ps` (abcm2ps) / `.pdf` (ps2pdf) / `.wav`+`.mp4` (fluidsynth + ffmpeg).

- `.gcs` files use `#include`/`#ifdef`/macros; the `.E` step is real CPP: `gcc -E -x c -nostdinc -C -CC -traditional-cpp x.gcs -o x.E` (flags vary slightly per Makefile).
- Always run compilers from the piece's own directory — `.gcs` `#include`s are relative.
- `.E`, `.fs`, `.mp4`, etc. are build artifacts; `make clean` removes them.

## Two compilers (single Python files — must stay in sync)

- `music/ims/imscomp` — actively developed compiler (~20.9k lines).
- `music/musicomp2abc/musicomp2abc` — older name, formerly a byte-identical copy. **The SFZ feature (`--sfzpipecsv` in imscomp, ~21k-line single file) is deliberately NOT mirrored** (precedent 938141ca/4151c3e8); DOALL keeps bare-diff at 0. `ims/calculate.py` ↔ `musicomp2abc/calculate.py` must still stay mirrored.
- `ims/calculate.py` — expression parser used by the compilers.

Suffix convention: `foo.fs/.abc/.csv` = musicomp2abc output; `foo_2.fs/_2.abc/_2.csv` = imscomp output. Piece Makefiles build both.

## Commands

**`music/BUILD.md` is the build quick reference — read it before running a build.** The two commands that settle most questions are `make help` (works in `music/` and in every piece dir; lists targets and what each does) and `make setup` (read-only check of tools + sample libraries; currently `0 missing`). Do not go hunting through the tree to decide whether a tool or the VPO library is missing — `make setup` answers that.

- Per-piece dirs (`music/b/01`, `music/b/09`, `music/songs`, `music/t/e`, `music/ims`, ...): `make fs` / `make abc` / `make pdf` / `make mp4` / `make -B b9m2_2.fs` (force rebuild one imscomp output). Makefiles enumerate `SONGS`/`TESTS` names.
- Top-level `music/`: `make all | mp4 | mp4_2 | sfz | sfz-mp4 | sfz-clean | clean`. `all` recurses into `musicomp2abc songs ims ims/tc-testing/gershwin t b`; `mp4`/`mp4_2` into `musicomp2abc songs t b`; `sfz`/`sfz-mp4`/`sfz-clean` into `songs ims t b` (**not** `musicomp2abc`).
- **`make all` does not do SFZ** — `sfz` is a separate target. For both, run `make all` *and* `make sfz`.
- **Top-level `make all` exits non-zero by design**: the 2 known gershwin `new-g3` named ARGHs (`pete.E`, `new-g3.ps`). Not a regression. `abcm2ps` "Length incompatible with BASE" warnings on `e.abc` also recover. Don't grep logs for `error` without filtering `-Werror` out of the `gcc -E` lines first.
- `./create-mp4` — builds all 25 YouTube `.mp4`s in parallel.
- `make sfz` top-level is a long job (VPO, 7 desks per instrument) — background it to a log rather than blocking.

## Tests (differential regression — the real test suite)

- `cd music/ims && ./DOALL` runs 5 suites in parallel (songs/, musicomp2abc/, b/, t/e/, tc-testing/gershwin/). Each `AAA.diff._2` compiles every listed `.gcs` with BOTH compilers across 5 formats (`--vert --hori --csv --fs --abc`) and diffs outputs.
- **bare ARGH** = imscomp-vs-musicomp2abc output difference; **named ARGH** = execution failure. Baseline as of 2026-09-27: **0 bare / 2 named**. Any increase in either is a regression. The 2 named are the known pre-existing `gershwin new-g3.E` failures — both compilers fail identically, so they are not a compiler-mirror problem.
- One suite: `cd music/b && ./AAA.diff._2`. Single compiler check: `cd music/ims && make tests`. (`ims/RUN-Tests` is older and references a stale `~/musicomp2abc/...` path — prefer DOALL.)

### Diagnosing an audio bug (don't reason about the synth — measure it)

DOALL only proves the two compilers agree. It says nothing about whether the output is *audible*, and both compilers can be identically wrong. For anything in the note/CC/pedal/legato paths, build a micro-benchmark and measure per-note RMS:

```sh
# ~4 notes, 2 bars, one voice, on acoustic grand -- 2 seconds of render
cat m.gcs > ...; gcc -E -x c -nostdinc -C -CC -traditional-cpp m.gcs -o m.E
./imscomp --fluidsynth m.E m.fs          # run from the dir holding calculate.py
cat m.fs | fluidsynth -q -F /tmp/m.wav /Users/m4/src/GeneralUser/GeneralUser.sf2
ffmpeg -ss <note_onset> -t <dur> -i /tmp/m.wav \
  -af astats=metadata=1:reset=0,ametadata=print:key=lavfi.astats.Overall.RMS_level -f null -
```

A note that reads ~25 dB below its neighbours is silent; ~2 dB below is just a missing hammer strike. Always compare the *same* bar twice — bugs driven by `random.seed`-based humanize often hit one bar and not the next, which is what made the note-off-ordering bug look intermittent.

Traps, each of which has silently produced a wrong "measurement":

- **Never pipe fluidsynth's stdout into `head`.** The pipe closes early, fluidsynth dies of SIGPIPE mid-render, and you get a valid-looking 0.049 s WAV. Redirect to `/dev/null` instead.
- **To compare against an older compiler**, copy it *and* `ims/calculate.py` into a scratch dir together; `import calculate` resolves against the script's own directory, so running an extracted `imscomp` from `/tmp` dies with `ModuleNotFoundError`.
- **CSV times in `gcs2sfz` are fractional seconds, converted to integer ticks before the guards run.** Mixing units, or handing a float to `range()`, is easy and silent — see the bend-ramp `TypeError` fixed in cf435253.
- **Never judge an SFZ/ensemble change by sample-domain diffing of the mixed WAV.** `GCS2SFZ_ENSEMBLE` renders each desk separately and `amix`es them, so any pitch change in any desk shifts the whole waveform's phase. Block-RMS of `A-B` then reports ~85% of blocks "differing" for *any* pair of renders, including two that sound identical — the metric is saturated and carries no information. Prove the change at the CSV/note-event level (how many attacks were removed, which notes were grouped), and use your ears for the rest. Same trap as the SIGPIPE one above: a plausible number that measures nothing.


## Audio/video pipeline (works on the macOS box)

The full `make pdf` and `make mp4` pipelines now run on the current macOS (Apple Silicon) machine. All three audio/video tools live in `~/bin` (which is early on PATH), NOT in brew's `/opt/homebrew/bin`, to stay non-destructive to Homebrew-managed tools:

- `~/bin/fluidsynth` — **patched build** with the stdin→WAV-render feature that `gcs2youtube` depends on. Source: `sparse` checkout at `~/src/fluidsynth.m4-editing` (also `saved-m4-stuff/src/fluidsynth.m4-editing`), built with `cmake -S . -B build2 -DBUILD_SHARED_LIBS=OFF` (static) then copied into `~/bin`. The patch adds `fluid_cmd_handler_set_renderer()` + a renderer field/`pending_samples` on the cmd handler, makes `sleep` render audio frames instead of sleeping when a renderer is attached, and restructures fluidsynth.c's `-F` path to render stdin `.fs` commands (see `music/DEVIN-2026-03-23.md`); stock fluidsynth still fails with "No midi file specified!".
- `~/bin/ffmpeg` / `~/bin/ffprobe` — **source-built ffmpeg 9.0.1** (`/tmp/ffmpeg-9.0.1`) with `--enable-libass --enable-libfreetype --enable-libharfbuzz --enable-fontconfig --enable-libx264` (and x265/vpx/mp3lame/opus/videotoolbox/audiotoolbox). The brew ffmpeg 9.0.1 has **no text rendering** (`ass`/`drawtext`/`subtitles` filters missing), so `gcs2youtube`'s ASS overlay errored with "No option name near 'ass=…'"; this custom build provides those filters.
- `~/bin/ps2pdf` (+ `ps2pdf14`/`ps2pdfwr`/`gs`) — symlinks to brew ghostscript 10.07.1 so `make pdf` works even when `/opt/homebrew/bin` isn't on PATH.

- `.fs → .wav → .mp4` goes through `gcs2youtube` (`make mp4`), which pipes the `.fs` into fluidsynth: `cat foo.fs | fluidsynth -q -F out.wav <soundfont>`. The stdin-render mode is the custom patch above; stock fluidsynth fails with "No midi file specified!".
- SoundFont: `/Users/m4/src/GeneralUser/GeneralUser.sf2` (macOS) or `/home/m4/src/GeneralUser_GS/GeneralUser.sf2` (Linux).
- `mp4cat` concatenates `.mp4`s (multi-movement symphonies).

## SFZ render pipeline (macOS) — `make sfz`, `make sfz-mp4`

Second, higher-fidelity render path using the Virtual Playing Orchestra sample library via sfizz. Docs: `music/SFZ.md`.

- Chain: `.gcs` → `.E` (CPP) → **`imscomp --sfzpipecsv`** → per-instrument CSV (`start,dur,midi_note,velocity,pan,reverb,vol_start,vol_end,pan_end`) → **`music/sfz/gcs2sfz`** → one WAV per instrument → mixed `*-sfz-mix.wav` → `*-sfz.mp4`. Suffix rules: `foo.fs`/`foo_2.fs` belong to the GM path; SFZ outputs are `sfz-csv/…`, `sfz-mix/`, `*-sfz.mp4`.
- Instruments: VPO samples via `~/bin/sfizz_render` (pan = score CC10, reverb = CC91, offline `aecho` tail since `~/bin/ffmpeg`'s `afir` mutes dry audio); unmapped instruments fall back to GM via patched `~/bin/fluidsynth`; one-shot effects (1812 canon + church bells) play whole WAV sonics from `music/sfz/library/EFFECTS/` (`canon.wav`, `church-bells.wav` fetched — `setup.sh --effects`).
- Commands: `make sfz` (= WAV mixes) / `make sfz-mp4` / `make sfz-clean` at each piece dir or the top level (`music/` recurses `songs ims t b`). `make sfz-mp4` **requires `~/bin` before `/opt/homebrew/bin` on PATH** — the brew ffmpeg has no `drawtext`, so the overlay (and `gcs2sfz`'s `shutil.which("ffmpeg")`) must resolve to `~/bin/ffmpeg`.
- `gcs2sfz` env knobs: `GCS2SFZ_LEAD_IN` (default **0** — the sfz-mix must stay time-aligned with the `.fs` overlay timeline, so a positive lead-in would misplace the scrolling measure text) and `GCS2SFZ_TAIL` (default `6.0` s of silence appended after the longest stem; trim to the actual reverb ring-out for short slices by exporting a smaller value). Articulation: `GCS2SFZ_ARTIC_MAX` (default `0.5`), `GCS2SFZ_ARTIC_BLEND` (default `1`), `GCS2SFZ_ATTACK` (default `0.03`); `GCS2SFZ_GAIN` (`"name:db,name:db"`) boosts quiet stems per instrument. Expression: `GCS2SFZ_EXPR_EXP` (default `4.0`) — `write_midi` predistorts CC11 as `127*(v/127)^E` on the sfizz path so VPO (linear CC11) matches GM's (exponential CC11) fp→p contrast; the GM fallback (`pred_expr=False`) skips per-note CC11 entirely since the CSV merges all voices onto one MIDI channel (fixed the Chopin "stutter" ~26s in — cross-voice CC11 pumping). The fallback instead folds expression into note velocity as `vel*(vol/127)^GCS2SFZ_GM_EXPR` (default `1.3`) and runs fluidsynth with `-g GCS2SFZ_GM_GAIN` (default `0.5`, matching the `.fs`'s `synth.gain 0.5`; fluidsynth's own default 0.2 made fallback stems ~8 dB quiet) — without both, GM-fallback pieces were quiet with velocity-only dynamics (vol(mp) too loud, vol(ppppp) audibly ringing; calibrated on `ims/test-volume-levels`).
- Waveform overlay `music/wav2waveform` (run by `gcs2youtube` for BOTH GM and sfz mp4s) draws a cyan loudness scale: dashed lines + small `#dB` labels at `--db-markers "-6,-12,-18,-24"` dBFS (2 px thick for 4:2:0); `--db-markers ""` disables.
- Full-sweep status: all SONGS/ims/t/e/b pieces render. `songs` `SONGS_NO_PS` (`inv1 vinci sonata macros-complicated`) are excluded from sfz by design.
- DEVIN notes for the SFZ/pipeline work — read on a need-to-know basis when touching that path (SFZ.md's "next fixes" list is the current top):
  - `music/DEVIN-2026-09-03-sfz.md` — reviewed SFZ implementation plan + constraints (sfizz_render, 16-channel limits, one-shots).
  - `music/DEVIN-2026-09-11-sfz-articulation.md` — articulation recipe defaults + open b/01 v1-1 first-measure pizzicato bug.
  - `music/DEVIN-2026-09-11-sfz-leadin-waveform.md` — `GCS2SFZ_LEAD_IN/TAIL` timeline alignment + wav2waveform dB scale.
  - `music/DEVIN-2026-09-15-cc11-sfz-loudness.md` — CC11 predistortion (`GCS2SFZ_EXPR_EXP`) + GM-fallback stutter fix; gain sync (`GCS2SFZ_GM_GAIN`) and expression-in-velocity (`GCS2SFZ_GM_EXPR`) calibrated on `test-volume-levels`; make-clean re-render memo.
  - `music/DEVIN-2026-09-19-sfz-ensemble.md` — stage/ensemble simulation: 11th CSV `staff` column; per-desk detune (each desk internally in tune via channel-wide pitch-bend at tick 0 + re-zero, desks spread over ±4 cent via `GCS2SFZ_ENSEMBLE_DETUNE`, never two desks on the same tuning) + per-staff pan spread (`GCS2SFZ_ENSEMBLE_PAN`), each desk rendered as its own stream and amixed; `GCS2SFZ_ENSEMBLE=0` disables.
  - `music/DEVIN-2026-09-20-b9m2-horn-imbalance.md` — RESOLVED: b/09 b9m2 horns overwhelmed mm 51-58; m45 `vol(p)<` + late-placement `vol<` made horns/clarinet ramp to ~ff by ~m49 while orchestra stayed ~pp until m57 `ff`. Fixed in b9m2.gcs: two `vol(p)<`->`vol<`, `vol<` moved to line head, HorninBV16 + Timpani got lead-ins, redundant m46 `vol<` dropped.
  - `music/DEVIN-2026-09-27-legato-slur.md` — legato/slur work: `l` semantics, CC68 footswitch span, `--priming`, SFZ slur-by-pitch-bend, and the measured fusion payoff. READ THIS before touching `'l'` handling in either compiler or `_collapse_legato_groups` in gcs2sfz. The short version: **`l` on a note means "glide to the NEXT note — no attack at its onset"**, so put `l` on the notes that slur, NOT on the last note of a run: in `c4 e4l f4l g4` the group is `e f g` and `c` is struck fresh. On the GM path the pedal is re-armed ~1–31 ms into each slurred-in note and the previous note's note-off is pinned just before the new onset — fluidsynth treats CC68 as monophonic, so holding it across a run kills the notes and a late note-off silences the note just started. Both compilers must stay mirrored on all of it. **Since 2026-10-10 the GM slur-in soft onset is VELOCITY-driven: `slur_velocity=55` (a lower-velocity re-strike keeps ADSR timing but sits the whole note ~9-11 dB under a fresh one — measured uniform across pp..ff on the GeneralUser grand), with the NRPN FILTERFC onset-filter (`slur_filter`, default 0 = off) as an optional extra. Since 2026-10-12 `slur_velocity` is ADAPTIVE: `slur_velocity_decay=0.65` (shipped) multiplies the slur-in velocity by this for each extra 0.5 s the SOURCE note SOUNDED beyond 0.5 s (real time and tempo-aware, score ticks converted via `tempo t,l`), so long-held slur sources get a proportionally quieter continued note — measured per-note, the slur-in body now lands at the source's decayed seam level at mp..pp (1.0 = flat, 0.8 mild, 0.55 aggressive). `--sfzpipecsv` is re-pinned to the old `slur_velocity=45`/no-filter/**`slur_velocity_decay=1.0`** in imscomp's SFZ dispatch — never change that pin without re-verifying gcs2sfz. **Since 2026-10-12 there are TWO slur profiles:** the piano/default decaying stroke keeps `slur_velocity=55`+`slur_velocity_decay=0.65`, while instruments that HOLD their level while sounding (bowed strings, winds, brass, voices — the `sustained_slur_instruments` set) use `sustained_slur_velocity=100`+`sustained_slur_decay=1.0`: a bow/breath continues, so its slur-in is NOT dipped at all — the piano-tuned 55% sat the violin slur-in −14..−16 dB under a fresh note (measured, read as "way too quiet"), 100 puts it at fresh level at every dynamic and holds it at any source length. The violin probe (`ims/test-legato.gcs`) pins 100/1.0 to match the shipped default. `--sfzpipecsv` also pins `sustained_slur_velocity=45`+`sustained_slur_decay=1.0` so the shared SFZ CSV is unchanged. On the SFZ/VPO side, `gcs2sfz` has the matching `_SUSTAINED_SLUR_INSTRUMENTS`/`_sustained_slur_levels`: a sustained slur-in is rendered ONLY on the slow-attack stem (the fast stem excludes slur-in marks), so its pinned ~45% cut left it −8.7..−9.6 dB under fresh; gcs2sfz now re-raises the slur-in velocity to its SOURCE note's and drops the slow-stem +3 dB piano compensation for sustained instruments, landing at ~0 dB (measured, DEVIN-2026-10-12-sustained-slur.md). On the SFZ side, `gcs2sfz` re-applies the adaptive decay on the **GM fallback only** (`GCS2SFZ_LEGATO_VEL_DECAY`, default 0.65, mirror of `slur_velocity_decay`; source sounded-duration from the CSV's absolute onset seconds, floored at the compilers' velocity_min 20) so the SFZ piano reaches the same seam level as the `.fs` render; the VPO/sfizz path keeps the flat 45% CSV and its own slow-attack handling (other instruments = later). Detail: `DEVIN-2026-10-05-legato-piano-velocity.md` + `DEVIN-pre-knowledge.md` "GM slur soft onset".**
  - `music/DEVIN-2026-10-12-sustained-slur.md` — sustained-instrument (bowed strings, winds, brass) slur-in level: the piano-tuned `55`/`0.65` sat the violin slur-in −14..−16 dB under a fresh note (measured sweep, all dynamics); new `sustained_slur_velocity=100`+`sustained_slur_decay=1.0` profile (100 = no dip, holds at any source length), picked per-instrument via `sustained_slur_instruments`; `--sfzpipecsv` pins `45`/`1.0`. SFZ/VPO mirror: `gcs2sfz` `_SUSTAINED_SLUR_INSTRUMENTS`/`_sustained_slur_levels` re-raise the slow-stem-only slur-in velocity to the source's and drop the +3 dB compensation → ~0 dB (was −9).
  - `music/AAA.README.md` — differential-test suite HOWTO for `AAA.diff._2`/DOALL.

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
