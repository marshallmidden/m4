# Build & verify quick reference

Written after a session where re-deriving "which make target does the SFZ
render" was repeatedly annoying. **Read this before running any build.**

## 0. Two commands that answer most questions

```sh
make help     # in music/ or ANY piece dir -- lists targets and what each does
make setup    # read-only check of tools + sample libraries
```

`make help` works in every directory that has a Makefile — `music/`, `b/01/`,
`ims/`, `songs/`, and each piece dir. It is authoritative; the tables below are
just a copy. **`make setup` (aka `./setup.sh`) is the correct way to find out
whether a tool or sample library is missing** — do not go hunting through the
tree, and do not conclude something is absent. Current status: `0 missing`.

## 1. Top-level targets (`cd music`)

| target | does | recurses into |
|---|---|---|
| `all` | build all songs (`.abc` → `.ps` → `.pdf`, plus `.fs`/`.csv`) | `musicomp2abc songs ims ims/tc-testing/gershwin t b` |
| `mp4` | YouTube videos from all songs | `musicomp2abc songs t b` |
| `mp4_2` | same, imscomp `_2` variants | `musicomp2abc songs t b` |
| `sfz` | **SFZ VPO mix WAVs → `sfz-mix/`** | `songs ims t b` — **not** `musicomp2abc` |
| `sfz-mp4` | encode SFZ mixes to mp4 | `songs ims t b` |
| `sfz-clean` | remove `sfz-csv/`, `sfz-mix/` | `songs ims t b` |
| `clean` | remove intermediates (`.csv .mid .abc .ps _m.ps .fs .wav`) | all of the `all` dirs |
| `setup` | tool/library check, read-only | — |

**`make all` does not do SFZ.** `sfz` is a separate target. If you want
everything, that is `make all` *and* `make sfz`.

## 2. Per-piece targets (`cd music/b/01`, `music/ims`, `music/songs`, `music/t/e`, …)

Piece Makefiles enumerate their own pieces, named by the `SONGS`/`TESTS`/
`V1`,`V2`… variables — run `make help` in the dir to see the actual list.

- `make all` — build the listed pieces
- `make s1`, `make s2`, … — build one piece only (b/01 uses per-piece targets)
- `make sfz` / `sfz-mp4` / `sfz-clean` — as above, for that dir's pieces
- `make clean` — note `clean: sfz-clean` in `ims/Makefile`, so `make clean`
  there also drops the SFZ dirs
- Force one output: `make -B b9m2_2.fs`
- `ims/`: `make tests` = single-compiler check. Use `music/ims/./DOALL` for the
  real differential suite (see `AGENTS.md`).

## 3. The two compilers and their output suffixes

`musicomp2abc` (older) vs `ims/imscomp` (actively developed). Piece Makefiles
usually build both.

| suffix | belongs to |
|---|---|
| `foo.fs` `foo.abc` `foo.csv` | `musicomp2abc` |
| `foo_2.fs` `foo_2.abc` `foo_2.csv` | `imscomp` |

Formats: `--abc --midi1csv --fluidsynth --vert --hori --staves --lilypond`,
plus `--sfzpipecsv` (imscomp only, deliberately not mirrored).

`.gcs` → `.E` is real CPP: `gcc -E -x c -nostdinc -C -CC -traditional-cpp`
(per-Makefile flags vary slightly). **Always run compilers from the piece's own
directory** — `.gcs` `#include`s are relative.

## 4. SFZ specifics

Chain: `.gcs` → `.E` → `imscomp --sfzpipecsv` → per-instrument CSVs in
`sfz-csv/<piece>/` → `gcs2sfz` → `sfz-mix/<piece>-sfz-mix.wav` → `sfz-mp4`.
Details and env knobs: `SFZ.md`.

To render one instrument while iterating (much faster than a whole piece):

```sh
cd music/b/01
make -B sfz-csv/v1-1/.done          # regenerate the .E + CSVs only
../../music/sfz/gcs2sfz sfz-csv/v1-1/violin.csv \
    --outdir /tmp/vioab --sfzdir ../../music/sfz/library/VPO
```

**`--sfzdir` must point at `library/VPO`, not `library/`.** `gcs2sfz` appends
`VPO_ROOT` (`Virtual-Playing-Orchestra3`) itself, so the wrong value produces
`gcs2sfz: VPO root not found: .../library/Virtual-Playing-Orchestra3` — the
missing `VPO` component in that message is the tell. It is *not* a missing
library. The library is 695 MB and gitignored, so a fresh clone looks empty;
`make setup` reports its real state.

One VPO instrument with 7 desks ≈ 15 s. A whole piece with 12 instruments is a
few minutes. **Top-level `make sfz` is a long job** — run it backgrounded with
output to a log rather than blocking on it.

## 5. Expected failures — do NOT chase these

`make all` at top level **exits non-zero**, by design of the current tree:

- `ims/tc-testing/gershwin`: `new-g3` (`pete.E` Error 1, `new-g3.ps` Error 1)
- These are the 2 known **named ARGHs**. Both compilers fail identically, so
  they are not a mirror problem.
- `abcm2ps` also emits `Length incompatible with BASE` warnings on
  `e.abc` (1812). It recovers and writes the file; not a failure.

Grepping a build log for `error` is misleading: every `gcc -E` line contains
`-Werror`. Filter it out (`grep -i error | grep -v -- -Werror`) before believing
a hit.

DOALL baseline is `0 bare / 2 named` — see `AGENTS.md`.

## 6. Artifacts

`make clean` removes intermediates. `.E`, `.fs`, `.abc`, `.csv`, `.mid`, `.ps`,
`.pdf`, `.wav`, `sfz-csv/`, `sfz-mix/`, `*-sfz.mp4` are all build products.
Some are not gitignored (e.g. `musicomp2abc/LEGATO1*.E`), so check
`git status` after building and delete stray artifacts rather than committing
them.

## 7. Rendering traps

- **Never pipe fluidsynth's stdout into `head`.** The pipe closes, fluidsynth
  dies of SIGPIPE mid-render, and you get a valid-looking 0.049 s WAV. Use
  `> /dev/null`.
- **Never judge an SFZ/ensemble change by diffing the mixed WAV.** The
  per-desk `amix` shifts phase for any pitch change, so block-RMS of `A-B`
  reports ~85% of blocks "differing" for *any* pair, including two that sound
  identical. Prove changes at the CSV/note-event level and use ears for the
  rest.
- To run an old compiler for comparison, copy it **and** `ims/calculate.py`
  into the same scratch dir; `import calculate` resolves against the script's
  own directory.
