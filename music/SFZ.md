*) how to make symphony sound better? (Fluidsynth)
    Here are the free orchestral soundfonts and tips for simulating a 14-violin
    section in FluidSynth that we discussed:
      High-Quality Free Orchestral SoundFonts:
        - Aegean Symphonic Orchestra (ASO): Best for full symphonic pieces and
          highly realistic orchestral panning.
          (https://www.google.com/url?q=https://sites.google.com/view/hed-sounds/aegean-symphonic-orchestra&source=gmail&ust=1788278402290000&sa=E)
        - Virtual Playing Orchestra: Best for deep instrument sections with
          various playing styles (longs, shorts, pizzicato).
          (https://www.google.com/url?q=http://virtualplaying.com/&source=gmail&ust=1788278402290000&sa=E)
        - HQ Orchestral Soundfont Collection v3.0: Best for a massive all-in-one
          classic arrangement toolkit (~500MB compiled symphonic compilation).
          (https://www.google.com/url?q=https://musical-artifacts.com/artifacts/817&source=gmail&ust=1788278402290000&sa=E)
        - Cadenza Strings: Best for layering onto existing instruments to build
          physical depth in high-register string ranges.
          (https://www.google.com/url?q=https://www.zanderjaz.com/downloads/soundfonts/orchestral/&source=gmail&ust=1788278402290000&sa=E)

    Pro Tips for Simulating a 14-Violin Section in FluidSynth:
    1. The Layering and Detuning Trick: Load your string soundfont onto three
       or four different FluidSynth MIDI channels. Copy your violin track across
       them, but slightly detune each channel by a few cents
       (e.g., +4, -3, +7, -6 cents).
    2. Humanize the Timing: Micro-delay the note start times slightly across
       those channels.
    3. Stereo Panning: Pan some channels slightly left and others slightly right
       to fill out the stereo field like a real stage layout.
    4. Always Add Reverb: Turn on FluidSynth’s built-in reverb engine or route
       the audio through a spatial reverb effect.

*) is there new midi yet? New instruments?
    Here is the comprehensive guide on using SFZ formats and modern engines for
    playing music with your GCS format parser, including the requested syntax
    mapping layout example for your orchestral arrangements:

  The Reality Behind MIDI 2.0 "Sounds"
    MIDI 2.0 itself doesn't contain audio waveforms or sample files; it introduces
    32-bit controller resolution (over 4 billion velocity steps instead of MIDI
    1.0's 128) and per-note articulation tuning. To get those rich, stunning
    orchestral textures into your application, you can transition your engine
    from running standard .sf2 files inside FluidSynth to loading SFZ instruments
    or utilizing a modern software sampler engine.

    Step 1: Upgrade to the SFZ Format & Modern Engines
    Instead of using FluidSynth's aging architecture, you can target the
    open-source SFZ format. SFZ instrument text files point to collections of
    pristine audio samples. They allow massive, complex velocity layering and
    continuous crossfading, mimicking the high-resolution realism promised by MIDI 2.0.

    Replace FluidSynth backend with one of these software samplers:
    - Sfizz Engine
      (https://www.google.com/url?q=https://sfztools.github.io/sfizz/&source=gmail&ust=1788280061239000&sa=E):
      highly optimized, open-source C++ sampler library explicitly built for the
      SFZ format. It runs incredibly well from scripts, code, or command lines,
      making it easy to feed your parsed GCS note events drectly into it via
      raw audio buffers or API loops.
    - Decent Sampler
      (https://www.google.com/url?q=https://www.decentsamples.com/product/decent-sampler-plugin/&source=gmail&ust=1788280061239000&sa=E):
      A lightweight engine that runs on a cleanly structured, open XML file
      format. It handles gorgeous, deeply sampled custom orchestras and can be
      automated easily.

    Step 2: Load Premium Free Orchestral SFZ Libraries
    Once you hook up an SFZ player like Sfizz to your code, you can load massive,
    world-class sample libraries that make your 62-instrument arrangements sound
    phenomenal:
    - Versilian Studios Chamber Orchestra 2 (VSCO2 Community Edition): A highly
      detailed, open-source orchestral library. It features individual samples
      for different bowing techniques (staccato, sustain, vibrato, pizzicato)
      across all major instruments.
    - Virtual Playing Orchestra
      (https://www.google.com/url?q=http://virtualplaying.com/&source=gmail&ust=1788280061239000&sa=E):
      A completely pre-mapped orchestral package designed to make synthesized
      scoring sound incredibly realistic. It handles vast string sections
      cleanly without sounding synthetic.

    Step 3: Map your GCS Parser via the "CLAP" or Open API Pipeline
    If you are writing pure code to convert GCS commands into real-time playback,
    look into the open-source CLAP (Clever Audio Plug-in) standard
    (https://www.google.com/url?q=https://github.com/free-audio/clap&source=gmail&ust=1788280061239000&sa=E).
    Unlike older plugin systems, CLAP natively supports full MIDI 2.0 configurations,
    MPE (MIDI Polyphonic Expression), and per-note parameter modulations out of the box.

By compiling your parser to drive an open-source host or linking it to a library
like TinySoundFont
(https://www.google.com/url?q=https://github.com/schellingb/TinySoundFont&source=gmail&ust=1788280061239000&sa=E)
(which lets you pull raw floating-point audio data straight from an instrument
pool without standard MIDI channels), you can use your custom layout to control
high-definition orchestral samples directly.

===============================================================================
CORRECTIONS & PLAN (added 2026-09-03) — read this before trusting the above.
The text above is raw Google-query output. A full reviewed plan with the
critical implementation constraints lives in `DEVIN-2026-09-03-sfz.md`. Key
corrections:

1. CLAP is irrelevant here — it's a plugin-host format, not a render/output
   target for a headless WAV render.
2. Decent Sampler is closed-source with no useful CLI — skip it.
3. The "MIDI 2.0" framing is misleading. SFZ is orthogonal to MIDI 2.0; we do
   NOT need MIDI 2.0. SFZ is a *sampler* instrument format (text → audio
   samples), nothing to do with the MIDI protocol version.
4. THE KEY TOOL MISSING: **`sfizz_render`** — the CLI that ships with sfizz:
       sfizz_render --sfz inst.sfz --midi in.mid --wav out.wav
   This is what makes SFZ+MIDI→WAV practical headlessly. Networked above all
   the TTS/plugin talk.
5. Missing **Sonatina Symphonic Orchestra** (CC0, CC1-dynamics):
   https://github.com/peastman/sso — good for realistic crescendos in brass/
   strings (timbre change, not just volume).
6. MISSING HARD CONSTRAINT: **MIDI 1.0 = 16 channels/port (15 melodic; 9 =
   drums).** Beethoven and 1812 have more instruments/voices than that. A plain
   .mid can't put every staff on its own channel. The plan recommends bypassing
   .mid (emit per-instrument timed events directly for per-instrument
   sfizz_render + stem-mix), instead of trying to squeeze into 16 channels.
7. 1812's cannon and church bells are in NO free SFZ library — need custom
   one-shot WAV files mapped to a trivial SFZ region.

===============================================================================
PIPELINE & TARGETS (added 2026-09-09) — now wired into all piece dirs.
===============================================================================

`gcs2sfz` (music/sfz/gcs2sfz) drives the whole SFZ render:

  imscomp --sfzpipecsv piece.E -> per-instrument CSVs (sfz-csv/<piece>/)
      CSV schema: start,dur,midi_note,velocity,pan,reverb,vol_start,vol_end,pan_end
      (9 columns). pan = MIDI CC10, reverb = MIDI CC91, taken verbatim from the
      score's macros; pan 36/42/64/85/92 violin/2nd/viola/cello/bass plus
      woodwinds ~55-60, brass ~62-65, percussion ~35-50 give an orchestral L/R
      spread with a moderate hall send; adjustable per macro in
      b/instruments.include.
      vol_start/vol_end/pan_end capture the expression value AT the note
      (CC11 snapshot at note-on/note-off) plus the pan CC10 at note-off, so
      in-note crescendo/diminuendo and pan sweeps survive into the render.
      imscomp's humanized CC11/CC10 per-tick ramps (do_midi_vpi) are read from
      the shared event stream; a note's vol_start==vol_end means flat
      expression for that note (no extra MIDI emitted by gcs2sfz).
  gcs2sfz sfz-csv/<piece>/ --sfzdir <VPO lib> --outdir sfz-mix/ -> mixed WAV
      - VPO-mapped instruments: sfizz_render per instrument, mixed with
        ffmpeg (amix + loudnorm implicit chain).
  Voice/measure selection: `imscomp --sfzpipecsv` honors the SAME shared
      selection the midi path uses — no sfz-specific wiring needed. The voice
      filter lives in the shared note-print loop (imscomp ~line 6568) and the
      measures filter at parse time, so `--sfzpipecsv --voices 14` or
      `--measures 3` (or combined) slice the per-instrument CSVs correctly
      (verified on b/01 v1-1 and synthetic tests). Voice args are NUMBERS only
      (`--voices 14`); names are not accepted in either path (0 files).
      - Pan: write_midi emits CC10; sfizz honors CC10 -> pan by default
        (sfizz #475 linkage), verified L<center<R. fluidsynth also honors it.
      - In-motion dynamics (2026-09-10): write_midi emits the note's expression
        as CC11=vol_start and linearly ramps CC11 vol_start->vol_end and CC10
        pan->pan_end across the note [on,off). Audio-verified with VPO violin
        (sustain): CC11 40->120 swells RMS by ~1.5x while CC11 flat decays
        slightly; pan 30->100 sweeps audibly L->R. CC11 (via amplitude_oncc11)
        and CC10 both respond mid-note in sfizz; fluidsynth/GM likewise.
        (On the GM fallback the per-note CC11=vol_start emission is skipped —
        see the Expression / stutter-fix notes below.)
      - Reverb: VPO has no reverb effect in this sfizz build, and ~/bin/ffmpeg's
        afir mutes the dry signal, so the room is an offline aecho
        early-reflection tail baked into each instrument WAV scaled by
        reverb/127 (dry at unity; only echoes scale). The GM fallback gets
        real reverb from fluidsynth's CC91 handling.
      - GM fallback (new): instruments with NO VPO mapping (default
        "acoustic grand piano", "bell tower", "church bell", any unknown
        instrument) render through fluidsynth + GeneralUser.sf2 with an
        inserted General MIDI program change (0xC0). Detects the sf2
        automatically: /Users/m4/src/GeneralUser/GeneralUser.sf2 (Darwin) or
        /home/m4/src/GeneralUser_GS/GeneralUser.sf2 (Linux).
      - Articulation (2026-09-11): VPO sustain patches attack 0.2-0.6s and
        release 0.6-2.25s, blurring fast (16th/32nd) melodic runs - the melody
        was present (often LOUDER than GM) yet inaudible because every onset
        smeared into the previous note. Default recipe now splits each part by
        note length (GCS2SFZ_ARTIC_MAX=0.5): notes shorter than the threshold
        render staccato blended under a fast-attack sustain
        (GCS2SFZ_ARTIC_BLEND=1, GCS2SFZ_ATTACK=0.03), summing the staccato
        onset with the sustain body so the line neither smears nor decays to
        silence; longer notes use the fast-attack sustain alone. Verified on
        b/01 v: the violin melody now pulses (mix RMS 0.022-0.034 vs the old
        flat 0.004-0.008) and reads clearly against the texture. Env: set
        ARTIC_MAX=0 to disable the split, ARTIC_BLEND=0 to disable blending,
        ATTACK=0 to disable the attack override.
      - Expression (2026-09-15/16): sfizz (VPO wrapper `amplitude_oncc11=100`)
        applies CC11 LINEARLY in amplitude while GM/fluidsynth applies it
        exponentially, so the same vol_start..vol_end ramp rendered ~2.5 dB on
        sfizz vs ~-9 dB on GM (b/01 horn fp->p 91->61). write_midi now
        predistorts the emitted CC11 as 127*(v/127)^E
        (GCS2SFZ_EXPR_EXP, default 4.0) on the sfizz path so the fp->p contrast
        matches GM — measured on the b/01 `v` slice: SFZ E=4.0 = 12.2 dB vs GM
        12.5 dB (beat1-vs-beat5). Velocity, CC10, CC91 untouched (velocity-layer
        patches keep their timbral response).
      - GM-fallback stutter fix (2026-09-16): the GM path passes
        pred_expr=False and SKIPS the per-note CC11 emission entirely. The sfz
        CSV merges every voice of one instrument onto a SINGLE MIDI channel,
        while the `.fs` uses one channel per voice; emitting vol_start before
        every note-on toggled CC11 every ~90ms between voices (accompaniment
        pp vs melody mf) and pumped ALL sustained notes on that channel — the
        audible "stutter" ~26s into Chopin. Per-note dynamics already ride on
        note velocity; only CC10 pan sweeps, CC91 reverb, and in-note
        cresc/dim ramps still get CC events on the GM path.
      - GM-fallback loudness (2026-09-16): two knobs make the fallback match
        the `.fs` render. `GCS2SFZ_GM_GAIN` (default 0.5) sets fluidsynth's
        `-g` — its default is 0.2, which left every fallback stem ~8 dB quiet
        (20*log10(0.2/0.5)); the `.fs` emits `synth.gain 0.5`. And because
        CC11 is skipped, the expression level never reaches the synth, so
        write_midi folds it into the note-on velocity as
        `vel * (vol/127)^GCS2SFZ_GM_EXPR` (default 1.3, calibrated on
        ims/test-volume-levels: pppp +1.5 dB vs GM, mp within 1 dB, ppppp
        silent). Full details: DEVIN-2026-09-15-cc11-sfz-loudness.md. Full suite (130
        mixes + 130 sfz-mp4s) re-rendered; DOALL clean.
      - Per-instrument gain (2026-09-11): GCS2SFZ_GAIN="name:db,name:db"
        (e.g. GCS2SFZ_GAIN="violin:5") applies an ffmpeg volume ramp to each
        rendered stem before mixdown for quiet libraries (Sonatina strings).
  %-sfz.mp4: black 1920x1080 bg + ASS title overlay (fs2ass) + waveform
        (wav2waveform) + audio from the mix WAV -> x264+aac.
        (Fixed 2026-09-10: was feeding fs2ass the `.E` file, which has no
        echo/sleep lines — fs2ass failed and the sfz mp4 fell back to a plain
        static title. Now routes through `gcs2youtube -a <mix.wav> -o ... <_2.fs>`
        (new `--audio` option) so the sfz video gets the SAME overlay as the GM
        mp4: ALL title lines from the .E as a persistent heading, scrolling
        measure echoes, and the centre playhead arrow.)
      - One-shots (1812 canon + church bells): placements play the FULL sample (no atrim --
        cannon/gunshot/explosion tails ring out; a boom IS the sound) scaled by
        velocity, with a part-level limiter (0.95 FS). Exception: `church bells`
        (in `ONESHOT_TRIM_TO_DUR`) are trimmed to their SCORED duration, because
        the 31.6s church-bells.wav peal struck as ~2.7s notes would otherwise
        smear over the whole finale and ring 30s past the canon (fixed
        2026-09-18: e-sfz.mp4 882s -> 868s, ending on the orchestra's final
        chord). Mix target length derives from the actual rendered stems
        (ffprobe), so natural tails are never trimmed off.

Make targets (now in EVERY piece dir: songs/, ims/, b/01..04, b/06, b/sonata14,
b/09, t/e):

  make sfz         - render all pieces in this dir to sfz-mix/*-sfz-mix.wav
  make sfz-mp4     - encode sfz-mix/*.wav -> *-sfz.mp4
  make sfz-clean   - rm -rf sfz-csv/ sfz-mix/ *-sfz.mp4

Every piece/tool Makefile sets `.DEFAULT_GOAL := help` (top of file), so a bare
`make` (no goal) prints the help text instead of starting a VPO render — the
sfz targets come before `help` in the file and would otherwise win default-goal
selection (fixed 2026-09-10 across songs/, ims/, b/01..04, b/06).

ims/ also has:
  make sfz-tests       - render the TESTS list too
  make sfz-all         - SONGS + TESTS
  make sfz-mp4-tests / sfz-mp4-all - same but encoide to mp4

Top-level music/: `make sfz` / `make sfz-mp4` / `make sfz-clean` recurse into
`songs ims t b` (b recurses into its subdirs).

Known limitations:
  - A0/A1 are intentional error-injection tests; they fail `make tests` too
    (pre-existing). DECODE/ENCODE/L1/P2/STAFF/vc1-test also fail the plain
    `%.fs` compile — none are SFZ regressions.
  - The %-sfz.mp4 ffmpeg recipes need the source-built ~/bin/ffmpeg (ASS/
    drawtext filters); the brew ffmpeg lacks them.
  - sf2 path is auto-detected by $HOME layout; --sf2 overrides.
  - imscomp's --midi1csv/velocity: write_midi emits note velocity AND CC11
    = velocity so GeneralUser responds to dynamics.
  - `print_out_sfzpipecsv` note pairing (fixed 2026-09-10): a note whose
    note-off lands on a DIFFERENT channel than its note-on was dropped. This
    happens at pizz→arco switches: imscomp emits the closing note-off on the
    NEW (arco) channel while the note-on was on the OLD (pizz) channel, so the
    exact-key `(voiceon, chan, pitch)` lookup missed it and the note stayed in
    `pending_notes` forever. `release_sfz_note()` now falls back to the oldest
    pending `(voiceon, pitch)` across channels and attributes it to its START
    channel. Verified: full b/01 v1-1 run was 17269 sfz rows vs 17277 midi
    note-ons (8 missing) -> now identical pitch/vel multiset; `--voices 14`
    went 2056 -> 2057.

===============================================================================
NEXT FIXES / OPEN ITEMS for the SFZ path (as of 2026-09-17)
===============================================================================

**Current TODO (2026-09-17)** — full session record in
`DEVIN-2026-09-17-sfz-drums-loudness.md`:
- [x] SFZ drums split per instrument + voiced on VPO patch keys.
- [x] GM-fallback CC11 in-note ramp stale-state bug fixed (`piano` was silent
      after note 1 in `test-volume-levels`).
- [x] DOALL clean (0 bare / 2 pre-existing named).
- [x] **Per-instrument, per-level loudness normalization to the piano** (all 12
      `test-volume-levels` levels, interpolated for crescendos) — see item 4.
      Implemented as a flat stem gain (= the gain at vol 127, where CC11 has
      no headroom) plus a CC11 residual; in-range instruments now within ~0–2 dB
      across mp..fffff.
- [ ] ppp/pppp residuals (~3–8 dB on some instruments) — likely the slow VPO
      soft attack vs the 0.25 s measurement window; re-measure with a longer
      window before tuning the table further.
- [x] **Per-note articulation column + patch choice** (2026-09-19) — see item 5:
      `--sfzpipecsv` 10th column (`staccato/marcato/tenuto/accent/legato/tied/
      sustain`) from the score's suffixes; `gcs2sfz` picks staccato vs sustain
      per note (duration split stays as the no-column fallback). Also added the
      named-level machinery: `instruments.NAMED_LEVELS` (ppppp..fffff ↔ vol),
      `dynamic_vol`/`level_gain_db_named`, and the per-instrument
      `LEVEL_VELOCITY` table + `velocity_mult` (applied in `write_midi`,
      identity until calibrated).
- [ ] Calibrate per-instrument `LEVEL_VELOCITY` entries (velocity-map the named
      dynamics per instrument so each patch's pp..fff lands in its sample-layer
      ranges) — currently identity; use ims/test-volume-levels like LEVEL_GAIN.
- [x] **Out-of-range note warning** (2026-09-18): `gcs2sfz` parses each VPO
      patch's lokey..hikey span and warns when the CSV writes notes outside it
      (`warn_out_of_range` + `sfz_keyrange`, skipping kit drums and one-shots).
      On `test-volume-levels`: piccolo 6/6 out (patch d5..c8), timpani 5/6
      (c2..c4), tuba 4/6 (d1..d4), contrabass 1/6 (a4 > g4). The test piece
      writes C4–A4 on every instrument, so these notes simply have no VPO
      sample and stay silent; the warning surfaces them instead of failing
      silently. GM-fallback for out-of-range notes is NOT implemented (deferred
      — offer if a piece genuinely needs it). These stay out of `LEVEL_GAIN`.

Open bugs / quality items, in rough priority order:

1. ~~**b/01 v1-1 first-measure pizzicato — 2nd pizz note too loud.**~~ **RESOLVED
   2026-09-19** — root cause was an inverted velocity-layer balance in the VPO
   cello section of the merged `all-strings-SEC-pizzicato[-panned].sfz` patches:
   the low-velocity layer (xfout_lovel=0, hivel=62) carries per-region volume
   ~23-30 while the high-velocity layer (xfin_lovel=54, hivel=127) carries
   ~10-18, so a soft pluck (vel 54-62, both layers active) renders louder than a
   strong one (63+). The soft 2nd note (vel 57-64) lands exactly on that
   inverted boundary. Measured on a cello-only sweep (p50): v62=47.2dB ->
   v64=40.6dB, a 6.6dB drop as velocity increases. `gcs2sfz._cello_velocity_fix`
   now rewrites each low-velocity cello region's `volume=` to match its
   high-velocity partner (matched by lokey/hikey) at render time, making
   velocity->loudness monotonic (sfizz's default amp_veltrack supplies the
   range); applies to the SEC + panned pizz patches, no library edit needed.
   Original report: 2026-09-11 (see DEVIN-2026-09-11-sfz-articulation.md).
   Repro: `make sfz` in b/01, A/B the first-measure pizz phrase against the
   GM render (`--sfzpipecsv --measures 1` slice).

2. ~~GM-fallback rendered loudness sits ~10 dB below the `.fs` render.~~ **RESOLVED
   2026-09-16** — was two compounding causes, both fixed in `a834d9ee`:
   fluidsynth's default master gain 0.2 vs the `.fs`'s `synth.gain 0.5`
   (`GCS2SFZ_GM_GAIN`, ~-8 dB), plus the skipped per-note CC11 leaving
   expression stuck at 127 → dynamics compressed (`GCS2SFZ_GM_EXPR` folds the
   expression into note velocity). Calibrated on ims/test-volume-levels.

3. ~~**`sfz-mix/%-sfz-mix.wav` does NOT depend on `gcs2sfz`.**~~ **RESOLVED
   2026-09-18** — the mix rule only listed `sfz-csv/%/.done`, so edits to
   `gcs2sfz` did NOT trigger a re-render (`make sfz` said "Nothing to be
   done"). `$(CURDIR)/$(GCS2SFZ)` is now a prereq of every mix rule
   (songs/, ims/, b/01..04, b/06, b/09, b/sonata14, t/e).

Carried-over improvements (not regressions):

4. **Per-instrument per-level loudness table — DONE 2026-09-17.** A flat
   per-instrument gain was not enough: the SFZ/VPO instruments' loudness *curve*
   differs from the piano's, so the correction is **per level** and interpolated
   between the 12 `test-volume-levels` points. Target = the piano stem
   (`acoustic_grand_piano`); each instrument's dB offset lives in
   `instruments.LEVEL_GAIN`. The correction is split into a **flat stem gain**
   (`base_gain_db` = gain at vol 127, applied in `gcs2sfz.main` alongside
   `GCS2SFZ_GAIN`) plus a **CC11 residual** (`write_midi._pred`), because the
   predistorted CC11 is already ~127 at the top dynamics and cannot deliver a
   boost there. In-range instruments now match within ~0–2 dB over mp..fffff.
   Repro/notes: `/tmp/calibrate2.py` method in
   `DEVIN-2026-09-17-sfz-drums-loudness.md`.
5. ~~**Articulation choice is duration-only; the CSV has no per-note
   legato/staccato marking, so slurred (`l`) and marked-staccato short notes
   render the same.**~~ **RESOLVED 2026-09-19** — `--sfzpipecsv` now appends a
   10th per-note `articulation` column to every `Note_on_c` (only in sfzpipecsv
   mode; midi1csv/fluidsynth/abc outputs stay byte-identical and DOALL stayed
   0 bare / 2 pre-existing named). The token comes from the note's score
   suffixes (`sfz_articulation_token`: `s`→staccato, `m`→marcato, `u`→tenuto,
   `a`/`A`→accent, `l`/`z`→legato, `t`→tied, else sustain). `gcs2sfz` reads
   the column (optional; older 9-col CSVs keep the `GCS2SFZ_ARTIC_MAX`
   duration split) and renders staccato/marcato notes with the staccato patch,
   the rest with sustain. Also added `instruments.NAMED_LEVELS`/named-level
   gain lookup and a per-instrument `LEVEL_VELOCITY` velocity map (identity by
   default; see the open items). DOALL-verified and full `make sfz` in b/01.
6. `ARTIC_MAX=0` to disable is overridden by nothing today; switching the
   GM fallback of unmapped instruments to use sfizz's GM-capable soundfonts
   (or a default VPO piano) would remove the last GM-dependent instruments.
