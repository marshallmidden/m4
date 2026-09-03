# RESUME: v1-2 m141 staves recompile fix — SESSION STATE (2026-09-02, save for next day)

## Current task & status
- Fix the last remaining staves-conversion recompile error on piece v1-2:
  `m:141 v4: cresc/dimin False (80,0.75) going down or already at 80` on the clarinet staff.
  Goal: all 8 old-format pieces recompile clean via `--auto-staves --staves` → `--fluidsynth`.
- **DOUBLE-CHECK BEFORE TRUSTING MY SUMMARY**: I was mid-investigation of a stale-committed-file question
  (see "OPEN QUESTION / NEXT STEPS"). Do NOT commit until that is resolved and the mirror is done.

## What is DONE and VERIFIED
- Fix applied to `music/ims/imscomp` (uncommitted, `M` in git): replaced the `last_pfx` (last-prefix-target)
  logic in the `l_vl` update (~old lines 5353–5365) with a **peak** computation = HIGHEST vol/cresc/dimin
  target in the staff line (numeric via `v_name_print`/int), set via `vol_to_name(peak_vl)`. Added a comment.
- Regenerated v1-2 staff with fixed compiler → **recompiles clean (0 errors, exit 0)**. The m141 reset
  `volumes clarinet p` is emitted at line 2589 (right after `measure 141` = line 2587).
- **ALL 8 PIECES now recompile clean (0 errors) and are note-faithful (0/0 pitch diffs)** via my matrix:
  - v1-1, v1-2, v1-3, v1-4 (from b/01), b2m1, b2m2 (from b/02), b-6 (b/06), e (t/e).
  - Per-pitch multiset ref.fs (original .E) vs conv.fs (converted staff) = MATCH (0 diff) for all 8.
    noteon counts: v1-1=17278, v1-2=6964, v1-3=8927, v1-4=10653, b2m1=23391, b2m2=8264, b-6=16678, e=39433.
- `musicomp2abc/musicomp2abc` NOT mirrored yet (still byte-identical to pre-fix imscomp).

## Validation artifacts (all in /tmp/opencode/matrix, gitignored-temp)
Each piece has `<p>.E`, `<p>.staff.gcs`, `<p>.conv.fs`, `<p>.ref.fs`, `<p>.conv.err` (clean) in /tmp/opencode/matrix/<p>/.
v1-2-specific: /tmp/opencode/v12/ (v1-2.E, v1-2.new.gcs, convnew.fs, ref.fs, dbg files, diff.txt).

## OPEN QUESTION / NEXT STEPS (do these first tomorrow)
1. **Establish true baseline for the diff question** — the committed `b/01/v1-2.staff.gcs` is STALE (it was the
   known-good-but-still-erroring version from a prior session). A diff of new-vs-committed showed 140 removed
   `volumes` lines + 13 added `volumes` lines + instrument blocks moved. This is NOT necessarily caused by my
   fix — the committed file may be multiple revisions behind the current compiler (EV2/EV4 first_note_entry work).
   **To isolate my change**: generate v1-2 staff with the PRE-fix imscomp (stash/checkout the 9 lines) and diff
   against post-fix. Confirm the ONLY delta is the expected extra `volumes` resets (peak-based), not wholesale
   `volumes` line moves. If pre-vs-post diff is tiny, the 140-line change is just stale committed file, and the
   new staff outputs are fine.
2. **Mirror the fix** into `music/musicomp2abc/musicomp2abc` (`cp ims/imscomp musicomp2abc/musicomp2abc`), then
   verify both are byte-identical again (`cmp`). Also mirror `ims/calculate.py` ↔ `musicomp2abc/calculate.py` if
   touched (it was not).
3. Re-verify all 8 pieces once more from the mirrored compiler (the matrix uses ims/imscomp).
4. Optionally run a DOALL subset to confirm no regression (DOALL tests vert/hori/csv/fs/abc, NOT --staves, so
   print_out_staves changes shouldn't affect it — but run `cd music/b && ./AAA.diff._2` or `music/ims && .DOALL`
   with a LONG timeout in background; it timed out before at 300000 ms).
5. `git diff music/ims/imscomp` to confirm only the intended change, then commit (likely a new commit for this fix).
   Last commits are from 2026-09-02 (fluidsynth echo + 256-voice stress); user likely wants a commit for this fix.

## Commands that work / gotchas
- Reproduce v1-2 error: `cd /tmp/opencode/v12 && python3 <music>/ims/imscomp --auto-staves --staves v1-2.E > v1-2.staff.gcs && python3 <music>/ims/imscomp --fluidsynth v1-2.staff.gcs` (was exit 1 with m141 error, now exit 0).
- The matrix script is at /tmp/opencode/matrix.sh — it STAGES pieces into /tmp/opencode/matrix/ (b-pieces use
  ../instruments.include symlinked to music/b/instruments.include; b2m2 also copies music-2-2.gcs; e copies
  t/e/instruments.include locally) then per-piece: CPP → --auto-staves --staves → --fluidsynth (conv) + --fluidsynth (ref).
- **bash gotcha**: running CPP inside a nested `bash -c` with `$CPP` variable quoting silently fails (gcc's
  -Werror/-Wundef exits 1 while still writing the .E). Run the matrix as `bash -lc 'bash /tmp/opencode/matrix.sh'`
  (the `-lc` fixes it — plain `bash script.sh` failed the CPP step with "CPP FAILED").
- DOALL: `cd music/ims && ./DOALL` — run in background with timeout > 300000 ms (timed out at 300s before).

## Key code facts (for context)
- `l_vl[label]` is used ONLY as a `!=` comparison gate (line ~4539 set/compare; 5001, 5365 set) vs next measure's
  true entry volume — never as a value. Over-estimating it (peak) only emits extra, always-correct `volumes`
  resets, never wrong ones. This is WHY the peak-based fix is safe.
- Root cause of m141 error: reparse smears vol/cresc/dimin prefixes across ALL staff voices (one volume state per
  staff design). v5 ramps 60→80→60 while v4 (resting, shorter event count) catches `cresc(mf,0.125)` but not
  `dimin(p,0.125)` return, stranding v4 at 80 → m141 `cresc(mf=80,0.75)` on v4 → "already at 80".
- Fix validated empirically earlier: injecting `volumes clarinet p` at measure 141 resets v4→60 and clears error.

## Files
- music/ims/imscomp — FIX APPLIED (uncommitted). MUST be mirrored to musicomp2abc.
- music/musicomp2abc/musicomp2abc — identical copy, NOT yet updated.
- music/b/01/v1-2.staff.gcs (committed) — old known-good-with-error version; new good version exists in /tmp/opencode/matrix/v1-2/.
- music/DEVIN-2026-08-28-staves-continue.md, DEVIN-2026-08-27-staves.md — prior staves session notes.
- /tmp/opencode/v12/diff.txt — diff of committed vs new v1-2 staff (for context on the open question).
