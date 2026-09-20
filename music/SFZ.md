===============================================================================
SFZ render pipeline: Virtual Playing Orchestra via sfizz
===============================================================================

Second, higher-fidelity render path alongside the GM/fluidsynth path
(`make fs` / `make mp4`). Render chain:

  piece.gcs --CPP--> piece.E
      --imscomp --sfzpipecsv--> sfz-csv/<piece>/<instrument>.csv
      --gcs2sfz--> sfz-mix/<piece>-sfz-mix.wav
      --gcs2youtube -a--> <piece>-sfz.mp4

    imscomp:  `imscomp --sfzpipecsv piece.E`   (per-instrument CSVs)
    gcs2sfz:  `gcs2sfz sfz-csv/<piece>/ --sfzdir <VPO lib> --outdir sfz-mix/`
    video:    `gcs2youtube -a sfz-mix/<piece>-sfz-mix.wav -o <piece>-sfz.mp4 \
                  <piece>_2.fs`   (same overlay as the GM mp4, audio swapped)

VPO samples render via ~/bin/sfizz_render; instruments without a VPO mapping
fall back to GM through the patched ~/bin/fluidsynth + GeneralUser.sf2.

===============================================================================
CSV schema
===============================================================================

10 columns:

  start,dur,midi_note,velocity,pan,reverb,vol_start,vol_end,pan_end,articulation

  - start/dur: timeline seconds (float). dur covers [note-on, note-off).
  - pan = MIDI CC10, reverb = MIDI CC91, taken verbatim from the score's
    macros. Hard-coded orchestral seating in b/instruments.include:
    violin 36 / 2nd 42 / viola 64 / cello 85 / bass 92, woodwinds ~55-60,
    brass ~62-65, percussion ~35-50 give an L/R stage layout plus a moderate
    hall send; adjustable per macro. sfizz honors CC10 as pan by default
    (sfizz #475), verified L < center < R; fluidsynth/GM too.
  - vol_start/vol_end: expression (CC11) value AT the note, snapped at
    note-on/note-off; pan_end = CC10 at note-off. A note's vol_start==vol_end
    means flat expression (no per-note MIDI emitted by gcs2sfz); otherwise
    write_midi ramps CC11 vol_start->vol_end and CC10 pan->pan_end across the
    note [on, off), audio-verified (VPO violin: CC11 40->120 swells RMS ~1.5x,
    pan 30->100 sweeps audibly L->R; both CC11 via amplitude_oncc11 and CC10
    respond mid-note in sfizz, fluidsynth/GM likewise).
  - articulation (10th column, added 2026-09-19): per-note token derived from
    the note's score suffixes, emitted only under --sfzpipecsv (midi1csv/
    fluidsynth/abc outputs stay byte-identical). Tokens: `s`->staccato, `m`->
    marcato, `u`->tenuto, `a`/`A`->accent, `l`/`z`->legato, `t`->tied, else
    sustain. gcs2sfz reads it (optional; older 9-col CSVs keep the duration
    split) and renders staccato/marcato notes with the staccato patch, all
    others with sustain. Implementation: imscomp sfz_articulation_token() /
    sfz_note_on_line() at the three Note_on_c sites; gcs2sfz read_csv_notes()
    returns 11-tuples. Full detail: DEVIN-2026-09-11-sfz-articulation.md.
  - staff (11th column, added 2026-09-19): the 0-based voice number a note
    came from. An instrument spread across several staffs is a section of
    DESKS; the render backend gives each desk its own small detune and pans
    them across the stage (see the Stage/ensemble simulation section).
    Single-staff instruments render tuned the same as ever.

Voice/measure selection is shared with the midi path -- no sfz-specific
wiring: `--voices 14` / `--measures 3` slice the per-instrument CSVs via the
same filter loop (verified b/01 v1-1 + synthetic). Voice args are NUMBERS
only (`--voices 14`); names are not accepted (0 files).

