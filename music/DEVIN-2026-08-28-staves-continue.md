# Session handoff 2026-08-28 — --auto-staves conversion note fidelity (CONTINUING)

Working dir: /Users/m4/newtmp/saved-m4-stuff/src/github.com/m4/music
Goal from CHECK-STAVES / TODO: does `ims/imscomp --auto-staves --staves` on an
old-format (putd/=) `.gcs` preserve every note (pitch + on/off) when the staff
output is recompiled? Fix what doesn't.

## State of the repo (uncommitted on top of HEAD 0e15e64e)
- `ims/imscomp` modified: (1) do_xpose KeyError fix (done previously, attr);
  (2) **NEW this session**: per-staff initial key emission in print_out_staves.
- `musicomp2abc/musicomp2abc` has ONLY the do_xpose fix mirrored. **NEEDS the
  new key fix mirrored too** (compare with ims/imscomp).
- Untracked: music/CHECK-STAVES, music/DEVIN-2026-08-27-staves.md,
  music/ims/__pycache__/.

## What I confirmed this session (reproduction)
Setup: temp copy of b/02/b2m1.gcs (byte-identical to LEARNING .gcs.backup,
md5 9096d25...) at /var/folders/.../T/opencode/stavetest/. Included the piece's
own instruments.include (b/02/instruments.include) + b/instruments.include as
../instruments.include.

Pipeline (run from piece dir):
1. CPP: `gcc -E -x c -undef -Wundef -Werror -nostdinc -C -CC -traditional-cpp b2m1.gcs -o b2m1.E`
2. ref fs: `python3 ims/imscomp --fluidsynth b2m1.E ref.fs`  -> exit 0, 23162 note events
3. convert: `python3 ims/imscomp --auto-staves --staves b2m1.E > b2m1.staff.txt`
4. conv fs: `python3 ims/imscomp --fluidsynth b2m1.staff.txt conv.fs` -> exit 10 (errors)

Parsing: `.fs` `sleep N` values are in **milliseconds** (fluidsynth native).
Prior session's "huge sleeps" scare was a non-issue; full piece ~655k ms = ~11 min.

## ROOT CAUSE FOUND (the -1 semitone bug AND most legato errors)
The auto-staves output header had ONLY `key c` (key_default). The original voices
use `=key ... d` (D major, 2 sharps) / `=key clarinetA..B f`. With no key in the
staff file, recompiling resolves every written `5c`/`4f` etc. as NATURAL, so
C#/F# positions come out -1 semitone (verified: flute m92 note #4 was 84 in conv
vs 85 in ref). The "legato to same note" errors (71 before, at m92/94/96/267/269
etc.) were mostly SYMPTOMS: a flattened sharp note collided with the following
same-pitch note under legato (fixed by the key).

## Fix applied (ims/imscomp only)
In `print_out_staves`, after emitting `staff {label}: {voices}` lines, when
`auto_staves`, emit per-staff keys mirroring print_header's staff_name logic:
```
if v0 in key_voice and '' in key_voice[v0] and key_voice[v0][''] != key_default:
    print_output(f'key     {label}  {key_voice[v0][""]}\n')
```
(added `global key_voice`). Result header now has `key flute d`, `key oboe d`,
`key clarinet f`, `key bassoon d`, `key violin d`, `key viola d`, `key cello d`,
`key contrabass d` (french_horn/trumpet/timpani c omitted = default).
Verified on minimal repro legato_test_key.gcs: `key flute d` removed the error,
and b2m1 converted file errors dropped 71 -> 26, legato errors mostly gone.

Confirmed good reference: b/02/b2m4.gcs (committed conversion) emits per-staff
keys inline (`flute: treble key d`). wt/v11.gcs is C major so `key c` sufficed.

## Remaining errors in converted b2m1 (26 total after fix)
1. `cresc/dimin False (80,0.375) going down or already at 80` x?? (m4, m12, m23
   ...) - merge emits cresc/dimin where the running volume already exceeds
   target. Reference compiles clean. These are merge artifacts.
