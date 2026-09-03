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
    "pizzicato strings":("Strings", "cello-SEC-pizzicato.sfz"),

    # ---- Percussion --------------------------------------------------------
    "timpani":          ("Percussion", "timpani-hit.sfz"),

    # ---- VPO patches for GM percussion/effects where they map cleanly -------
    "glockenspiel":     ("Percussion", "glockenspiel.sfz"),
    "xylophone":        ("Percussion", "xylophone.sfz"),
    "tubular bells":    ("Percussion", "tubular-bells.sfz"),
    "vibraphone":       ("Percussion", "vibraphone-open.sfz"),

    # ---- One-shot effects (no orchestral VPO sample) -----------------------
    # These are rendered from the standalone WAV files in library/EFFECTS/.
    "gunshot":          "EFFECTS/gunshot.wav",
    "explosion":        "EFFECTS/explosion.wav",
    "church bells":     "EFFECTS/church-bells.wav",
}

# cymbal / drumkit GM names that map to VPO percussion (rough).
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

VPO_ROOT = "Virtual-Playing-Orchestra3"


def vpo_rel_path(instrument_name: str) -> str:
    """Return the VPO-relative path (no leading slash) for an instrument name.

    Result is relative to the VPO root directory, e.g.
    "Strings/1st-violin-SEC-sustain.sfz". One-shots return a path still relative
    to the main library root (music/sfz/library).
    """
    name = (instrument_name or "").strip().lower()
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
    """Resolve an instrument to an absolute SFZ/WAV path under library root."""
    rel = vpo_rel_path(instrument_name)
    if rel is None:
        return None
    return os.path.join(root_library, VPO_ROOT, rel)
