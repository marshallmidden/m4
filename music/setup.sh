#!/usr/bin/env bash
# setup.sh -- install the music pipeline's missing pieces on a fresh clone.
#
# The repo does NOT vendor the third-party sample libraries or the hand-built
# audio tools; this script checks what is present, fetches what can be fetched,
# and documents the source builds. None of it runs unless you pass a --fetch
# flag; the default is a read-only check.
#
#   ./setup.sh              check only (report what's missing, exit 1 if any)
#   ./setup.sh --check      same as above (also useful via `make setup`)
#   ./setup.sh --all        fetch VPO library + GeneralUser.sf2 + sfizz, then check
#   ./setup.sh --vpo        download VPO wave files (616MB) + standard scripts
#   ./setup.sh --sf2        get GeneralUser.sf2 (git clone of the release)
#   ./setup.sh --sfizz      put sfizz_render + libsfizz into ~/bin (macOS)
#   ./setup.sh --notes      print build recipes for the hand-built tools
#
# Non-destructive: fetches write only to music/music/sfz/library/ and ~/bin,
# and never overwrite an existing file unless --force is given.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# AGENTS.md pipeline root is one level up from this file only if the script
# lives in music/music; here it lives at music/, so:
PIPELINE_DIR="$SCRIPT_DIR"                       # music/
SFZ_DIR="$PIPELINE_DIR/music/sfz"                # music/music/sfz
VPO_PARENT="$SFZ_DIR/library/VPO"                # target: .../library/VPO/
VPO_DEST="$VPO_PARENT/Virtual-Playing-Orchestra3"

BINDIR="${BINDIR:-$HOME/bin}"
GENERALUSER_DIR="${GENERALUSER_DIR:-}"
if [ -z "$GENERALUSER_DIR" ]; then
    if [ "$(uname -s)" = "Darwin" ]; then
        GENERALUSER_DIR="$HOME/src/GeneralUser"
    else
        GENERALUSER_DIR="$HOME/src/GeneralUser_GS"
    fi
fi

FORCE=0
while [ "$#" -gt 0 ]; do
    case "$1" in
        --all) DO_VPO=1; DO_SF2=1; DO_SFIZZ=1; CHECK_AFTER=1 ;;
        --vpo) DO_VPO=1 ;;
        --sf2) DO_SF2=1 ;;
        --sfizz) DO_SFIZZ=1 ;;
        --notes) DO_NOTES=1 ;;
        --check) DO_CHECK=1 ;;
        --force) FORCE=1 ;;
        -h|--help) sed -n '2,18p' "$0"; exit 0 ;;
        *) echo "setup.sh: unknown option: $1 (try --help)"; exit 2 ;;
    esac
    shift
done

OS="$(uname -s)"

# --------------------------------------------------------------------------
say_head()  { printf '\n== %s\n' "$*"; }
say_ok()    { printf '   OK       %s\n' "$*"; }
say_miss()  { printf '   MISSING  %s\n' "$*"; }
say_info()  { printf '   info     %s\n' "$*"; }

tool_ok() {
    # Consider a tool present if it is on PATH anywhere OR in $BINDIR.
    command -v "$1" >/dev/null 2>&1 || [ -x "$BINDIR/$1" ]
}

has_dir()  { [ -d "$1" ]; }
has_file() { [ -r "$1" ]; }

# --------------------------------------------------------------------------
check() {
    say_head "Required system tools"
    for t in gcc make python3 git curl unzip tar; do
        if command -v "$t" >/dev/null 2>&1; then say_ok "$t"; else say_miss "$t"; fi
    done

    say_head "Audio/video tools (looked in PATH and $BINDIR)"
    for t in fluidsynth ffmpeg ffprobe sfizz_render csvmidi abcm2ps ps2pdf; do
        if tool_ok "$t"; then say_ok "$t"; else say_miss "$t"; fi
    done

    say_head "Sample libraries"
    if has_file "$GENERALUSER_DIR/GeneralUser.sf2"; then
        say_ok "GeneralUser.sf2 ($GENERALUSER_DIR/GeneralUser.sf2)"
    else
        say_miss "GeneralUser.sf2 -> $GENERALUSER_DIR/GeneralUser.sf2"
    fi
    if has_dir "$VPO_DEST" && has_dir "$VPO_DEST/libs" && has_dir "$VPO_DEST/Strings"; then
        say_ok "VPO library ($VPO_DEST)"
    else
        say_miss "VPO library -> $VPO_DEST  (needs wave files + scripts unzipped merge)"
    fi

    say_head "YouTube upload (only needed for ./youtube-upload / make mp4 upload)"
    if [ -r "$HOME/.config/youtube/client_secrets.json" ]; then
        say_ok "OAuth client_secrets.json present"
    else
        say_info "OAuth client_secrets.json not found -- uploads will need it"
    fi

    if [ "${1:-}" = "exit1" ]; then
        missing=0
        for t in gcc make python3 git curl unzip tar fluidsynth ffmpeg ffprobe sfizz_render csvmidi abcm2ps ps2pdf; do
            tool_ok "$t" || missing=$((missing+1)); 
        done
        has_file "$GENERALUSER_DIR/GeneralUser.sf2" || missing=$((missing+1))
        has_dir "$VPO_DEST/libs" || missing=$((missing+1))
        printf '\n%d missing component(s). Run ./setup.sh --all (or --notes for tool recipes).\n' "$missing"
        if [ "$missing" -gt 0 ]; then exit 1; fi
    fi
    return 0
}