===============================================================================
gcs2sfz render behaviors
===============================================================================

  - sfizz render: one sfizz_render per instrument -> per-instrument WAV
    (`sfz-mix/`), mixed via ffmpeg (amix + loudnorm-implicit chain).
  - Reverb: VPO has no reverb effect in this sfizz build and ~/bin/ffmpeg's
    afir mutes the dry signal, so the room is an offline aecho early-
    reflection tail baked into each instrument WAV, scaled by reverb/127
    (dry at unity; only echoes scale). The GM fallback gets real reverb from
    fluidsynth's CC91 handling.
  - GM fallback: instruments with NO VPO mapping (acoustic grand piano, bell
    tower, church bell, unknown instruments) render through fluidsynth +
    GeneralUser.sf2 with an inserted GM program change (0xC0). sf2
    auto-detected: /Users/m4/src/GeneralUser/GeneralUser.sf2 (darwin) or
    /home/m4/src/GeneralUser_GS/GeneralUser.sf2 (linux); --sf2 overrides.
  - Articulation recipe: VPO sustain patches attack 0.2-0.6s and release
    0.6-2.25s, which smears fast (16th/32nd) runs. Default recipe splits each
    part by note length (GCS2SFZ_ARTIC_MAX=0.5): short notes render staccato
    blended under a fast-attack sustain (GCS2SFZ_ARTIC_BLEND=1,
    GCS2SFZ_ATTACK=0.03), long notes use the fast-attack sustain alone.
    ARTIC_MAX=0 disables the split, BLEND=0 disables blending, ATTACK=0 the
    attack override. With the articulation column present the split is keyed
    per note (staccato/marcato -> staccato patch) instead of duration-only.
  - Expression predistortion: sfizz applies CC11 LINEARLY in amplitude while
    GM is exponential, so write_midi predistorts emitted CC11 as
    127*(v/127)^GCS2SFZ_EXPR_EXP (default 4.0) on the sfizz path, matching
    the fp->p contrast of GM (measured b/01: SFZ E=4.0 = 12.2 dB vs GM 12.5
    dB). Velocity/CC10/CC91 untouched.
  - GM-fallback expression folds into velocity: the sfz CSV merges every
    voice of one instrument onto a SINGLE MIDI channel, so emitting CC11
    before each note-on pumped ALL sustained notes on the channel (the ~26s
    Chopin "stutter"). The GM path (pred_expr=False) skips per-note CC11 and
    instead writes the note's velocity as vel*(vol/127)^GCS2SFZ_GM_EXPR
    (default 1.3); only CC10 sweeps, CC91, and in-note ramps still emit CC.
    Run fluidsynth with `-g GCS2SFZ_GM_GAIN` (default 0.5, matching the .fs's
    `synth.gain 0.5`; stock 0.2 left fallback stems ~8 dB quiet).
  - Per-instrument stem gain: GCS2SFZ_GAIN="name:db,name:db" applies an
    ffmpeg volume ramp before mixdown (quiet libraries, e.g. Sonatina
    strings). Per-level loudness normalisation to the piano lives in
    instruments.LEVEL_GAIN (flat stem gain + CC11 residual in write_midi).
  - One-shots (1812 canon + church bells): placements play the FULL sample
    (no atrim -- tails ring out; a boom IS the sound) scaled by velocity,
    with a part-level limiter (0.95 FS). Exception: `church bells` (in
    ONESHOT_TRIM_TO_DUR) are trimmed to their scored duration, or the 31.6s
    peal would smear over the finale and ring 30s past the canon (fixed
    2026-09-18). Mix length derives from actual rendered stems (ffprobe), so
    natural tails are never trimmed off.
  - Out-of-range warning: gcs2sfz parses each VPO patch's lokey..hikey and
    warns when a CSV writes notes outside it (skipping kit drums and
    one-shots). On test-volume-levels: piccolo 6/6, timpani 5/6, tuba 4/6,
    contrabass 1/6 silent -- surfaced, not silent. GM fallback for
    out-of-range notes not implemented (deferred).
  - determinism: write_midi uses imscomp's deterministic timeline (per-note
    humanize seeded random.seed(42)) where the CSV carries per-note values;
    sfizz render itself is deterministic per input.

===============================================================================
Stage / ensemble simulation (tuning, detune, pan) -- implemented 2026-09-19
===============================================================================

Facts the design rests on:

  - All voices are tuned IDENTICALLY (same semitones). The only per-note
    randomization is imscomp's humanize: timing jitter of +/-5 ticks
    (~ +/-2.6ms at 120 bpm, random.seed(42) for determinism) plus a small
    velocity/volume wobble. No pitch detune or per-voice pitch offset existed
    before this feature.
  - Pan is per-part from the score (CC10, the seating above). All voices of
    one instrument MERGE onto a single MIDI channel in the sfz CSV
    (by_instrument roll-up), so before this feature there was no
    intra-section spread: a violin section of several desks sounded like one
    voice in one seat (all its desks carry pan 36, for example).

