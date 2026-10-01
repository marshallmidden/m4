# Legato on the render paths — GM mono-mode investigation
Dev notes @ 2026-09-28. Follow-up to `DEVIN-2026-09-27-legato-slur.md`.

## Why the piano SFZ has no legato

- The piano has **no VPO mapping** (`instruments.py`: `GM_PROGRAM["acoustic
  grand piano"] = 0`; VPO is an orchestral library, no piano), so it renders
  through the **GM/fluidsynth fallback**.
- In `gcs2sfz.write_midi()` the fusion is gated `if pred_expr and ...`
  (`gcs2sfz:469`). The GM fallback deliberately passes `pred_expr=False` (the
  CC11/expression handling differs), so **the legato fusion is skipped for
  every GM-fallback instrument** — piano included.
- So: "piano has no legato" is not a missing sample or a compiler bug; it is
  the `pred_expr` gate in `gcs2sfz`. (A piano maybe shouldn't glide at all;
  see the decision below.)

## Why the SFZ violin glide sounds horrible

- The fusion bends the previous note's voice up to the slurred-in pitch over
  `GCS2SFZ_LEGATO_GLIDE_MS=20` ms (`gcs2sfz:471`) on the **sustain patch**.
- On a violin section rendered with `GCS2SFZ_ENSEMBLE` (7 desks, each detuned
  and panned separately, `DEVIN-2026-09-19-sfz-ensemble.md`), the bend is
  audible as portamento smear on every fused note. Subjective judgement
  (listened on 2026-09-28): bad enough to reject the pitch-bend fusion as the
  default for sustained strings.

## The fluidsynth "#ifdef" question — there is no #ifdef

The "poly mode disables legato" the user remembered is an **`if`**, not a
preprocessor guard:

`src/synth/fluid_synth_monopoly.c:240-248` in
`fluid_synth_get_fromkey_portamento_legato()`:

```c
/* In mono mode legato playing returns the note prior most recent note played */
if(fluid_channel_is_playing_mono(chan) && (chan->mode  & FLUID_CHANNEL_LEGATO_PLAYING))
    default_fromkey = fluid_channel_prev_note(chan);
/* In poly mode legato playing, actually we don't want playing legato.
   So returns INVALID_NOTE */
```

- `fluid_channel_is_playing_mono = (mode & POLY_OFF) || CC68 >= 64`
  (`fluid_chan.h:224`).
- CC68 = `LEGATO_SWITCH` (GM2 legato footswitch). In **poly** mode the
  footswitch routes noteons into the mono-playing machinery, but `fromkey`
  stays `INVALID_NOTE`, so `fluid_synth_noteon_monopoly_legato`
  (`fluid_synth_monopoly.c:670`) finds no fromkey voice and falls through to
  `fluid_preset_noteon()` → **every slurred note re-attacks**.
- This is *why* the current `.fs` design (CC68 down at the slurred onset,
  re-armed ~1-31 ms in, previous note-off pinned one tick before) works the way
  it does: it never asks fluidsynth for legato at all — it just stops the
  decay-kill and lets each note ring as a fresh attack. Measured long ago and
  the numbers are baked into the `imscomp` comment at `imscomp:7055-7076`.

## Does true mono mode give no-attack legato? Yes — for sustained patches. No for the piano soundfont.

Simplest way to put ONE channel group in mono from a `.fs`:

```
resetbasicchannels
setbasicchannels 0 mono_omnioff 0     # 3 = OMNIOFF_MONO  (nbr=0 = this channel only)
```

(Without `resetbasicchannels` first, the default Omni-On-Poly group spanning
all 128 channels rejects the overlap: `setbasicchannels: ... invalid argument`
→ the command is a silent no-op, which produced the "bit-identical to poly"
red herring.)

With the legato footnote held into the next note (i.e. **note-on of the slurred
note comes BEFORE the previous note-off**, which is the reverse of the current
pinned ordering), `NO_ATTACK` legato mode (default, `fluid_chan.c:91`) should
continue the old voice without a new attack.

Measured (minimal 3-pair `.fs`, GeneralUser.sf2, `-r44100`, `-F`):

| instrument | patch | poly mode @65 | mono mode @65 | verdict |
|---|---|---|---|---|
| piano (prog 0) | multi velocity-layer | +6.4 dB hammer, two voices beating | **+6.2 dB hammer still** | mono no-attack does NOT hold |
| strings (prog 48) | sustained | fresh attack ramp, later 61-62 dB double layering | smooth single voice, levels then decays | mono no-attack works |

Piano fails because `fluid_synth_noteon_monopoly_legato` only ignores the
*zones of the fromkey voice* (`zone_range->ignore = TRUE`); the GeneralUser
piano patch spans several velocity-crossfade zones and the re-trigger noteon
finds a non-ignored zone → fresh hammer. Conclusion: **native no-attack legato
is patch-dependent**; it cannot be assumed for a GM soundfont.