# --------------------------------------------------------------------------
# sfizz_render: build from source at the known-good commit. The 1.2.3 release
# tarball carries a stale Jan-2024 CI binary that predates the "Ensure that
# voices are cleaned up before being force-reused" fix (1.2.3 tag) and can hang
# forever on dense per-pitch note streams (reproduced on b/09). Pin to the
# post-fix HEAD (f5c6e29, 1.2.4-dev) and build a static, copy-able binary.
SFIZZ_SRC_URL="https://github.com/sfztools/sfizz"
SFIZZ_SRC_COMMIT="f5c6e29"
fetch_sfizz() {
    say_head "Building sfizz_render from source (git $SFIZZ_SRC_COMMIT)"
    if tool_ok sfizz_render && [ "$FORCE" -eq 0 ]; then
        say_ok "sfizz_render already available; skip (--force to re-fetch)"
        return 0
    fi
    for t in git cmake g++ make; do
        if ! command -v "$t" >/dev/null 2>&1; then
            say_info "Missing build tool: $t. Cannot build sfizz from source."
            return 1
        fi
    done
    tmp="$(mktemp -d)"
    git clone --recursive "$SFIZZ_SRC_URL" "$tmp/sfizz" >/dev/null 2>&1 \
        || { echo "  FAILED to clone sfizz (network/ GitHub?)."; rm -rf "$tmp"; return 1; }
    git -C "$tmp/sfizz" checkout "$SFIZZ_SRC_COMMIT" >/dev/null 2>&1 \
        || { echo "  FAILED to checkout $SFIZZ_SRC_COMMIT."; rm -rf "$tmp"; return 1; }
    if ! cmake -S "$tmp/sfizz" -B "$tmp/build" -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_FLAGS="-Wno-missing-template-arg-list-after-template-kw" \
          >"$tmp/cmake.log" 2>&1; then
        echo "  FAILED cmake configure (see $tmp/cmake.log)"; rm -rf "$tmp"; return 1
    fi
    if ! cmake --build "$tmp/build" --target sfizz_render -j"$(sysctl -n hw.ncpu 2>/dev/null || echo 4)" \
          >"$tmp/build.log" 2>&1; then
        echo "  FAILED cmake build (see $tmp/build.log)"; rm -rf "$tmp"; return 1
    fi
    mkdir -p "$BINDIR"
    cp "$tmp/build/library/bin/sfizz_render" "$BINDIR/sfizz_render"
    chmod +x "$BINDIR/sfizz_render"
    rm -rf "$tmp"
    say_ok "installed $BINDIR/sfizz_render (verify: $BINDIR/sfizz_render --help)"
}

# --------------------------------------------------------------------------
fetch_vpo() {
    say_head "Fetching VPO library (wave files 616MB + standard scripts)"
    if has_dir "$VPO_DEST/libs" && [ "$FORCE" -eq 0 ]; then
        say_ok "VPO already installed at $VPO_DEST; skip (--force to refetch)"
        return 0
    fi
    mkdir -p "$VPO_PARENT"
    tmp="$(mktemp -d)"
    WAVE="https://archive.org/download/virtual-playing-orchestra-3-2-wave-files/Virtual-Playing-Orchestra3-2-wave-files.zip"
    SCRIPTS="https://virtualplaying.com/vp-downloads/Virtual-Playing-Orchestra3-3-standard-scripts.zip"
    echo "  [1/2] wave files (616MB): $WAVE"
    curl -fL --max-time 3600 "$WAVE" -o "$tmp/vpo-wave.zip" || { echo "  FAILED wave download"; rm -rf "$tmp"; return 1; }
    echo "  [2/2] standard scripts: $SCRIPTS"
    curl -fL --max-time 300 "$SCRIPTS" -o "$tmp/vpo-scripts.zip" || { echo "  FAILED scripts download"; rm -rf "$tmp"; return 1; }
    echo "  unzipping into $VPO_PARENT/ (merge, overwrite yes)"
    unzip -q -o "$tmp/vpo-wave.zip"    -d "$VPO_PARENT" || { echo "  FAILED unzip wave"; rm -rf "$tmp"; return 1; }
    unzip -q -o "$tmp/vpo-scripts.zip" -d "$VPO_PARENT" || { echo "  FAILED unzip scripts"; rm -rf "$tmp"; return 1; }
    rm -rf "$tmp"
    if has_dir "$VPO_DEST/Strings" && has_dir "$VPO_DEST/libs"; then
        say_ok "VPO installed at $VPO_DEST"
    else
        say_miss "unzip did not produce $VPO_DEST/{Strings,libs} -- check zip layout"
        return 1
    fi
}

