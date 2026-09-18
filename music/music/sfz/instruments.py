import os

"""Map imscomp instrument names to VPO (Virtual Playing Orchestra) SFZ files.

imscomp's ``--sfzpipecsv`` mode emits one CSV per instrument, named after the GM
instrument name (lowercased, e.g. ``french horn``, ``pizzicato strings``). This
module maps those names to the appropriate VPO .sfz patch so the ``gcs2sfz``
renderer can load a real sample library instead of the GM GeneralUser synth.

VPO library root (set at look-up time, not hardcoded here):
    music/sfz/library/VPO/Virtual-Playing-Orchestra3/

Family folders: Brass/, Strings/, Woodwinds/, Percussion/, Keys/, Vocals/

Naming convention in VPO:
    <instrument>-<SOLO|SEC>-<articulation>.sfz
    - SOLO = single player, SEC = section (ensemble)
    - articulation: sustain, normal-mod-wheel, accent, staccato, pizzicato,
      tremolo (strings), KS-Cn (key-switch / all articulations on one patch)

Defaults chosen: section (SEC) patches where available (orchestral feel),
sustain articulation (continuous lines). If a piece has no pitch data for an
instrument, the note velocity drives the sample.
"""

# Emitted GM instrument name (lowercased) -> relative path under the VPO root.
# Absolute paths containing no slash are treated as one-shot sample files kept
# in music/sfz/library/... (see EFFECT_* below).
MAP = {
    # ---- Woodwinds ---------------------------------------------------------
    "flute":            ("Woodwinds", "flute-SEC-sustain.sfz"),
    "oboe":             ("Woodwinds", "oboe-SEC-sustain.sfz"),
    "clarinet":         ("Woodwinds", "clarinet-SEC-sustain.sfz"),
    "bassoon":          ("Woodwinds", "bassoon-SEC-sustain.sfz"),
    "piccolo":          ("Woodwinds", "piccolo-SOLO-sustain.sfz"),
    "english horn":     ("Woodwinds", "english-horn-SOLO-sustain.sfz"),

    # ---- Brass -------------------------------------------------------------
    "french horn":      ("Brass", "french-horn-SEC-sustain.sfz"),
    "trumpet":          ("Brass", "trumpet-SEC-sustain.sfz"),
    "trombone":         ("Brass", "trombone-SEC-sustain.sfz"),
    "bass trombone":    ("Brass", "bass-trombone-SOLO-sustain.sfz"),
    "tuba":             ("Brass", "tuba-SOLO-sustain.sfz"),
    "brass section":    ("Brass", "all-brass-SEC-sustain.sfz"),

    # ---- Strings -----------------------------------------------------------
    "violin":           ("Strings", "1st-violin-SEC-sustain.sfz"),
    "viola":            ("Strings", "viola-SEC-sustain.sfz"),
    "cello":            ("Strings", "cello-SEC-sustain.sfz"),
    "contrabass":       ("Strings", "bass-SEC-sustain.sfz"),
    "pizzicato strings":("Strings", "all-strings-SEC-pizzicato.sfz"),

    # ---- Percussion --------------------------------------------------------
    "timpani":          ("Percussion", "timpani-hit.sfz"),

    # ---- VPO patches for GM percussion/effects where they map cleanly -------
    "glockenspiel":     ("Percussion", "glockenspiel.sfz"),
    "xylophone":        ("Percussion", "xylophone.sfz"),
    "tubular bells":    ("Percussion", "tubular-bells.sfz"),
    "vibraphone":       ("Percussion", "vibraphone-open.sfz"),

    # ---- One-shot effects (no orchestral VPO sample) -----------------------
    # These are rendered from the standalone WAV files in library/EFFECTS/.
    "canon":            "EFFECTS/canon.wav",
    "gunshot":          "EFFECTS/gunshot.wav",
    "explosion":        "EFFECTS/explosion.wav",
    "church bells":     "EFFECTS/church-bells.wav",
}