2. `tied note #1 (75/63, 4d+/3d+) is not same as '' note?` - m47 & m232, oboe(3)/
   bassoon(7). `t` tie at start of measure into a rest/'nothing' (compare .E).
3. contrabass m90/93/95/265/268/282 legato-to-same. **Investigate related to key:
   reference resolves contrabass `2c8` -> 48 (natural) while converted (staff
   format, `key contrabass d`) resolves `2c8` -> 49 ! Contrabass uses
   `xpose VOICE -12` (macro, .E line ~451) AND `=key contrabassA d`. Need to
   understand why putd-parse produced 48 for 2c under key d (maybe key applied at
   parse yields 49 then xpose -12 => 37?? but ref shows 48, so the putd parse did
   NOT key-sharpen contrabass; the staff-format parse DOES). This inconsistency
   keeps ~6 legato errors alive and likely a handful of real pitch diffs.**
   Lines to study: `=key` handler ~9765, putd `=v` note resolution path,
   `xpose` handling, `pitch[m]` application at print_out_midi1csv_notes ~6381.

## Next steps (still open)
- Investigate contrabass/key inconsistency above; resolve m90-95/265-282 legato
  errors AND make sure no residual pitch diffs (contrabass octave/accidental).
- Fix/neutralize m4/m12 cresc/dimin and m47/m232 tie-into-nothing artifacts.
- Mirror ALL imscomp changes to musicomp2abc/musicomp2abc (do_xpose done; key fix
  outstanding) and keep calculate.py mirrored.
- Delete convert_to_staff.py from LEARNING (dead broken; see 08-27 note).
- Regenerate all 8 old pieces (b/01 v1-1..4, b/02 b2m1+b2m2, b/06 b-6, t/e e).
  Verify each: converted recompile exit 0, per-note fs compare (missing/extra ~0,
  modulo intentional ensemble; silence = ignore; timing in ms not s).
- Run `cd music/ims && ./DOALL` — must stay 0 bare / 0 named ARGH.

## Session scripts (in /var/folders/l8/.../T/opencode/stavetest/, may be wiped)
fsparse.py (parse .fs -> (start,end,pitch,vel,ch) events), compare_fs.py
(ignore channels, ~10 ms timing tolerance), measure_compare.py (per-measure
branch; not reliable due to goto reordering). imscomp.dbg = debug copy of
imscomp with the DBGLEGATO dump (turn on for voice/m selects at line ~6378).