# --------------------------------------------------------------------------
fetch_sf2() {
    say_head "Fetching GeneralUser.sf2 (v1.47.1)"
    if has_file "$GENERALUSER_DIR/GeneralUser.sf2" && [ "$FORCE" -eq 0 ]; then
        say_ok "GeneralUser.sf2 already at $GENERALUSER_DIR; skip (--force to reclone)"
        return 0
    fi
    mkdir -p "$(dirname "$GENERALUSER_DIR")"
    if has_dir "$GENERALUSER_DIR"; then
        say_info "re-cloning over existing dir (this is destructive -- use --force if intended)"
        [ "$FORCE" -eq 1 ] || { say_info "aborting; pass --force to overwrite"; return 1; }
        rm -rf "$GENERALUSER_DIR"
    fi
    echo "  git clone https://github.com/ad-si/GeneralUser.git -> $GENERALUSER_DIR"
    git clone --depth 1 https://github.com/ad-si/GeneralUser.git "$GENERALUSER_DIR" || return 1
    if has_file "$GENERALUSER_DIR/GeneralUser.sf2"; then
        say_ok "GeneralUser.sf2 installed at $GENERALUSER_DIR/GeneralUser.sf2"
    else
        say_miss "clone succeeded but no GeneralUser.sf2 found in repo"
        return 1
    fi
}

# --------------------------------------------------------------------------
print_notes() {
    cat <<'NOTES'

Hand-built binaries (in ~/bin) -- not fetchable as releases; build recipes:

fluidsynth (patched stdin renderer, REQUIRED for make mp4):
    source:  /Users/m4/src/fluidsynth.m4-editing  (patch vendor'd in-tree)
             also saved-m4-stuff/src/fluidsynth.m4-editing
    build:   cmake -S . -B build2 -DBUILD_SHARED_LIBS=OFF
             copy the resulting fluidsynth binary to ~/bin
    patch notes: music/DEVIN-2026-03-23.md; golang: fluid_cmd_handler_set_renderer().
    stock fluidsynth fails ("No midi file specified!") -- the m4 build is required.

ffmpeg/ffprobe (source-built 9.0.1 with text-rendering filters):
    /tmp/ffmpeg-9.0.1 built with:
      --enable-libass --enable-libfreetype --enable-libharfbuzz
      --enable-fontconfig --enable-libx264 (plus x265/vpx/mp3lame/opus/
       videotoolbox/audiotoolbox)
    the brew ffmpeg build has NO ass/drawtext/subtitles filters, so gcs2youtube's
    ASS overlay fails -- the custom build is required.

sfizz_render (already handled by --sfizz on macOS; Linux needs source build):
    see music/SFZ.md / sfztools docs. CLI: sfizz_render --sfz X.sfz
      --midi in.mid --wav out.wav. The VPO .sfz scripts are machine-independent.

ps2pdf / gs (ghostscript 10.07.1 via brew):
    the repo tools call ps2pdf; on macOS brew's ghostscript ships ps2pdf, which
    is symlinked/copied into ~/bin so `make pdf` works without /opt/homebrew/bin.

csvmidi / abcmidi (MIDI converters):
    built from the in-repo source trees music/abcmidi-m4-64 or from the
    standard abcmidi distro; the m4 builds live at ~/bin/csvmidi.

YouTube upload credential:
    put the OAuth2 client JSON at ~/.config/youtube/client_secrets.json
    (see ./youtube-upload --help).
NOTES
}

# --------------------------------------------------------------------------
if [ -n "${DO_NOTES:-}" ]; then print_notes; fi
action=0
if [ -n "${DO_SFIZZ:-}" ]; then fetch_sfizz; action=1; fi
if [ -n "${DO_VPO:-}" ]; then fetch_vpo; action=1; fi
if [ -n "${DO_SF2:-}" ]; then fetch_sf2; action=1; fi
if [ "$action" -eq 0 ] || [ -n "${DO_CHECK:-}" ] || [ -n "${CHECK_AFTER:-}" ]; then
    if [ "$action" -gt 0 ]; then
        check
    else
        check exit1
    fi
fi