# cymbal / drumkit GM names that map to VPO percussion (rough).
# KIT_FALLBACK maps name -> patch; KIT_KEY additionally pins the MIDI key each
# drum must be voiced on. imscomp emits the GM drum key (35 bass, 38 snare, 49
# crash, 54 tambourine, 81 triangle, ...) but the VPO patches are voiced on
# their own keys (e.g. bassdrum.sfz on c2/d2, snare.sfz on c3..), so the pitch
# in the CSV would hit no region (silent). KIT_KEY overrides the written note
# with the patch's fixed key, ignoring the score pitch (audio renders, and every
# drum of a kind sounds identical as a one-shot should).
KIT_FALLBACK = {
    "acoustic bass drum": "Percussion/bassdrum.sfz",
    "acoustic snare":     "Percussion/snare.sfz",
    "crash cymbal 1":     "Percussion/cymbals.sfz",
    "crash cymbal 2":     "Percussion/cymbals.sfz",
    "ride cymbal 1":      "Percussion/cymbals.sfz",
    "ride cymbal 2":      "Percussion/cymbals.sfz",
    "tambourine":         "Percussion/misc.sfz",
    "triangle":           "Percussion/misc.sfz",
    "open triangle":      "Percussion/misc.sfz",
}

# Patch key each KIT_FALLBACK drum must be voiced on (see the comment above).
# Chosen to land on a generic hit sound from the VPO registers:
#   bassdrum.sfz  c2 (36)  or  d2 (38)   -> c2 for the plain SSO hit
#   snare.sfz     c3 (48)  (snare-lh mf/ff hit)
#   cymbals.sfz   f#4 (66) (cymbal-crash1) for crash; g#4 (68) (susCymb1 hit) for ride
#   misc.sfz      a2 (45)  (Triangle3 hit) for triangle; f#3 (54) (Tamb1 hit) for tambourine
KIT_KEY = {
    "acoustic bass drum": 36,      # c2
    "acoustic snare":     48,      # c3
    "crash cymbal 1":     66,      # f#4
    "crash cymbal 2":     66,      # f#4
    "ride cymbal 1":      68,      # g#4
    "ride cymbal 2":      68,      # g#4
    "tambourine":         54,      # f#3
    "triangle":           45,      # a2
    "open triangle":      45,      # a2
}

# GM program numbers for instruments with no VPO/one-shot mapping. Rendered via
# the GeneralUser.sf2 fallback in gcs2sfz (--sf2). Keyed by the normalized
# (underscores->spaces, lowercased) instrument name; anything unmapped defaults
# to program 0 (Acoustic Grand Piano).
GM_PROGRAM = {
    "acoustic grand piano": 0,
    "grand piano": 0,
    "bell tower": 14,          # tubular bells
    "church bell": 14,
}
GM_DEFAULT = 0


def gm_program(instrument_name: str) -> int:
    name = (instrument_name or "").strip().lower().replace("_", " ")
    return GM_PROGRAM.get(name, GM_DEFAULT)


def kit_key(instrument_name: str):
    """Return the fixed MIDI key a drum instrument must be voiced on, or None.

    imscomp emits GM drum keys (35 bass, 38 snare, ...) in the CSV, but the VPO
    drum patches are voiced on their own keys (bassdrum c2=36, snare c3=48,
    ...). Notes played at the wrong key hit no region -> silent. When a piece
    later plays BOTH drums of one kind (e.g. two bass drums) they share the CSV
    pitch, but a one-shot drum ignores pitch anyway, so a single fixed key per
    instrument is both correct and simplest.
    """
    name = (instrument_name or "").strip().lower().replace("_", " ")
    return KIT_KEY.get(name)


VPO_ROOT = "Virtual-Playing-Orchestra3"


def vpo_rel_path(instrument_name: str) -> str:
    """Return the VPO-relative path (no leading slash) for an instrument name.

    Result is relative to the VPO root directory, e.g.
    "Strings/1st-violin-SEC-sustain.sfz". One-shots return a path still relative
    to the main library root (music/sfz/library).
    """
    name = (instrument_name or "").strip().lower()
    # imscomp's CSV filenames use underscores (french_horn, pizzicato_strings);
    # the MAP keys use spaces (french horn, pizzicato strings). Normalize.
    name = name.replace("_", " ")
    hit = MAP.get(name)
    if hit is None:
        hit = KIT_FALLBACK.get(name)
    if hit is None:
        # Unknown instrument: return None so caller can fall back to GM sf2.
        return None
    if isinstance(hit, str):
        # one-shot (EFFECTS/...)
        return hit
    family, filename = hit
    return f"{family}/{filename}"