## Comparison reality check
Whole-file note compare of ref.fs vs conv.fs is dominated by goto/repeat
reordering differences (measures 1-365 labeled same but played order/content
differs). Per-measure relative-time compare showed discrepancies in EVERY
measure. This means full note-faithful regeneration may still need goto/repeat
handling in the staff merge time-walk (see 08-27 note issue #3). Don't chase
global-time compare; use measure-scoped or section-scoped diff after fixing keys.

LEARNING dirs (/Users/m4/LEARNING/little/music/...) are backups - DO NOT EDIT.

# Session 2026-08-28 (later) — b/06 round-trip goes fully clean

Supersedes several "still open" items above. All four fixes below are now in BOTH
`ims/imscomp` and `musicomp2abc/musicomp2abc` (byte-identical again; verified).

## Fix 5 — getnote xpose gating (written pitches preserved)
`do a_note`'s xpose-baking branch is now gated with `and not auto_staves`
(imscomp ~16115). Under auto_staves, written pitches stay un-transposed; the
`elif auto_staves and (xv != 0 or xnk != ''): pass` (~16283) leaves them alone.

## Fix 6 — xpose header emission
`print_out_staves` now emits grouped `xpose {voices} {val} [{newkey}]` lines after
the per-staff `key` emission. b-6 header shows `xpose 5,6 -2 c`, `xpose 9,10 -7 c`,
`xpose 21 -12 c`.

## Fix 7 — volumes baseline emission (kills the m9 cresc/dimin errors)
auto_staves header now emits `volumes {label} {t}` per staff (from
`get_volume_level('', v0)` mapped through `vlprint`). Original set
`=volumes fluteA..contrabassA p` (which_vol_now=60) before m1; without it the
recompiled file starts at 100 and `cresc(f,1.0)` (100->90) at m9 errored
"going down or already at 100".

## Fix 8 — rest-collapse rule in the merge time-walk (kills tie errors at
## m148/m150/419/421) — the REAL fix for the "tied note ... not same as '' note?"
The old merge collapsed every all-rest chord group to one rest. That is WRONG when
the rest lengths differ (e.g. clarinet holds `r2` while clarinetB rests an 8th
then plays `4e8l 4fn8 4a8s`): the recompile misreads the merged `r2` as sequential
voice fill, overfills the measure (fill_voice_mlth pads resting voices up to a
bogus max, e.g. mlth 1680 vs 960-tick measure), and the pad rests clear
`last_note_on` before the next measure's tie continuation.
New rule (`group_rests` list, ~imscomp 4905/4937/5017): collapse to one rest ONLY
when every rest in the group has the same length; unequal lengths keep the full
chord `[r2,r8]`. Output b/06 now shows `clarinet: [r2,r8] 4e8l 4fn8 4a8s`.

## Result for b/06
`imscomp --auto-staves --staves b-6.E` -> `b-6.staff.gcs` (7075 lines), then
`imscomp --fluidsynth b-6.staff.gcs` -> exit 0, ZERO errors/warnings.
compare_fs.py: ref/conv note-on totals equal, per-pitch count diff EMPTY
(identical pitch multiset, 16678 note-ons); residual diff is channel assignment
only (some notes moved between channels; pitches and on/off times preserved).
(m <measure> in the fs loop is a STRING — trace comparisons need str(m).)

## Mirroring + regression
- `cp ims/imscomp musicomp2abc/musicomp2abc` (calculate.py already identical).
- DOALL re-run: 0 bare ARGH. Named ARGHs = 22, ALL pre-existing on HEAD (verified
  by stashing the two compilers and re-running b/ and gershwin suites): b2m3/b2m4
  fail "no measure processed yet - must have one before notes" (unrelated
  compiler bug in those hand-authored staff files), new-g3 fails 1 format. No new
  regressions.
- Diagnostics traces (TRACETIE/TOP/FML/CRESC) all removed; py_compile clean.

## 08-28 afternoon: volume-baseline tracking fix (l_vl last-prefix rule)
Regression pattern `cresc/dimin False (X,d) going down or already at X`:
the merged emit only tracked cresc/dimin targets in `l_vl` (merge loop,
`print_out_staves`), so a mid-measure inline `vol()` (b/06 m138 `vol(f)`) was
invisible -> a needed `volumes` reset in the next measure was skipped -> staff
kept a drifted level -> recompile "already at X".  Also `v1-1` m23 (15 errors) and
`b2m1` m4/m12/m23 depended on the entry-vs-mismatch reset.
FIX (`print_out_staves` merge loop, ims/imscomp ~5115): track the LAST
volume-touching prefix in the emitted staff line --
`(?:cresc|dimin)\((\w+),` OR `vol\((\w+)\)`, numeric arg -> `vol_to_name`.
Header entry stays `ba.volume[v0][m][0]` (correct: un-ramped entry level).
Variants tested: KG=known-good(1122), HA=dedup-only, HB=entering-vol-only,
CUR=both, VP=CUR+l_vl-fix, VDBG=debug-trace.  b2m1: KG10/HA6/HB10/CUR0/VP0.
b/06: CUR1 (m143) -> VP0.  v1-1: CUR15 -> VP0.  Result identical to VP after
traces stripped (verified SAME on all pieces).

## 08-28 late-afternoon: DOALL + mirror (validated workflow)
PENDING items final:
- `ims/imscomp` changes are in `print_out_staves` --staves only; DOALL (5 suites
  x vert/hori/csv/fs/abc, no staves) showed **0 bare ARGH** (byte-identical
  imscomp-vs-musicomp2abc output everywhere).  Named ARGHs = 22, all
  pre-existing and symmetric in BOTH compilers (b2m3/b2m4 "no measure processed
  yet" hand-authored tests; new-g3 1 format) -- NOT caused by the change.
- Mirror `cp ims/imscomp musicomp2abc/musicomp2abc` done AFTER validation (per
  user workflow: validate first, update musicomp2abc only if clean).  Byte-
  identical, calculate.py identical.
- LEFT IN musicomp2abc = known-good 11:42 state until now; now updated.
- NOTE (user): --fs diffs where chords route to different voices in the two
  compilers are NOT a clean comparison -- read DOALL bare-ARGH counts with that
  in mind; none occurred here.

## Clean recompile status (final compiler)
b2m1=0, b-6=0, v1-1=0, v1-3=0, v1-4=0.  Remaining errors:
- v1-2 m141 (1 err): pre-existing, PRESENT ALSO in known-good 1122 and in the
  prior port.  Cause: pending cresc(mf,0.125) from m140 completes at voice 4's
  m141 first note, colliding with new cresc(mf,0.75) (multi-voice staff-time
  artifact, not the l_vl logic).
- b2m2 (24), t/e/e.gcs (24): pre-existing old-format migrations.  Different
  flavors: b2m2 `dimin True (60,x) going up or already at 60` -- header resets
  to entry()=60 but true source entry is >60, i.e. ba.volume per-note values lag
  ramp completions (reference entry model unreliable mid-ramp). e.gcs m135
  `cresc False (100,3.0) already at 100` with brass voices ALL resting in m135 --
  ramp-merge lands a spanning cresc where the entry reads 100.  Both need a
  reference-volume model that resolves ramps to completion for the entry() /
  last-prefix() decisions -- open.

## 08-28 evening: b2m2 entry-model analysis (did NOT land a fix)
Probes (imscomp-PROBE/VOLDUMP on b2m2.E, dump before print_out_staves):
- ba.volume[11][*] for violin1A (v0 of the violin staff 11..16) is STATIC [60]
  in EVERY measure -- that voice never ramps.  The real b2m2 violin dynamics
  ride on v13/v16 (violin1C/violin2C): m22 `[-4,80,80]` (sf at first note,
  base=80); m26 `[60,-4,60...]` (sf 2nd note, base 60); m30 `[-4,80,...]`;
  m40 `[60,60,60,60,100]` (cresc to ff LATE in measure -- entry 60); m41-44
  `[-4/100,...]` ff sfs; m47 `[100,100,100,60]` (enters 100, dimin to 60);
  m55 back to 60s.  So per-voice measure-start "true entry" = first
  non-negative value in the array (skipping -4 sf at lead slot).
- So the header rule `ba.volume[v0][m][0]` (v0 only) is wrong whenever v0 is a
  non-ramping voice: at m22 it emits a spurious `volumes violin p` (60) when
  the true staff entry is 80 (mf) -- then `dimin(p,0.25)` errors "already 60".
- Tried two parameterizations in variants (imscomp-VS: max of per-measure
  SCALAR volume[v][m] over the group; imscomp-VSM: max of first-non-negative
  array value over the group): BOTH made it worse (41/42 errors).  Reason:
  at m51 violin enters at 60 (line `[..,cresc(f,0.375)3g16l,..]`, target 90) but
  a sibling sub-group still sits at ff=100 from the m41-47 ff-sf passage, so
  group-max over-inflates the header to 100 -> error.
- Conclusion: correct staff-entry requires the entry of the voice(es) that OWN
  the FIRST NOTE SLOT of the merged measure line (m22: sf voice base 80; m51:
  60), not group-wide max and not v0.  Needs a voice->note-slot map in the
  merge (how each first note is attributed) + per-voice first-slot entry from
  the arrays.  UNRESOLVED -- next session.

## Still open (unchanged from 08-27/08-28 earlier notes)
- contrabass xpose/key inconsistency (m90-95/265-282 legato) — not revisited.
- goto/repeat reordering in the merge time-walk (issue #3) still potential.
- Regenerate remaining old pieces (b/02 b2m2, t/e e.gcs) and v1-2 m141; re-run
  note-faithful compare for each.