## "Will it break midi files?" — yes, for real reasons

- Mono is a **per-basic-channel-group** property with no standard MIDI
  representation. `setbasicchannels` is a fluidsynth shell command, not a MIDI
  event — a `.mid`/csvmidi render cannot carry it.
- Mono forces one note at a time per channel: any two-note chord, trill or
  two-voice line over one voice/channel collapses to the latest note.
  `gcs2sfz`'s GM fallback intentionally merges all voices of a part onto one
  channel (the Chopin-stutter fix, `DEVIN-2026-09-15-cc11-sfz-loudness.md`) —
  mono would break those chords.
- The legato detector changes **note-off** semantics: a note-off while
  legato-playing converts to a note-glide instead of a release, so pedal and
  overlap handling differ from poly.

## Consequence for the option design

The user proposal: a conditional CLI enable/disable, on the assumption mono
legato "works". Measurement says:

1. `setbasicchannels ... mono_omnioff` works for **sustained single-zone
   patches** (violin section) and fails for **multi-zone patches** (piano).
2. It needs a **different note event ordering** than the current pinned
   note-off-before-onset form (`imscomp` emits note-off before the slurred
   note-on; mono legato needs the opposite).
3. It is not representable in `.mid` and changes polyphonic behavior — so it
   must be opt-in, off by default, and scoped.

Recommended shape (single env knob, both paths, consistent semantics):

- `GCS2SFZ_LEGATO` (existing) — `1`/`0` still means fusion on/off for SFZ.
- NEW `GCS2SFZ_LEGATO_STYLE`:
  - `reattack` (default): slurred-in note is a normal fresh attack. No bend,
    no mono. This is what the GM path does today and what SFZ should probably
    do by default given the violin verdict.
  - `glide` (opt-in): the current pitch-bend fusion (existing behavior).
  - `mono` (opt-in, GM/fluidsynth only): emit mono basic-channel + no-attack
    ordering; documented as patch-dependent (works on sustained patches, not on
    the GeneralUser piano) and is NOT emitted into `.mid` (`.mid` keeps the
    re-attack form for file compatibility).
- Everything off by default flips SFZ back to no-fusion — matches both user
  complaints (piano unchanged, violin no longer glides) at the cost of losing
  the measured 2.9% attack elimination.

## Decision -- implemented 2026-09-28

Chose **`reattack` as the default** (`GCS2SFZ_LEGATO_STYLE=reattack`), with the
glide as `GCS2SFZ_LEGATO_STYLE=glide` (opt-in). Rationale: the user's two
complaints (piano never fused; violin glide sounds horrible) are both satisfied
by dropping the fusion default, and the fusion's numbers (2.9-5.2% of re-strikes
eliminated) were never shown to be musical. Implemented in `gcs2sfz`:

- new `_legato_style()` helper: `GCS2SFZ_LEGATO_STYLE` is authoritative
  (`glide`/`reattack`, plus true/false aliases); the legacy `GCS2SFZ_LEGATO`
  knob (=0/1) is honored as an alias for old invocations and overridden by
  STYLE when both are set;
- the `write_midi` fusion gate is now `if pred_expr and _legato_style() ==
  "glide"` (was: `GCS2SFZ_LEGATO` defaulting to on);
- header docstring documents the knob.

Verified on `b/01` v1-1 violin CSV (`INSTR note-on/pitch-bend counts`):

| env | note-ons | bend events |
|---|---|---|
| (default) | 5027 | 1 (ensemble re-zero) |
| `LEGATO_STYLE=reattack` | 5027 | 1 |
| `LEGATO_STYLE=glide` | 4986 | 801 |
| `LEGATO=1` | 4986 | 801 |
| `LEGATO_STYLE=glide` + `LEGATO=0` | 4986 | 801 |

Default == legacy-`LEGATO=0` at the MIDI level, so the default render is
byte-sound; two full violin renders (repeat/glide, 483 s, 7 desks) confirm the
pipe runs clean. Re-rendered artifacts at `/tmp/vioab2/{default,glide}/
v1-1-sfz-mix.wav` ~pp.

**`mono` is NOT implemented.** Native no-attack legato is patch-dependent —
works on the GeneralUser strings, re-attacks on the piano — so as a render
feature it is a trap, and it is not representable in `.mid`. If a future need
is real, the residual design is in §"Consequence for the option design": GM
fallback would have to render CSV events as a `.fs` script (patched
fluidsynth stdin, active `noteon` ordering + `resetbasicchannels`/
`setbasicchannels ... mono_omnioff`), gated behind its own env var, off by
default.