def resolve(root_library: str, instrument_name: str):
    """Resolve an instrument to an absolute SFZ/WAV path under library root.

    One-shots (EFFECTS/...) live at the library root itself, not inside the
    VPO tree, so they are joined directly under root_library.
    """
    rel = vpo_rel_path(instrument_name)
    if rel is None:
        return None
    if rel.startswith("EFFECTS/"):
        return os.path.join(root_library, rel)
    return os.path.join(root_library, VPO_ROOT, rel)


# Per-instrument loudness normalization to the piano, keyed by the score's
# expression value (vol, 0..127): gain in dB applied to the note's emitted CC11
# so the instrument's rendered loudness matches the piano's at that level.
# Measured from ims/test-volume-levels against the acoustic_grand_piano stem
# (2026-09-17); see DEVIN-2026-09-17-sfz-drums-loudness.md. Points are
# interpolated linearly in vol (and in dB) so crescendos/diminuendos glide
# between levels. Levels the test can't measure (instrument silent, e.g. the
# out-of-range piccolo/timpani high notes) are simply absent -> gain 0.
LEVEL_GAIN = {
    "acoustic_bass_drum": [(28, 12.1), (38, 20.1), (48, 15.7), (59, 9.5), (68, 8.0), (82, 6.9), (89, 2.5), (102, 1.6), (111, 1.0), (118, -1.5), (127, -3.4)],
    "acoustic_snare": [(48, 25.7), (59, 19.8), (68, 19.2), (82, 14.0), (89, 12.7), (102, 12.4), (111, 12.0), (118, 9.1), (127, 7.7)],
    "bassoon": [(28, 12.8), (38, 23.3), (48, 21.5), (59, 16.9), (68, 6.6), (82, 8.0), (89, 9.0), (102, 9.1), (111, 9.6), (118, 1.7), (127, 0.6)],
    "brass_section": [(38, 22.4), (48, 19.4), (59, 14.2), (68, 17.0), (82, 7.3), (89, 5.7), (102, 4.3), (111, 2.1), (118, 2.2), (127, 2.5)],
    "cello": [(38, 20.9), (48, 14.7), (59, 12.2), (68, 12.0), (82, 8.5), (89, 6.7), (102, 4.4), (111, 4.5), (118, 5.4), (127, 4.7)],
    "clarinet": [(28, 12.3), (38, 22.3), (48, 16.4), (59, 9.9), (68, 10.5), (82, 5.6), (89, 5.3), (102, 4.9), (111, 4.5), (118, 2.3), (127, 1.2)],
    "contrabass": [(28, 12.9), (38, 21.8), (48, 16.9), (59, 13.8), (68, 13.9), (82, 11.2), (89, 9.0), (102, 7.4), (111, 4.5), (118, 3.9), (127, 3.9)],
    "crash_cymbal_1": [(48, 21.8), (59, 20.4), (68, 14.5), (82, 10.8), (89, 7.7), (102, 7.7), (111, 5.8), (118, 5.6), (127, 2.0)],
    "english_horn": [(28, 14.4), (38, 19.4), (48, 19.3), (59, 17.6), (68, 3.7), (82, 6.2), (89, 12.8), (102, 8.0), (111, 8.1), (118, 4.1), (127, 3.3)],
    "flute": [(38, 22.8), (48, 17.1), (59, 20.4), (68, 21.8), (82, 19.3), (89, 15.3), (102, 9.3), (111, 8.7), (118, 12.2), (127, 9.7)],
    "french_horn": [(38, 22.3), (48, 16.1), (59, 10.8), (68, 11.6), (82, 9.6), (89, 6.3), (102, 8.4), (111, 6.5), (118, 2.7), (127, 2.7)],
    "oboe": [(38, 24.4), (48, 22.5), (59, 18.1), (68, 12.1), (82, 13.9), (89, 9.8), (102, 11.7), (111, 13.6), (118, 6.7), (127, 8.7)],
    "open_triangle": [(48, 27.4), (59, 23.7), (68, 21.5), (82, 21.3), (89, 20.4), (102, 17.5), (111, 19.1), (118, 16.1), (127, 15.8)],
    "piano": [(28, -3.7), (38, -3.2), (48, 1.1), (59, 0.5), (68, -0.5), (82, -0.4), (89, -1.3), (102, 0.4), (111, 0.5), (118, 0.1), (127, 0.5)],
    "pizzicato_strings": [(38, 23.4), (48, 21.2), (59, 15.1), (68, 10.4), (82, 12.0), (89, 9.6), (102, 8.3), (111, 8.6), (118, 4.8), (127, 5.5)],
    "tambourine": [(38, 24.9), (48, 19.9), (59, 14.9), (68, 13.2), (82, 13.6), (89, 13.9), (102, 14.3), (111, 13.3), (118, 12.2), (127, 10.7)],
    # timpani / tuba / piccolo are deliberately ABSENT: their out-of-range
    # mappings make them measure as silence (huge apparent gain) and a flat base
    # gain would then make their audible notes far too loud. Fix the mappings
    # (see SFZ.md item 8) before adding them.
    "trombone": [(38, 22.2), (48, 14.6), (59, 14.2), (68, 9.3), (82, 8.0), (89, 4.2), (102, 4.4), (111, 4.2), (118, 0.9), (127, 0.2)],
    "trumpet": [(38, 22.1), (48, 14.8), (59, 14.9), (68, 10.5), (82, 14.0), (89, 8.5), (102, 6.1), (111, 4.2), (118, 6.5), (127, 3.6)],
    "viola": [(38, 16.6), (48, 17.5), (59, 12.8), (68, 10.2), (82, 11.3), (89, 9.6), (102, 6.1), (111, 4.4), (118, -0.4), (127, -2.5)],
    "violin": [(38, 24.1), (48, 25.0), (59, 23.3), (68, 25.6), (82, 22.3), (89, 19.3), (102, 18.2), (111, 16.3), (118, 16.9), (127, 13.1)],
}

