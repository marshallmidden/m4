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