Pitch-bend mechanics (why it works this way):

  - Pitch bend is CHANNEL-wide: it shifts every currently-sounding and future
    voice on that channel by a fixed interval (cents), not per-note.
  - The bend range is set via RPN 0 (registered parameter, "pitch bend
    range"). fluidsynth's default is +/-2 semitones (200 cents); raw bend
    values span 0..16383 with 8192 = center. So at the default range,
    cents_from_bend = (bend - 8192) * 200 / 8192  (~41 cents per 4096 steps;
    +8 cents ~= bend 8519). sfizz honors CC pitch bend the same way.
  - Bend is PERSISTENT per channel: write_midi re-zeros it (back to 8192) at
    the end of every detuned stream, so no stale bend bleeds into a stream
    that reuses the channel.

Implemented behavior:

  - Non-ensemble instruments (a single staff): tuned the same as before -- NO
    detune, score pan verbatim. Output stays byte-identical to pre-feature.
  - Ensembles (an instrument spanning multiple staffs) are rendered as a
    section of DESKS, tuned the way real desks are:
      * Each desk is internally in tune -- all its notes sit on one channel
        and the per-stream pitch-bend shifts the WHOLE desk together, exactly
        like a real player whose strings are tuned against each other.
      * Each desk is tuned slightly DIFFERENTLY from its neighbors -- the
        desks spread deterministically over -4..+4 cents (GCS2SFZ_ENSEMBLE_
        DETUNE), no two desks of a section ever on the same tuning (two desks
        on the same tuning would just double the volume, the "why have two
        violins" trap). One desk carries 0 and is the section's tuning
        reference.
      * Each desk is panned slightly differently around the score's base pan
        (the desks can't stand on top of each other); they fan across +/-10
        pan units (GCS2SFZ_ENSEMBLE_PAN).
  - Implementation:
      * imscomp --sfzpipecsv appends the 11th column: staff id (= voice_on,
        the 0-based voice number; ''/absent -> single-staff or legacy).
        Gated on --sfzpipecsv like the articulation column; all other
        outputs stay byte-identical (DOALL 0 bare).
      * gcs2sfz _ensemble_plan() groups a multi-staff instrument's notes by
        staff and render_instrument() renders each staff as its OWN stream
        (own MIDI file/channel, own sfizz_render or fluidsynth run), then
        amixes the staff WAVs into the instrument stem. Because each stream is
        its own channel, the channel-wide pitch bend applies per stream.
      * Detune: write_midi emits a tick-0 pitch-bend (raw = 8192 + cents *
        8192/200, clamped) and a trailing re-zero. Each desk gets its own
        value: the deterministic permutation of -R..+R (R = GCS2SFZ_ENSEMBLE_
        DETUNE, default 4) keyed on crc32(name|value), re-zeroed per stream.
        GM-fallback streams get the same bend. Observed on b/01 v1-1: violin
        (7 desks) [+4,+0,-3,-2,+1,+3,-4] cent; viola 3 desks [-1,+3,+2];
        winds 2 desks each; pizzicato_strings (9 desks) [-2,+3,+2,-3,-1,+4,
        +0,+1,-4].
      * Pan: each staff stream adds a deterministic per-staff offset to the
        score's per-note pans (and pan_end): the staffs fan evenly across
        +/-GCS2SFZ_ENSEMBLE_PAN units (clamped 0..127) with a small per-staff
        jitter, e.g. v1-1's 7 violin desks fan [-9,-8,-3,+1,+2,+8,+11] around
        base pan 36. Single-staff instruments use the score pan verbatim
        (offset 0).
  - Env knobs:
      * GCS2SFZ_ENSEMBLE=0 disables the whole feature (pre-feature single-
        stream behavior). Default ON.
      * GCS2SFZ_ENSEMBLE_DETUNE (default 4): +/- cents per desk.
      * GCS2SFZ_ENSEMBLE_PAN (default 10): +/- pan units per desk.
  - Backwards compat: legacy 9-col CSVs, and 10-col with empty 11th, parse
    and render exactly as before (no staff => no split).

===============================================================================
Make targets
===============================================================================

Every piece dir (songs/, ims/, b/01..04, b/06, b/09, b/sonata14, t/e), plus
the top-level music/ recursion (into `songs ims t b`):

  make sfz         - render all pieces in this dir to sfz-mix/*-sfz-mix.wav
  make sfz-mp4     - encode sfz-mix/*.wav -> *-sfz.mp4
  make sfz-clean   - rm -rf sfz-csv/ sfz-mix/ *-sfz.mp4

ims/ adds: make sfz-tests / sfz-all / sfz-mp4-tests / sfz-mp4-all.

Every Makefile sets `.DEFAULT_GOAL := help`, so a bare `make` prints help
instead of starting a VPO render (sfz targets precede `help` in the files).
The %-sfz-mix.wav rules list `$(CURDIR)/$(GCS2SFZ)` as a prereq so edits to
gcs2sfz trigger re-render.

===============================================================================
Known limitations
===============================================================================

  - A0/A1 are intentional error-injection tests; they fail `make tests` too
    (pre-existing). DECODE/ENCODE/L1/P2/STAFF/vc1-test also fail the plain
    `%.fs` compile -- none are SFZ regressions.
  - The %-sfz.mp4 ffmpeg recipes need the source-built ~/bin/ffmpeg
    (ASS/drawtext). brew ffmpeg lacks them.
  - `print_out_sfzpipecsv` note pairing (fixed 2026-09-10): a note whose
    note-off lands on a different channel than its note-on (pizz->arco
    switch) was dropped from the exact key (voiceon, chan, pitch); the
    release now falls back to the oldest pending (voiceon, pitch) across
    channels and attributes it to its START channel. Verified: full b/01 v1-1
    17269 sfz rows == midi note-ons (was 8 short).

SFZ pieces excluded by design: `SONGS_NO_PS` (inv1 vinci sonata
macros-complicated) in songs/; their GM renders are unchanged.

===============================================================================
Status / next items (2026-09)
===============================================================================

Resolved:
  - b/01 v1-1 first-measure pizz 2nd note too loud -- RESOLVED 2026-09-19.
    Root cause: inverted velocity-layer balance in the merged VPO cello
    section patches (low-vel layer volume ~23-30, high-vel ~10-18; soft pluck
    at the 54-62 boundary rendered louder than a strong 63+ one). gcs2sfz
    _cello_velocity_fix rewrites low-velocity cello regions' volume to match
    their high-velocity partner at render time (monotonic velocity->loudness,
    amp_veltrack supplies the range); no library edit.
  - GM-fallback loudness ~10 dB below .fs -- RESOLVED 2026-09-16 (GCS2SFZ_GM_
    GAIN + GM_EXPR, above; commit a834d9ee).
  - sfz-mix WAVs didn't depend on gcs2sfz -- RESOLVED 2026-09-18 (rule now
    lists $(GCS2SFZ)).
  - Per-instrument per-level loudness table -- DONE 2026-09-17 (instruments.
    LEVEL_GAIN, flat stem gain + CC11 residual; in-range instruments within
    ~0-2 dB across mp..fffff; repro notes in
    DEVIN-2026-09-17-sfz-drums-loudness.md).
  - Per-note articulation column -- RESOLVED 2026-09-19 (10th CSV column;
    commit 9be0a4f7), plus the named-level machinery (instruments.NAMED_LEVELS
    / dynamic_vol / level_gain_db_named / per-instrument LEVEL_VELOCITY
    velocity map, calibrated 2026-09-20 see below). DOALL 0 bare / 2
    pre-existing named; full `make sfz` in b/01.
  - Per-instrument LEVEL_VELOCITY calibration -- DONE 2026-09-20. Only the 5
    patches with REAL velocity layers got rows (french_horn/trumpet/
    brass_section: 75-crossfade, mp anchored 70->68; trombone: 75/90
    crossfade, mp 70->68 and f 90->93 staying clear of the blend; pizzicato:
    54/62 crossfade, p 60->58 centered); the ~12 single-layer instruments are
    documented as intentionally absent (velocity == pure loudness there,
    LEVEL_GAIN owns it). Moves are <=0.5 dB, within measurement noise, so
    LEVEL_GAIN was untouched. Note: the fff/ffff pizzicato inversion at high
    velocity is a pre-existing patch/measurement quirk (those velocities are
    identity), not a LEVEL_VELOCITY regression. See
    DEVIN-2026-09-20-sfz-level-velocity.md.
  - Stage/ensemble simulation -- DONE 2026-09-19 (11th staff column +
    per-desk detune + per-staff pan spread; see the section above). Each desk
    internally in tune, tuned slightly differently from its neighbors (no two
    desks on the same tuning), one reference desk at 0. Full sfz+`sfz-mp4` in
    b/01 (v1-1..v1-4) and b/09 (b9m1..b9m3): violin 8 desks [-4..+4], viola 3-4,
    winds 2 each, pizz 9 desks; cello/contrabass/timpani single-staff
    untouched; GCS2SFZ_ENSEMBLE=0 restores pre-feature output; legacy 9/10-col
    CSVs render unchanged; DOALL 0 bare / 2 pre-existing named.

Open:
  - ppp/pppp residuals (~3-8 dB on some instruments) -- likely the slow VPO
    soft attack vs the 0.25s measurement window; re-measure with a longer
    window.
  - A/B-listen the ensemble detune/pan choices across pieces after the full
    suite re-render (130 mixes + 130 sfz-mp4s) and tune the detune magnitude /
    pan spread to taste (currently +/-8 cent, +/-10 pan, deterministic).
  - ARTIC_MAX=0 is not overridden by anything; switching the GM fallback to a
    sfizz-renderable GM soundfont (or a default VPO piano) would drop the
    last GM-dependent instruments.