## Files touched
- `music/music/sfz/gcs2sfz` — `_legato_style()` + gate + docstring.
- `music/SFZ.md`, `DEVIN-2026-09-27-legato-slur.md` — default/semantics + A/B
  records updated.
- Experiment artifacts (throwaway): `/tmp/mono/{poly,mono,cc68held,
  strings_*}.{fs,wav}` — the mini `.fs` recipe: header + `select` +
  `resetbasicchannels`/`setbasicchannels` + note events + `quit`; render with
  `~/bin/fluidsynth -q -n -K 256 -r44100 -F x.wav <sf2> < x.fs`.

---

## Follow-up 2026-09-28 (pm): make the SIMPLE test audibly legato

The user's test (`make test-legato.{fs,mp4} test-legato-sfz.mp4`,
`cat test-legato.fs | fluidsynth -a coreaudio -n -q -K 256 -r96000`): **no
legato** for piano/violin/viola on the `.fs`→fluidsynth path and in the GM mp4;
SFZ: nothing for piano, violin shows the slur but "the 4th note cuts off".
Directive: "I am not going to worry about any of the songs, if a simple test
doesn't work."

Root cause: the `.fs` slur emission (CC68 re-arm + previous note-off pinned
ONE tick BEFORE the onset) produced contiguous-but-fresh attacks — the mono
machinery is inert in poly mode, so it never connected anything. No CC68
emission survives if you actually overlap, because a CC68-down note-off after a
later note-on damps the note just started (-56.7 dB), see the `imscomp` comment.

### Compilers: note-off OVERLAP, no CC68 (both, mirrored)
- **Removed** the CC68 ON/OFF pair at a slurred onset (`HERE23/24`) and the
  monotone "pedal re-armed for 2..31ms" comment (the OLD legacy `legato_on`
  CC68 machinery was already dead — `legato_on` is never set True anywhere).
- **Overlap join**: at a slurred-in note, the source's note-off is now emitted
  at `note_time + max(1, min(MIDICLICKSPERQUARTER//16, last_note_length//4))`
  (≈2..31ms of a beat, here 62.5ms on the 500ms eighth) — i.e. the fresh
  attack of the slurred-in note lands INSIDE the still-ringing source tail.
  Same-pitch repeats keep the old `note_time - 1` pin (a late note-off on the
  same key kills the new voice).
- **Post-run snap**: the first note AFTER a slur run is now pinned to the grid
  like the run itself (new `prev_was_slur_member` state; humanize skipped). The
  fresh strike used to drift +~8-60ms late against the grid-pinned run end,
  which is the "4th note cuts off" gap. It now lands at the run's end.

`musicomp2abc` mirrored byte-identically (`diff` clean); DOALL bare-diff stays 0.

### gcs2sfz reattack join (`GCS2SFZ_LEGATO_JOIN_MS`, default 30)
- `write_midi` in `reattack` style extends each `'legato'` source's off into
  its target onset by `GCS2SFZ_LEGATO_JOIN_MS` (same-pitch and >200ms-separated
  pairs skipped). This is the only legato cue the GM fallback's merged one
  channel can carry; on sfizz it is a floor on top of the CSV's native overlap.

### Measured (test-legato, GM .fs→fluidsynth→RMS)
| window | plain measure | slur measure |
|---|---|---|
| per-note attack (150ms) | -39.1..-39.5 dB | -38.0..-39.7 dB (all present, none -56 dB) |
| 45ms barrel BEFORE each onset | -54..-58 dB (silence) | **-37.6..-38.9 dB (~+17 dB tail into the attack)** |

The slur boundary that used to be near-silent now carries the source's ring
into the slurred-in attack — that is the audible legato. Framing/downsides:
`l` on a note = "glide to the NEXT note, no attack at its onset" is preserved;
the fresh strike after the run keeps a ~50ms phrase breath (deliberate).

Artifacts: `/tmp/test-legato-overlap{,2}.wav` (GM A/B), rebuilt
`ims/test-legato.{fs,_m.fs,.mp4}` and `test-legato-sfz.mp4` (all 2026-09-28
12:10-12:11). `~$ make` inside `ims/` regenerates everything.

---

## Follow-up 2026-09-28 (late pm): soft slur-in + seam pedal + slow attack

User verdict on the overlap-only version: **piano no legato on either path**;
strings *slight* on GM, "there but still too much of an attack" on SFZ. So a
fresh full-velocity re-strike is not legato, and strings need their onset
tamed. Three mechanisms added (this section), all mirroring the `l` semantics
(source slurs into the NEXT note).

