# Converting old putd/= `.gcs` pieces to new staff `.gcs` — Findings & Fixes (2026-08-27)

Goal: verify `--staves` / `--auto-staves` conversion preserves every note
(pitch + on/off timing), then fix issues found. User confirmed: backups live in
`/Users/m4/LEARNING/little/music/` and MUST NOT be modified; the git repo is
`/Users/m4/newtmp/saved-m4-stuff/src/github.com/m4/music`.

## Repo / paths
- Git repo root = the `music/` content (`b/`, `t/`, `ims/`, `musicomp2abc/`, ...).
  `music/` prefix in git paths maps to repo root.
- Conversion output and `.gcs.backup` originals live in
  `~/LEARNING/little/music/{b/01,b/02,b/06,t/e}/`. Do NOT edit those.
- Working temp copies: `/var/folders/l8/.../T/opencode/stavetest/music` (copy of
  LEARNING) and `.../gitrepo` (copy of git repo). Helpers: `fsparse.py`,
  `compare_fs.py` in the same temp dir.

## Which 10 files and their format (checked `.gcs.backup`)
- OLD format (putd + `=v`), NEED conversion (8): b/01/v1-1..v1-4, b/02/b2m1,
  b/02/b2m2 (putd only, no =v — special), b/06/b-6, t/e/e.gcs
- ALREADY staff/new format (no conversion): b/02/b2m3, b/02/b2m4
  (the committed b2m3/b2m4 are good reference conversions)

## Old-format structure (putd/=/=v)
- `putd =fluteA=1=` maps a named voice to a number.
- `=flute  fluteA..fluteB` declares the instrument and voice range.
- Note content: `measure N` stanzas, then one `=v voiceName: content` line per
  voice per measure. e.g. b2m1: 365 measures x 26 voices = 9486 `=v` lines.
- `=v` content is comma-separated note columns: `5d32, r8,r4`, `vol(p)4f8l,...`,
  with per-voice suffix A/B/C/D for divisi (violin1A, violin1B, ...).

## Target staff format (see committed b/02/b2m4.gcs)
```
measure N
page ...
bars ...
voice N
staff flute: 1,2
...
# one MERGED note line per staff (all divisi voices folded into chords):
flute: [5d32,5d32]
violin: [3d32,3d32,r32,...]
```

## THE REAL FIX is in imscomp, NOT convert_to_staff.py
- `ims/convert_to_staff.py` (in LEARNING dir) generates a per-voice suffix format
  (`fluteA:`, `fluteB:`) that **imscomp rejects** (does not compile). Its
  `chr(ord('A') + count - 1)` at line 249 overflows A..Z into `[ \ ] ^ _ ` a..z
  `{ | } ~`, AND it counts `=v` *occurrences* (9486) not distinct voices (26), so
  suffixes run away. This script is not a usable path — DELETE it.
- The working generator is imscomp's own `--auto-staves`. Steps:
  1. Preprocess with CPP first (real Makefile flags): `gcc -E -x c -undef -Werror
     -nostdinc -C -CC -traditional-cpp in.gcs -o in.E` (run from piece dir).
  2. `python3 ims/imscomp --auto-staves --staves in.E > out.staff.txt`
     (output goes to STDOUT for --staves; positional arg 2 is ignored).
  3. This emits proper grouped `measure`+`staff`+merged-line format.
- Running `--auto-staves` on the RAW `.gcs` (not `.E`) fails with "no measure
  processed yet" on `=flute`/`#include` lines. ALWAYS run it on the `.E`.

## Fix applied to ims/imscomp (mirror to musicomp2abc/musicomp2abc!)
- `do_xpose` (~line 14675) crashed: `key_sig_int[key_default]` with
  `key_default='C'` (string) -> `KeyError: 'C'`. `key_sig_int` maps int->keyname.
- Fix: if key_default is not an int, convert via `key_sig.get(str(kd).lower(),0)`
  before indexing `key_sig_int`. This unlocks `--auto-staves` on old files.

## Remaining imscomp merge bugs (note fidelity when recompiling staff->fs)
Even after the above fix, the staff output recompiled with
`imscomp --fluidsynth` gives:
1. A batch of notes come out **pitch -1 semitone** vs reference on exactly the
   C#/F# (D-major) positions: 49<->48, 54<->53, 61<->60, 66<->65, 73<->72,
   78<->77, 85<->84, 90<->89 (identical counts each way). Root: NOT the key
   signature (injecting `key <label> d` did not change it).
2. **Legato `-l` suffixes** cause "legato to same note ... (use a tie)" errors
   -> converted staff file recompiles with exit 10 (warnings: 13, errors: 10),
   though `.fs` is still produced. Reference compiles exit 0.
   (Merge code at imscomp ~4954 strips `l` only from final chord of a measure —
   needs broader handling.)
3. **Larger structural issue (the real blocker):** the merged output is
   substantially mangled for full-orchestra old pieces. Side-by-side of measure 3
   shows the converted content does NOT match the original: duplicated
   chorus/repeat sections, spurious `vol(60)`/`vol(90)`/`vol(100)` artifacts,
   octave-0 notes like `0a8`,`0b8`, and an `r1`/`r2d` explosion. Cause: the old
   files use `goto`/da-capo sectional repeats; the merge emits "goto in measure
   ... not handled" warnings and walks the wrong measures. Getting a note-faithful
   regeneration of these 19th-century full-orchestra old files requires fixing
   goto/repeat handling in the staff merge (print_out_staves time-walk), in
   addition to 1 & 2. This is a substantial, ongoing compiler task — NOT finished.

## Why not just use convert_to_staff.py
It was deleted. It emits per-voice `fluteA:` lines + `chr(ord('A')+count-1)`
suffixes that overflow A..Z into `[ \ ] ^ _ \` a..z `{ | } ~`, counts `=v`
occurrences not distinct voices, and its per-voice output format is rejected by
imscomp anyway. Not usable.

## Verification method
- Reference `.fs`: cpp `.gcs.backup` -> `.E`, then `imscomp --fluidsynth .E .fs`.
- Converted `.fs`: `imscomp --fluidsynth out.staff.txt out.fs`.
- Compare with `compare_fs.py ref.fs conv.fs` (parses note on/off, reports
  missing/extra, time delta). Post-fix goal: 0 missing, extras only from
  intentional `ensemble`, time_delta 0.
- b2m1 current state (before transposition/legato fixes): orig 23391 paired,
  conv 23397; 2583 notes differ only by -1 pitch (8 unique slots); ~6 extra at
  odd slots; legato compile exit 10.

## To-dos / state
- [x] classify 10 files (8 old, 2 new)
- [x] diagnose: real generator = imscomp --auto-staves on .E
- [x] fix imscomp do_xpose KeyError
- [ ] mirror do_xpose fix to musicomp2abc/musicomp2abc
- [ ] fix -1 semitone transposition offset in merge
- [ ] fix legato -l suffix recompile errors
- [ ] delete convert_to_staff.py
- [ ] regenerate all 8 old pieces, verify build + note preservation