# Guard rails: the table is a first-order fit from single notes per level and
# the out-of-range instruments measure as silence (huge apparent gain). Cap the
# correction so a bad row can never blow up a render.
LEVEL_GAIN_MIN_DB = -6.0
LEVEL_GAIN_MAX_DB = 30.0


def level_gain_db(instrument_name: str, vol: int) -> float:
    """Per-instrument CC11 gain (dB) for a score expression value ``vol``.

    Returns 0.0 for unmapped instruments (incl. the out-of-range ones the test
    can't measure, and one-shots). Points are interpolated linearly in vol and
    dB; outside the measured range the nearest point is held.
    """
    name = (instrument_name or "").strip().lower().replace(" ", "_")
    pts = LEVEL_GAIN.get(name)
    if not pts:
        return 0.0
    return _interp_gain(pts, vol)


def base_gain_db(instrument_name: str) -> float:
    """Flat stem gain (dB) for an instrument: its gain at the loudest level.

    The per-level correction is emitted through CC11, but the predistorted CC11
    is already near 127 at the top dynamics, so a large boost there cannot be
    added any other way. Splitting the correction into a flat stem gain (taken
    at vol=127, where CC11 has no headroom) plus a CC11 residual (for the lower
    levels, which do have headroom) lets the full curve be realized. Applied
    alongside GCS2SFZ_GAIN in main().
    """
    return level_gain_db(instrument_name, 127)


def _interp_gain(pts, vol: int) -> float:
    if vol <= pts[0][0]:
        g = pts[0][1]
    elif vol >= pts[-1][0]:
        g = pts[-1][1]
    else:
        g = pts[-1][1]
        for (v0, g0), (v1, g1) in zip(pts, pts[1:]):
            if v0 <= vol <= v1:
                f = (vol - v0) / (v1 - v0) if v1 > v0 else 0.0
                g = g0 + (g1 - g0) * f
                break
    return max(LEVEL_GAIN_MIN_DB, min(LEVEL_GAIN_MAX_DB, g))