### 1. Compilers (both, mirrored): slur-in SOFT velocity + CC64 seam pedal
- New var `slur_velocity` (default **45.0**, `%` of normal velocity), right below
  `pedal_overlap` (default **30**) in each compiler. Any note whose PREDECESSOR carries `l` —
  including chained ones that carry `l` themselves — is emitted at
  `round(note_velocity * slur_velocity / 100)` (floor `_velocity_min`), so
  only the run's FIRST note is struck at full force. It reads `prev_note_slurred`,
  not the note's own suffix.
- Overlap narrowed to a HALF beat-clip: `_ovp = MIDICLICKSPERQUARTER//32`
  (15 ticks / 31.25ms here, was `//16`/62.5ms) so source+target do not sound
  together long enough to read as a two-note blur ("slightly loud when the two
  notes are playing"); the seam connection is carried by the pedal instead.
- **CC64 legato seam pedal**: at every slurred-in onset (different pitch),
  press CC64 `note_time - pedal_overlap_ticks`, release at
  `note_time + pedal_overlap_ticks + _ovp` (30+15 = 45 ticks = 93.75ms here),
  so the source's tail rings a LITTLE way UNDER the soft slurred-in note
  instead of damping at its key-lift, and the damper lifts soon after. Only on
  melodic channels (`l_instrument in instruments` — drums excluded) and
  anchored to `note_time` so the pedal cannot outlast the run. Tags
  `HERE_PED_SLUR_ON/OFF`.
- This is what a percussive patch (piano) needs to read as connected: the
  resonance carries the seam; the soft strike is a color, not a re-attack.
- Musicomp2abc mirrored byte-identically; verified `diff test-legato.fs
  test-legato_m.fs` → IDENTICAL; b/01 v1-1..v1-4 + test-legato × 5 formats
  (vert/hori/csv/fs/abc) all identical.

### 2. gcs2sfz: GM-fallback seam pedal + VPO slow-attack split
- `write_midi`: new env `GCS2SFZ_LEGATO_PEDAL_MS` (default 60) — on the GM
  fallback (`pred_expr=False`) presses CC64 `pedal_ticks` before each slur-in
  onset and releases it `pedal_ticks` after (into `cc_timeline`). Skipped on
  the sfizz path because sfizz honours CC64 as a damper. The compilers' `.fs`
  seam pedal and this are the same idea; the CSV velocities are already soft
  because `slur_velocity` flows into `--sfzpipecsv`, so the GM piano reaches
  the seam with BOTH cues.
- New env `GCS2SFZ_LEGATO_ATTACK_MS` (default 150): in reattack style, the
  slurred-in sustain notes of a desk/part re-render on a slow-onset copy of
  the sustain patch (`_edited_patch` ampeg_attack override, exactly the
  `GCS2SFZ_ATTACK` machinery reversed) and `amix` with the fast-attack rest.
  A slurred-in VPO note now SWELLS under the source's overlapping tail instead
  of striking on `GCS2SFZ_ATTACK=0.03`. Both the articulation `short+longs`
  branch and the no-articulation fallback get the split (helper
  `_slur_target_marks(notes)` — nearest preceding same-staff 'legato' source;
  `_slur_slow_attack()`).
- New env `GCS2SFZ_LEGATO_ATTACK_GAIN_DB` (default **3.0**): post-render
  ffmpeg `volume` on the slow-attack slur stem before amix. Perceived level =
  attack + intensity + volume, so the tamed onset also lowered the note body;
  +3 dB gives the sustained body back without re-striking the onset. Applied
  only to the slur-in stem (via a `gain_db` param threaded through
  `_render_sfz`), never to the whole render.
- `GCS2SFZ_LEGATO_VEL_FACTOR` (default **1.0** = off): extra hardening of
  slur-in midi velocity on top of the CSV's. Left as a knob; double-softening
  (0.55×0.55) made the GM piano slur-ins SILENT (-54.7/-60.9 dB windows, vs
  -38 for a plain strike), so the default leaves velocity to the compilers.

### Measured (test-legato)
GM piano (GeneralUser), onsets: plain -37.5..-38.6 dB; slur-ins -49.3/-50.6 dB
(≈11-13 dB below a plain strike — attack slightly softer than the 55% round),
body after the pedal lift ≈ -52.8 dB (the "holding too quiet" side of the
velocity tradeoff; the +3 dB stem boost applies to VPO, not this GM fallback).

SFZ violin (full mix): slur-in onsets stay at plain-note level (-33.5..-40.5 dB
over the 0.25 s window with the +3 dB swell gain) as swelling bodies rather
than 0.03 s-attack strikes.

Rebuilt `ims/test-legato.{fs,_m.fs,.mp4}` and `test-legato-sfz.mp4` (14:37);
overlap 31.25ms, slur-in vel 34-35 with `cc <ch> 64 127/0` seams lasting 93.75ms
at each boundary, fresh NT3 vel 83 with no pedal bleed across the run break;
`diff test-legato.fs test-legato_m.fs` stays IDENTICAL.