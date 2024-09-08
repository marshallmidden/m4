#!/usr/bin/python3 -B
# ----------------------------------------------------------------------------
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
# ------------------------------------------------------------------
try:
    import xml.etree.cElementTree as E
except:
    import xml.etree.ElementTree as E
# yrt
import os, sys, types, re, math
# ------------------------------------------------------------------
python3 = sys.version_info.major > 2
if python3:
    tupletype = tuple
    listtype = list
    max_int = sys.maxsize
else:
    tupletype = types.TupleType
    listtype = types.ListType
    max_int = sys.maxint
# ------------------------------------------------------------------
from print_method_names import *

def DBGPRT(s, method=None):
    print(s, file=sys.stderr, flush=True)
    if method is not None:
        print_methods(method)
    # fi
# End of DBGPRT
# ------------------------------------------------------------------

dynamics_map = {                                    # for direction/direction-type/dynamics/
    'p':    ' vol(p) ',
    'pp':   ' vol(pp) ',
    'ppp':  ' vol(ppp) ',
    'pppp': ' vol(pppp) ',
    'f':    ' vol(f) ',
    'ff':   ' vol(ff) ',
    'fff':  ' vol(fff) ',
    'ffff': ' vol(ffff) ',
    'mp':   ' vol(mp) ',
    'mf':   ' vol(mf) ',
    'sfz':  ' vol(sfz) ',
}

percSvg = '''%%beginsvg
    <defs>
    <text id="x" x="-3" y="0">&#xe263;</text>
    <text id="x-" x="-3" y="0">&#xe263;</text>
    <text id="x+" x="-3" y="0">&#xe263;</text>
    <text id="normal" x="-3.7" y="0">&#xe0a3;</text>
    <text id="normal-" x="-3.7" y="0">&#xe0a3;</text>
    <text id="normal+" x="-3.7" y="0">&#xe0a4;</text>
    <g id="circle-x"><text x="-3" y="0">&#xe263;</text><circle r="4" class="stroke"></circle></g>
    <g id="circle-x-"><text x="-3" y="0">&#xe263;</text><circle r="4" class="stroke"></circle></g>
    <path id="triangle" d="m-4 -3.2l4 6.4 4 -6.4z" class="stroke" style="stroke-width:1.4"></path>
    <path id="triangle-" d="m-4 -3.2l4 6.4 4 -6.4z" class="stroke" style="stroke-width:1.4"></path>
    <path id="triangle+" d="m-4 -3.2l4 6.4 4 -6.4z" class="stroke" style="fill:#000"></path>
    <path id="square" d="m-3.5 3l0 -6.2 7.2 0 0 6.2z" class="stroke" style="stroke-width:1.4"></path>
    <path id="square-" d="m-3.5 3l0 -6.2 7.2 0 0 6.2z" class="stroke" style="stroke-width:1.4"></path>
    <path id="square+" d="m-3.5 3l0 -6.2 7.2 0 0 6.2z" class="stroke" style="fill:#000"></path>
    <path id="diamond" d="m0 -3l4.2 3.2 -4.2 3.2 -4.2 -3.2z" class="stroke" style="stroke-width:1.4"></path>
    <path id="diamond-" d="m0 -3l4.2 3.2 -4.2 3.2 -4.2 -3.2z" class="stroke" style="stroke-width:1.4"></path>
    <path id="diamond+" d="m0 -3l4.2 3.2 -4.2 3.2 -4.2 -3.2z" class="stroke" style="fill:#000"></path>
    </defs>
    %%endsvg'''

tabSvg = '''%%beginsvg
    <style type="text/css">
    .bf {font-family:sans-serif; font-size:7px}
    </style>
    <defs>
    <rect id="clr" x="-3" y="-1" width="6" height="5" fill="white"></rect>
    <rect id="clr2" x="-3" y="-1" width="11" height="5" fill="white"></rect>'''

kopSvg = '<g id="kop%s" class="bf"><use xlink:href="#clr"></use><text x="-2" y="3">%s</text></g>\n'
kopSvg2 = '<g id="kop%s" class="bf"><use xlink:href="#clr2"></use><text x="-2" y="3">%s</text></g>\n'

# ------------------------------------------------------------------
info_list = []                                  # diagnostic messages
def info(s, warn=1):
    x = ('-- ' if warn else '') + s + '\n'
    info_list.append(x)
    if __name__ == '__main__':                  # only when run from the command line
        sys.stderr.write(x)
    # fi
# End of info

# ------------------------------------------------------------------
class Measure:
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def __init__(s, p):
        s.reset()
        s.ixp = p                               # part number  
        s.ixm = 0                               # measure number
        s.mdur = 0                              # measure duration(nominal metre value in divisions)
        s.divs = 0                              # number of divisions per 1/4
        s.mtr = 4,4                             # meter
    # End of __init__
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def reset(s):                               # reset each measure
        s.attr = ''                             # measure signatures, tempo
        s.lline = ''                            # left barline, but only holds ':' at start of repeat, otherwise empty
#--         s.rline = '|'                           # right barline
#--         s.rline = '\n'                           # right barline
        s.rline = ''                            # right barline
        s.lnum = ''                             # (left) volta number
    # End of reset
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# End of class Measure

# ------------------------------------------------------------------
class Note:
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def __init__(s, dur=0, n=None):
        s.tijd = 0                              # the time in XML division units
        s.dur = dur                             # duration of a note in XML divisions
        s.fact = None                           # time modification for tuplet notes(num, div)
        s.tup = ['']                            # start(s) and/or stop(s) of tuplet
        s.tupgcs = ''                           # tuplet string to issue before note
        s.beam = 0                              # 1 = beamed
        s.grace = 0                             # 1 = grace note
        s.before = []                           # gcs string that goes before the note/chord
        s.after = ''                            # the same after the note/chord
        s.ns = n and[n] or[]                    # notes in the chord
        s.tab = None                            # (string number, fret number)
        s.ntdec = ''                            # !string!, !courtesy!
    # End of __init__
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# End of class Note

# ------------------------------------------------------------------
class Elem:
    def __init__(s, string):
        s.tijd = 0                              # the time in XML division units
        s.str = string                          # any gcs string that is not a note
    # End of __init__
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# End of class Elem

# ------------------------------------------------------------------
class Counter:
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def inc(s, key, voice):
        s.counters[key][voice] = s.counters[key].get(voice, 0) + 1
    # End of inc
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def clear(s, vnums):                        # reset all counters
        tups = list(zip(vnums.keys(), len(vnums) * [0]))
        s.counters = {'note': dict(tups), 'nopr': dict(tups), 'nopt': dict(tups)}
    # End of clear
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def getv(s, key, voice):
        return s.counters[key][voice]
    # End of getv
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def prcnt(s, ip):                           # print summary of all non zero counters
        for iv in s.counters['note']:
            if s.getv('nopr', iv) != 0:
                info(f"part {ip}, voice {iv} has {s.getv('nopr', iv)} skipped non printable notes")
            # fi
            if s.getv('nopt', iv) != 0:
                info(f"part {ip}, voice {iv} has {s.getv('nopt', iv)} notes without pitch")
            # fi
            if s.getv('note', iv) == 0:         #o real notes counted in this voice
                info(f'part {ip}, skipped empty voice {iv}')
            # fi
        # rof
    # End of prcnt
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# End of class Counter

# ------------------------------------------------------------------
class Music:
    def __init__(s):
        s.tijd = 0                              # the current time
        s.maxtime = 0                           # maximum time in a measure
        s.gMaten = []                           # [voices,.. for all measures in a part]
        s.vnums = {}                            # all used voice id's in a part(xml voice id's == numbers)
        s.cnt = Counter()                       # global counter object
        s.vceCnt = 1                            # the global voice count over all parts
        s.lastnote = None                       # the last real note record inserted in s.voices
        s.repbra = 0                            # true if volta is used somewhere
    # End of __init__
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def initVoices(s, newPart=0):
        s.vtimes, s.voices = {}, {}
        for v in s.vnums:
            s.vtimes[v] = 0                     # {voice: the end time of the last item in each voice}
            s.voices[v] = []                    # {voice: [Note|Elem, ..]}
        # rof
        if newPart:
            s.cnt.clear(s.vnums)                # clear counters once per part
        # fi
    # End of initVoices
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def incTime(s, dt):
        s.tijd += dt
        if s.tijd < 0:
            s.tijd = 0                          # erroneous <backup> element
        # fi
        if s.tijd > s.maxtime:
            s.maxtime = s.tijd
        # fi
    # End of incTime
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def appendElemCv(s, voices, elem):
        for v in voices:
            s.appendElem(v, elem)               # insert element in all voices
        # rof
    # End of appendElem
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def insertElem(s, v, elem):                 # insert at the start of voice v in the current measure
        obj = Elem(elem)
        obj.tijd = 0                            # because voice is sorted later
        s.voices[v].insert(0, obj)
    # End of insertElem
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def appendObj(s, v, obj, dur):
        obj.tijd = s.tijd
        s.voices[v].append(obj)
        s.incTime(dur)
        if s.tijd > s.vtimes[v]:
            s.vtimes[v] = s.tijd                # don't update for inserted earlier items
        # fi
    # End of appendObj
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def appendElem(s, v, elem, tel=0):
        s.appendObj(v, Elem(elem), 0)
        if tel:
            s.cnt.inc('note', v)                # count number of certain elements in each voice(in addition to notes)
        # fi
    # End of appendElem
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def appendElemT(s, v, elem, tijd):          # insert element at specified time
        obj = Elem(elem)
        obj.tijd = tijd
        s.voices[v].append(obj)
    # End of appendElemT
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def appendNote(s, v, note, noot):
        note.ns.append(note.ntdec + noot)
        s.appendObj(v, note, int(note.dur))
        s.lastnote = note                       # remember last note/rest for later modifications(chord, grace)
        s.cnt.inc('note', v)                # count number of real notes in each voice
    # End of appendNote
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def getLastRec(s, voice):
        if s.gMaten:
            return s.gMaten[-1][voice][-1]      # the last record in the last measure
        # fi
        return None                             # no previous records in the first measure
    # End of getLastRec
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def addChord(s, note, noot):                # careful: we assume that chord notes follow immediately
        for d in note.before:                   # put all decorations before chord
            if d not in s.lastnote.before:
                s.lastnote.before += [d]
            # fi
        # rof
        s.lastnote.ns.append(note.ntdec + noot)
    # End of addChord
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def addBar(s, lbrk, m):                     # linebreak, measure data
        if m.mdur and s.maxtime > m.mdur:
            info(f'measure {m.ixm+1} in part {m.ixp+1} longer than metre')
        # fi
        s.tijd = s.maxtime                      # the time of the bar lines inserted here
        for v in s.vnums:
            if m.lline or m.lnum:               # if left barline or left volta number
                p = s.getLastRec(v)             # get the previous barline record
                if p:                           # in measure 1 no previous measure is available
                    x = p.str                   # p.str is the barline string
                    if m.lline:                 # append begin of repeat, m.lline == ':'
#--                         DBGPRT(f'm.lline={m.lline}')
                        x = (x + m.lline).replace(':|:','::').replace('||','|')
                    elif m.lnum:                # new behaviour with I:repbra 0
#--                         DBGPRT(f'm.lnum={m.lnum}')
                        x += m.lnum             # add volta number(s) or text to all voices
                        s.repbra = 1            # signal occurrence of a volta
                    # fi
                    p.str = x                   # modify previous right barline
                elif m.lline:                   # begin of new part and left repeat bar is required
#--                     DBGPRT(f'beginning new part m.lline={m.lline}')
                    s.insertElem(v, '|:')
                # fi
            # fi
            if lbrk:
                p = s.getLastRec(v)             # get the previous barline record
                if p:
                    p.str += lbrk               # insert linebreak char after the barlines+volta
                # fi
            # fi
            if m.attr:                          # insert signatures at front of buffer
                s.insertElem(v, f'{m.attr}')
            # fi
            s.appendElem(v, f' {m.rline}')    # insert current barline record at time maxtime
            s.voices[v] = sortMeasure(s.voices[v], m)  # make all times consistent
            mkBroken(s.voices[v])
        # rof
        s.gMaten.append(s.voices)
        s.tijd = s.maxtime = 0
        s.initVoices()
    # End of addBar
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def outVoices(s, divs, ip, isSib):          # output all voices of part ip
        vvmap = {}                              # xml voice number -> gcs voice number(one part)
        vnum_keys = list(s.vnums.keys())
        if isSib:
            vnum_keys.sort()
        # fi
        lvc = min(vnum_keys or[1])              # lowest xml voice number of this part
        for iv in vnum_keys:
            if s.cnt.getv('note', iv) == 0:     # no real notes counted in this voice
                continue                        # skip empty voices
            # fi
            vn = []                                     # for voice iv: collect all notes to vn.
            for im in range(len(s.gMaten)):
                measure = s.gMaten[im][iv]
                vn.append(outVoice(measure, divs[im], im, ip))
            # rof
            bn = 1
            while vn:                           # while still measures available
                chunk = vn[0]
                gcsOut.add(f'measure {bn}')     # line with barnumer
                chunk = re.sub(r'  +', ' ', chunk)    # Get rid of doubled spaces.
                stf = f'V{s.vceCnt}'
                gcsOut.add(f'{stf}: {chunk}')
                del vn[0]                       # chop first measure
                bn += 1
            # elihw

            vvmap[iv] = s.vceCnt                # xml voice number -> gcs voice number
            s.vceCnt += 1                       # count voices over all parts
        # rof
        s.gMaten = []                           # reset the follwing instance vars for each part
        s.cnt.prcnt(ip+1)                       # print summary of skipped items in this part
        return vvmap
    # End of outVoices
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# End of class Music

# ------------------------------------------------------------------
global instruments
# select chan sfont bank prog -      prog, bank.
instruments = {
    'Piano'.lower() : [0, 0],
    'Acoustic Grand Piano'.lower() : [0, 0],
    'Stereo Grand'.lower() : [0, 0],
    'Bright Acoustic Piano'.lower() : [1, 0],
    'Bright Grand'.lower() : [1, 0],
    'Electric Grand Piano'.lower() : [2, 0],
    'Electric Grand'.lower() : [2, 0],
    'Honky-tonk Piano'.lower() : [3, 0],
    'Honky-tonk'.lower() : [3, 0],
    'Electric Piano 1'.lower() : [4, 0],
    'Time Electric Piano 1'.lower() : [4, 0],
    'Electric Piano 2'.lower() : [5, 0],
    'FM Electric Piano'.lower() : [5, 0],
    'Harpsichord'.lower() : [6, 0],
    'Clavi'.lower() : [7, 0],
    'Clavinet'.lower() : [7, 0],
    'Celesta'.lower() : [8, 0],
    'Glockenspiel'.lower() : [9, 0],
    'Music Box'.lower() : [10, 0],
    'Vibraphone'.lower() : [11, 0],
    'Marimba'.lower() : [12, 0],
    'Xylophone'.lower() : [13, 0],
    'Tubular Bells'.lower() : [14, 0],
    'Dulcimer'.lower() : [15, 0],
    'Drawbar Organ'.lower() : [16, 0],
    'Tonewheel Organ'.lower() : [16, 0],
    'Percussive Organ'.lower() : [17, 0],
    'Rock Organ'.lower() : [18, 0],
    'Church Organ'.lower() : [19, 0],
    'Pipe Organ'.lower() : [19, 0],
    'Reed Organ'.lower() : [20, 0],
    'Accordion'.lower() : [21, 0],
    'Harmonica'.lower() : [22, 0],
    'Tango Accordion'.lower() : [23, 0],
    'Bandoneon'.lower() : [23, 0],
    'Acoustic Guitar (nylon)'.lower() : [24, 0],
    'Nylon Guitar'.lower() : [24, 0],
    'Acoustic Guitar (steel)'.lower() : [25, 0],
    'Steel Guitar'.lower() : [25, 0],
    'Electric Guitar (jazz)'.lower() : [26, 0],
    'Jazz Guitar'.lower() : [26, 0],
    'Electric Guitar (clean)'.lower() : [27, 0],
    'Clean Guitar'.lower() : [27, 0],
    'Electric Guitar (muted)'.lower() : [28, 0],
    'Muted Guitar'.lower() : [28, 0],
    'Overdriven Guitar'.lower() : [29, 0],
    'Overdrive Guitar'.lower() : [29, 0],
    'Distortion Guitar'.lower() : [30, 0],
    'Guitar Harmonics'.lower() : [31, 0],
    'Acoustic Bass'.lower() : [32, 0],
    'Electric Bass (finger)'.lower() : [33, 0],
    'Finger Bass'.lower() : [33, 0],
    'Electric Bass (pick)'.lower() : [34, 0],
    'Pick Bass'.lower() : [34, 0],
    'Fretless Bass'.lower() : [35, 0],
    'Slap Bass 1'.lower() : [36, 0],
    'Slap Bass 2'.lower() : [37, 0],
    'Synth Bass 1'.lower() : [38, 0],
    'Synth Bass 2'.lower() : [39, 0],
    'Violin'.lower() : [40, 0],
    'Viola'.lower() : [41, 0],
    'Cello'.lower() : [42, 0],
    'Contrabass'.lower() : [43, 0],
    'Double Bass'.lower() : [43, 0],
    'Tremolo Strings'.lower() : [44, 0],
    'Stero Strings Trem'.lower() : [44, 0],
    'Pizzicato Strings'.lower() : [45, 0],
    'Orchestral Harp'.lower() : [46, 0],
    'Timpani'.lower() : [47, 0],
    'String Ensemble 1'.lower() : [48, 0],
    'Stereo Strings Fast'.lower() : [48, 0],
    'String Ensemble 2'.lower() : [49, 0],
    'Stereo Strings Slow'.lower() : [49, 0],
    'Synth Strings 1'.lower() : [50, 0],
    'Synth Strings 2'.lower() : [51, 0],
    'Choir Aahs'.lower() : [52, 0],
    'Concert Choir'.lower() : [52, 0],
    'Voice Oohs'.lower() : [53, 0],              # NOT RIGHT!
    'Synth Voice'.lower() : [54, 0],             # NOT RIGHT!
    'Orchestra Hit'.lower() : [55, 0],           # ?? strange sound. ??
    'Trumpet'.lower() : [56, 0],
    'Trombone'.lower() : [57, 0],
    'Tuba'.lower() : [58, 0],
    'Muted Trumpet'.lower() : [59, 0],
    'French Horn'.lower() : [60, 0],
    'French Horns'.lower() : [60, 0],
    'Brass Section'.lower() : [61, 0],
    'Synth Brass 1'.lower() : [62, 0],
    'Synth Brass 2'.lower() : [63, 0],
    'Soprano Sax'.lower() : [64, 0],
    'Alto Sax'.lower() : [65, 0],
    'Tenor Sax'.lower() : [66, 0],
    'Baritone Sax'.lower() : [67, 0],
    'Oboe'.lower() : [68, 0],
    'English Horn'.lower() : [69, 0],
    'Bassoon'.lower() : [70, 0],
    'Clarinet'.lower() : [71, 0],
    'Piccolo'.lower() : [72, 0],
    'Flute'.lower() : [73, 0],
    'Recorder'.lower() : [74, 0],
    'Pan Flute'.lower() : [75, 0],
    'Blown bottle'.lower() : [76, 0],
    'Bottle Blow'.lower() : [76, 0],
    'Shakuhachi'.lower() : [77, 0],
    'Whistle'.lower() : [78, 0],
    'Irish Tin Whistle'.lower() : [78, 0],
    'Ocarina'.lower() : [79, 0],
    'Lead 1 (square)'.lower() : [80, 0],
    'Square Lead'.lower() : [80, 0],
    'Lead 2 (sawtooth)'.lower() : [81, 0],
    'Saw Lead'.lower() : [81, 0],
    'Lead 3 (calliope)'.lower() : [82, 0],
    'Synth Calliope'.lower() : [82, 0],
    'Lead 4 (chiff)'.lower() : [83, 0],
    'Chiffer Lead'.lower() : [83, 0],
    'Lead 5 (charang)'.lower() : [84, 0],
    'Charang'.lower() : [84, 0],
    'Lead 6 (voice)'.lower() : [85, 0],
    'Solo Vox'.lower() : [85, 0],
    'Lead 7 (fifths)'.lower() : [86, 0],
    '5th Saw Wave'.lower() : [86, 0],
    'Lead 8 (bass + lead)'.lower() : [87, 0],
    'Bass & lead'.lower() : [87, 0],
    'Pad 1 (new age)'.lower() : [88, 0],
    'Fantasia'.lower() : [88, 0],
    'Pad 2 (warm)'.lower() : [89, 0],
    'Warm Pad'.lower() : [89, 0],
    'Pad 3 (polysynth)'.lower() : [90, 0],
    'Polysynth'.lower() : [90, 0],
    'Pad 4 (choir)'.lower() : [91, 0],
    'Space Voice'.lower() : [91, 0],
    'Pad 5 (bowed)'.lower() : [92, 0],
    'Bowed Glass'.lower() : [92, 0],
    'Pad 6 (metallic)'.lower() : [93, 0],
    'Metal Pad'.lower() : [93, 0],
    'Pad 7 (halo)'.lower() : [94, 0],
    'Halo Pad'.lower() : [94, 0],
    'Pad 8 (sweep)'.lower() : [95, 0],
    'Sweep Pad'.lower() : [95, 0],
    'FX 1 (rain)'.lower() : [96, 0],
    'Ice Rain'.lower() : [96, 0],
    'FX 2 (soundtrack)'.lower() : [97, 0],
    'Soundtrack'.lower() : [97, 0],
    'FX 3 (crystal)'.lower() : [98, 0],
    'Crystal'.lower() : [98, 0],
    'FX 4 (atmosphere)'.lower() : [99, 0],
    'Atmosphere'.lower() : [99, 0],
    'FX 5 (brightness)'.lower() : [100, 0],
    'Brightness'.lower() : [100, 0],
    'FX 6 (goblins)'.lower() : [101, 0],
    'Goblins'.lower() : [101, 0],
    'FX 7 (echoes)'.lower() : [102, 0],
    'Echo Drops'.lower() : [102, 0],
    'FX 8 (sci-fi)'.lower() : [103, 0],
    'Star Theme'.lower() : [103, 0],
    'Sitar'.lower() : [104, 0],
    'Banjo'.lower() : [105, 0],
    'Shamisen'.lower() : [106, 0],
    'Koto'.lower() : [107, 0],
    'Kalimba'.lower() : [108, 0],
    'Bag pipe'.lower() : [109, 0],
    'Bagpipes'.lower() : [109, 0],
    'Fiddle'.lower() : [110, 0],
    'Shanai'.lower() : [111, 0],
    'Shenai'.lower() : [111, 0],
    'Tinkle Bell'.lower() : [112, 0],
    'Tinker Bell'.lower() : [112, 0],
    'Agogo'.lower() : [113, 0],
    'Steel Drums'.lower() : [114, 0],
    'Woodblock'.lower() : [115, 0],
    'Wood Block'.lower() : [115, 0],
    'Taiko Drum'.lower() : [116, 0],
    'Melodic Tom'.lower() : [117, 0],
    'Synth Drum'.lower() : [118, 0],
    'Reverse Cymbal'.lower() : [119, 0],
    'Guitar Fret Noise'.lower() : [120, 0],
    'Fret Noise'.lower() : [120, 0],
    'Breath Noise'.lower() : [121, 0],
    'Seashore'.lower() : [122, 0],
    'Bird Tweet'.lower() : [123, 0],
    'Birds'.lower() : [123, 0],
    'Telephone Ring'.lower() : [124, 0],
    'Telephone 1'.lower() : [124, 0],
    'Helicopter'.lower() : [125, 0],
    'Applause'.lower() : [126, 0],
    'Gunshot'.lower() : [127, 0],
    'Gun Shot'.lower() : [127, 0],
    'Synth Bass 101'.lower() : [38, 1],
    'Mono Strings Trem'.lower() : [44, 1],
    'Mono Strings Fast'.lower() : [48, 1],
    'Mono Strings Slow'.lower() : [49, 1],
    'Concert Choir Mono'.lower() : [52, 1],
    'Trumpet 2'.lower() : [56, 1],
    'Trombone 2'.lower() : [57, 1],
    'Muted Trumpet 2'.lower() : [59, 1],
    'Solo French Horn'.lower() : [60, 1],
    'Brass Section Mono'.lower() : [61, 1],
    'Square Wave'.lower() : [80, 1],
    'Saw Wave'.lower() : [81, 1],
    'Synth Mallet'.lower() : [98, 1],
    'Cut Noise'.lower() : [120, 1],
    'Fl. Key Click'.lower() : [121, 1],
    'Rain'.lower() : [122, 1],
    'Dog'.lower() : [123, 1],
    'Telephone 2'.lower() : [124, 1],
    'Car-Engine'.lower() : [125, 1],
    'Laughing'.lower() : [126, 1],
    'Machine Gun'.lower() : [127, 1],
    'Echo Pan'.lower() : [102, 2],
    'String Slap'.lower() : [120, 2],
    'Thunder'.lower() : [122, 2],
    'Horse Gallop'.lower() : [123, 2],
    'Door Creaking'.lower() : [124, 2],
    'Car-Stop'.lower() : [125, 2],
    'Scream'.lower() : [126, 2],
    'Lasergun'.lower() : [127, 2],
    'Howling Winds'.lower() : [122, 3],
    'Bird 2'.lower() : [123, 3],
    'Door'.lower() : [124, 3],
    'Car-Pass'.lower() : [125, 3],
    'Punch'.lower() : [126, 3],
    'Explosion'.lower() : [127, 3],
    'Stream'.lower() : [122, 4],
    'Scratch'.lower() : [123, 4],
    'Car-Crash'.lower() : [125, 4],
    'Heart Beat'.lower() : [126, 4],
    'Bubbles'.lower() : [122, 5],
    'Windchime'.lower() : [124, 5],
    'Siren'.lower() : [125, 5],
    'Footsteps'.lower() : [126, 5],
    'Train'.lower() : [125, 6],
    'Jet Plane'.lower() : [125, 7],
    'Chorused Tine EP'.lower() : [4, 8],
    'Chorused FM EP'.lower() : [5, 8],
    'Coupled Harpsichord'.lower() : [6, 8],
    'Church Bells'.lower() : [14, 8],
    'Detuned Tnwl. Organ'.lower() : [16, 8],
    'Detuned Perc. Organ'.lower() : [17, 8],
    'Pipe Organ 2'.lower() : [19, 8],
    'Italian Accordian'.lower() : [21, 8],
    'Ukulele'.lower() : [24, 8],
    '12-String Guitar'.lower() : [25, 8],
    'Hawaiian Guitar'.lower() : [26, 8],
    'Chorused Clean Gt.'.lower() : [27, 8],
    'Funk Guitar'.lower() : [28, 8],
    'Feedback Guitar'.lower() : [30, 8],
    'Guitar Feedback'.lower() : [31, 8],
    'Synth Bass 3'.lower() : [38, 8],
    'Synth Bass 4'.lower() : [39, 8],
    'Orchestra Pad'.lower() : [48, 8],
    'Synth Strings 3'.lower() : [50, 8],
    'Brass Section 2'.lower() : [61, 8],
    'Synth Brass 3'.lower() : [62, 8],
    'Synth Brass 4'.lower() : [63, 8],
    'Sine Wave'.lower() : [80, 8],
    'Doctor Solo'.lower() : [81, 8],
    'Taisho Koto'.lower() : [107, 8],
    'Castanets'.lower() : [115, 8],
    'Concert Bass Drum'.lower() : [116, 8],
    'Melodic Tom 2'.lower() : [117, 8],
    '808 Tom'.lower() : [118, 8],
    'Starship'.lower() : [125, 8],
    'Carillon'.lower() : [14, 9],
    'Burst Noise'.lower() : [125, 9],
    'Piano & Str.-Fade'.lower() : [0, 11],
    'Piano & Str.-Sus'.lower() : [1, 11],
    'Tine & FM EPs'.lower() : [4, 11],
    'Piano & FM EP'.lower() : [5, 11],
    'Tinkling Bells'.lower() : [8, 11],
    'Bell Tower'.lower() : [14, 11],
    'Techno Bass'.lower() : [38, 11],
    'Pulse Bass'.lower() : [39, 11],
    'Stereo Strings Velo'.lower() : [49, 11],
    'Synth Strings 4'.lower() : [50, 11],
    'Synth Strings 5'.lower() : [51, 11],
    'Brass Section 3'.lower() : [61, 11],
    'Whistlin'.lower() : [78, 11],
    'Sawtooth Stab'.lower() : [81, 11],
    "Doctor's Solo".lower() : [87, 11],
    'Harpsi Pad'.lower() : [88, 11],
    'Solar Wind'.lower() : [89, 11],
    'Mystery Pad'.lower() : [96, 11],
    'Synth Chime'.lower() : [98, 11],
    'Bright Saw Stack'.lower() : [100, 11],
    'Cymbal Crash'.lower() : [119, 11],
    'Filter Snap'.lower() : [121, 11],
    'Interference'.lower() : [127, 11],
    'Bell Piano'.lower() : [0, 12],
    'Bell Tine EP'.lower() : [4, 12],
    'Christmas Bells'.lower() : [10, 12],
    'Clean Guitar 2'.lower() : [27, 12],
    'Mean Saw Bass'.lower() : [38, 12],
    'Full Orchestra'.lower() : [48, 12],
    'Mono Strings Velo'.lower() : [49, 12],
    'Square Lead 2'.lower() : [80, 12],
    'Saw Lead 2'.lower() : [81, 12],
    'Fantasia 2'.lower() : [88, 12],
    'Solar Wind 2'.lower() : [89, 12],
    'White Noise Wave'.lower() : [122, 12],
    'Shooting Star'.lower() : [127, 12],
    'Woodwind Choir'.lower() : [48, 13],
    'Square Lead 3'.lower() : [80, 13],
    'Saw Lead 3'.lower() : [81, 13],
    'Night Vision'.lower() : [88, 13],
    'Mandolin'.lower() : [25, 16],
    'Standard Drums'.lower() : [0, 120],
    'Standard 2 Drums'.lower() : [1, 120],
    'Room Drums'.lower() : [8, 120],
    'Power Drums'.lower() : [16, 120],
    'Electronic Drums'.lower() : [24, 120],
    '808/909 Drums'.lower() : [25, 120],
    'Dance Drums'.lower() : [26, 120],
    'Jazz Drums'.lower() : [32, 120],
    'Brush Drums'.lower() : [40, 120],
    'Orchestral Perc.'.lower() : [48, 120],
    'SFX Kit'.lower() : [56, 120],
    'Standard'.lower() : [0, 128],
    'Standard 2'.lower() : [1, 128],
    'Room'.lower() : [8, 128],
    'Power'.lower() : [16, 128],
    'Electronic'.lower() : [24, 128],
    '808/909'.lower() : [25, 128],
    'Dance'.lower() : [26, 128],
    'Jazz'.lower() : [32, 128],
    'Brush'.lower() : [40, 128],
    'Orchestral'.lower() : [48, 128],
    'SFX'.lower() : [56, 128],
}

# ------------------------------------------------------------------------------
global drum_sounds
drum_sounds = {
    'Acoustic Bass Drum'.lower(): 35,
    'Bass Drum 1'.lower(): 36,
    'Side Stick'.lower(): 37,
    'Acoustic Snare'.lower(): 38,
    'Hand Clap'.lower(): 39,
    'Electric Snare'.lower(): 40,
    'Low Floor Tom'.lower(): 41,
    'Closed Hi Hat'.lower(): 42,
    'High Floor Tom'.lower(): 43,
    'Pedal Hi-Hat'.lower(): 44,
    'Low Tom'.lower(): 45,
    'Open Hi-Hat'.lower(): 46,
    'Low-Mid Tom'.lower(): 47,
    'Hi Mid Tom'.lower(): 48,
    'Crash Cymbal 1'.lower(): 49,
    'High Tom'.lower(): 50,
    'Ride Cymbal 1'.lower(): 51,
    'Chinese Cymbal'.lower(): 52,
    'Ride Bell'.lower(): 53,
    'Tambourine'.lower(): 54,
    'Splash Cymbal'.lower(): 55,
    'Cowbell'.lower(): 56,
    'Crash Cymbal 2'.lower(): 57,
    'Vibraslap'.lower(): 58,
    'Ride Cymbal 2'.lower(): 59,
    'Hi Bongo'.lower(): 60,
    'Low Bongo'.lower(): 61,
    'Mute Hi Conga'.lower(): 62,
    'Open Hi Conga'.lower(): 63,
    'Low Conga'.lower(): 64,
    'High Timbale'.lower(): 65,
    'Low Timbale'.lower(): 66,
    'High Agogo'.lower(): 67,
    'Low Agogo'.lower(): 68,
    'Cabasa'.lower(): 69,
    'Maracas'.lower(): 70,
    'Short Whistle'.lower(): 71,
    'Long Whistle'.lower(): 72,
    'Short Guiro'.lower(): 73,
    'Long Guiro'.lower(): 74,
    'Claves'.lower(): 75,
    'Hi Wood Block'.lower(): 76,
    'Low Wood Block'.lower(): 77,
    'Mute Cuica'.lower(): 78,
    'Open Cuica'.lower(): 79,
    'Mute Triangle'.lower(): 80,
    'Open Triangle'.lower(): 81,
}

# ----------------------------------------------------------------------------
# ------------------------------------------------------------------
class GCSoutput:
    pagekeys = 'scale,pageheight,pagewidth,leftmargin,rightmargin,topmargin,botmargin'.split(',')
    vn_to_staffname = {}
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def __init__(s, fnmext):
        s.fnmext = fnmext
        s.outlist = []                          # list of gcs strings
        s.title = 'title   Title'
        s.key = 'none'
        s.clefs = {}                            # clefs for all gcs-voices
        s.mtr = 'none'
        s.tempo = 0                             # 0 -> no tempo field
        s.tempo_units = 4                       # note type of tempo direction
        s.volpan = 2                            # 0 -> no %%MIDI, 1 -> only program, 2 -> all %%MIDI
        s.outfile = sys.stdout
        s.pageFmt = {}
        for k in s.pagekeys:
            s.pageFmt[k] = None
        # rof
    # End of __init__
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def add(s, str):
        s.outlist.append(str + '\n')            # collect all GCS output
    # End of add
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def mkHeader(s, stfmap, partlist, midimap, vmpdct, koppen): # stfmap = [parts], part = [staves], stave = [voices]
        accVce, accStf, staffs = [], [], stfmap[:]  # staffs is consumed
        for x in partlist:                      # collect partnames into accVce and staff groups into accStf
            try:
                prgroupelem(x, ('', ''), '', stfmap, accVce, accStf)
            except:
                info('lousy musicxml: error in part-list')
            # yrt
        # rof
        staves = ' '.join(accStf)
        clfnms = {}
        for part, (partname, partabbrv) in zip(staffs, accVce):
            if not part:
                continue                        # skip empty part
            # fi
            firstVoice = part[0][0]             # the first voice number in this part
            nm  = partname.replace('\n','\\n').replace('.:','.').strip(':')
            snm = partabbrv.replace('\n','\\n').replace('.:','.').strip(':')
            if not snm:
                snm = ''
            # fi
            nothing = ''
            if not nm:
                staffnm = ''
                nm = ''
            else:
#--                pattern = r'[0-9 .+_-]'
                pattern = r'[^A-Za-z0-9]'
                # Match all digits in the string and replace them with an empty string.
                staffnm = re.sub(pattern, '', nm)
            # fi
            if staffnm == '':
                staffnm = f'V{firstVoice}'
            # fi
            s.vn_to_staffname[firstVoice] = staffnm
            clfnms[firstVoice] = f'    $$ {nm} - {snm}'
            DBGPRT(f'mkHeader #A firstVoice={firstVoice} nm="{nm}" snm="{snm}" clfnms[firstVoice]="{clfnms[firstVoice]}')
        # rof
        hd = [f'{s.title}\n']
        for i, k in enumerate(s.pagekeys):
            if s.pageFmt[k] != None:
                xyz = i > 0 and 'cm' or ''
                hd.append(f'%%{k} {s.pageFmt[k]:.2f}{xyz}\n')
            # fi
        # rof
        if staves and len(accStf) > 1:
            hd.append('%%score ' + staves + '\n')
        # fi
        if s.tempo:
            tempo = f'tempo {s.tempo},{s.tempo_units}\n'
        else:
            tempo = ''                          # default to no tempo field
        # fi
        hd.append(f'{tempo}')
        hd.append(f'meter {s.mtr}\n')
        hd.append(f'key {s.key}\n')
        vxs = sorted(vmpdct.keys())
        for vx in vxs:
            hd.extend(vmpdct[vx])
        # rof
        for vnum, clef in s.clefs.items():
            ch, prg, vol, pan = midimap[vnum-1][:4]
            dmap = midimap[vnum - 1][4:]        # map of percussion notes to midi notes
            if dmap and 'perc' not in clef:
                clef = (clef + ' map=perc').strip();
            # fi
#--             hd.append(f"V:{vnum} {clef} {clfnms.get(vnum, '')}\n")
#--             hd.append(f"{vn_to_staffname[vnum]}: {clef} {clfnms.get(vnum, '')}\n")
#--             hd.append(f"V:{vnum} {clef} {clfnms.get(vnum, '')}\n")
#--             hd.append(f"V:{vnum} {clef} {clfnms.get(vnum, '')}\n")
            cmt = clfnms.get(vnum, '')
            if vnum not in s.vn_to_staffname:
                s.vn_to_staffname[vnum] = f'V{vnum}'
            # fi
            hd.append(f'* ............................................................................\n')
            if cmt != '':
                hd.append(f'staff {s.vn_to_staffname[vnum]}    {cmt}\n')
            else:
                hd.append(f'staff {s.vn_to_staffname[vnum]}\n')
            # fi
            c = clef.split('\n')
            for a in c:
                newc = re.sub(r'%%', f' {s.vn_to_staffname[vnum]} ', a)
                hd.append(newc + '\n')
            # rof

            if vnum in vmpdct:
                hd.append(f'%%voicemap tab{vnum}\n')
                hd.append('key none\n')
                hd.append('meter 4/4\n')
#--                 hd.append('%%clef none\n')
                hd.append(f'clef {s.vn_to_staffname[vnum]} none\n')
                hd.append('%%staffscale 1.6\n')
                hd.append('%%flatbeams true\n')
                hd.append('%%stemdir down\n')
            # fi
            if 'perc' in clef:
                hd.append('key none\n');          # no key for a perc voice
            # fi
# instrument 1,2 flute            $$ voice, name or number.
#--             if ch > 0 and ch != vnum:
#--                 hd.append(f'%%MIDI channel {ch}\n')
#--             # fi
#--             if prg > 0:
#--                 hd.append(f'%%MIDI program {prg - 1}\n')
#--             # fi
            if prg > 0:
                inst = None
                instval = [prg - 1, 0]      # Only first bank attempt.
                for key, value in instruments.items():
                    if instval == value:
                        inst = key
                        break
                    # fi
                # fi
                if inst is None:            # Value not in instrument list - HOW?
                    inst = prg - 1
                # fi
                hd.append(f'instrument {s.vn_to_staffname[vnum]} {inst}\n')
            # fi
            if vol >= 0:
#--                 hd.append(f'%%MIDI control 7 {vol:.0f}\n')  # volume == 0 is possible ...
                hd.append(f'intensity {s.vn_to_staffname[vnum]} {round(vol)}\n')  # volume == 0 is possible ...
            # fi
            if pan >= 0:
#--                 hd.append(f'%%MIDI control 10 {pan:.0f}\n')
                hd.append(f'pan {s.vn_to_staffname[vnum]} {round(pan)}\n')
            # fi
            for gcsNote, step, midiNote, notehead in dmap:
                if not notehead:
                    notehead = 'normal'
                # fi
                if gcsMid(gcsNote) != midiNote or gcsNote != step:
                    hd.append(f'%%MIDI drummap {gcsNote} {midiNote}\n')
                    DBGPRT(f'I:percmap {gcsNote} {step} {midiNote} {notehead}\n')
                # fi
            # fi
        # rof
        them = '0 '                 # the last measure
        lastv = 'V0'                # the last voice
        # lines[m][v] = xxx;
        lines = {}
        lines[them] = {}
        lines[them][lastv] = ''
        vtos = {}
        # Break into lines per measure.
        for line in s.outlist:
            line = line.rstrip()
            if line[0:8] == 'measure ':
                them = line[8:]
                if them not in lines:
                    lines[them] = {}
                    lines[them][lastv] = ''
                # fi
                continue
            # fi
            x = line.split(r':', 1)       # Split at first colon
            if len(x) == 2 and x[0][0] == 'V':
                n = x[0][1:].strip()
                try:
                    n = int(n)
                except:
                    lines[them][lastv] += line + '\n'
                    continue
                # yrt
                if n in s.vn_to_staffname:
                    lastv = s.vn_to_staffname[n]
                else:
                    lastv = x[0]
                # fi
                if lastv not in lines[them]:
                    lines[them][lastv] = ''
                # fi
#--                 DBGPRT(f'lastv="{lastv}" x[1]={x[1]} len(x)={len(x)}')
                lines[them][lastv] += lastv + ': ' + x[1].strip() + '\n'
            else:
#--                 DBGPRT(f'line added to lastv="{lastv}" them={them} line={line}')
                lines[them][lastv] += line + '\n'
            # fi
        # rof
        for them in lines:
            if len(lines[them]) > 1:
                hd.append('* ----------------------------------------------------------------------\n')
                hd.append(f'measure {them}\n')
            # fi
            for lastv in lines[them]:
                newc = re.sub(r'%%', f' {lastv} ', lines[them][lastv])
                hd.append(newc)
            # rof
        # rof
        s.outlist = hd
        if koppen:                              # output SVG stuff needed for tablature
            k1 = kopSvg                         # shift note heads 3 units left
            k2 = kopSvg2
            tb = tabSvg
            ks = sorted(koppen.keys())          # javascript compatibility
            ks = [k2 % (k, k) if len(k) == 2 else k1 % (k, k) for k in ks]
            tbs = map(lambda x: x.strip() + '\n', tb.splitlines())   # javascript compatibility
            s.outlist = tbs + ks + ['</defs>\n%%endsvg\n'] + s.outlist
        # fi
        s.outlist += '* ----------------------------------------------------------------------\n'
        s.outlist += 'cstop*'
    # End of mkHeader
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def getGCS(s):
        str = ''.join(s.outlist)
        return str
    # End of getGCS
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def writeall(s):  # determine the required encoding of the entire GCS output
        str = s.getGCS()
        if python3:
            s.outfile.write(str)
        else:
            s.outfile.write(str.encode('utf-8'))
        # fi
        s.outfile.write('\n')               # add empty line between tunes on stdout
        info(f'{s.fnmext} written with {len(s.clefs)} voices', warn=0)
# End of class GCSoutput

# ----------------------------------------------------------------------------
def simplify(a, b):                             # divide a and b by their greatest common divisor
    x, y = a, b
    while b:
        a, b = b, a % b
    # elihw
    return x // a, y // a                       # Integer divide and truncate.
# End of simplify

# ----------------------------------------------------------------------------
legal_mc_notes = {
        0.015625 : '64',
       0.0234375 : '64d',
         0.03125 : '32',
        0.046875 : '32d',
          0.0625 : '16',
         0.09375 : '16d',
         0.10937 : '16dd',
           0.125 : '8',
          0.1875 : '8d',
         0.21875 : '8dd',
            0.25 : '4',
           0.375 : '4d',
         0.43750 : '4dd',
             0.5 : '2',
            0.75 : '2d',
         0.87500 : '2dd',
             1.0 : '1',
             1.5 : '1d',
            1.75 : '1dd',
}

def gcsdur(nx, divs):                           # convert an musicXML duration d to gcs units
    if nx.dur == 0:
        return ''                               # when called for elements without duration
    # fi
    num, den = simplify(nx.dur, divs * 4)
    if nx.fact:                                 # apply tuplet time modification
        numfac, denfac = nx.fact
        num, den = simplify(num * numfac, den * denfac)
    # fi
    if den > 64:                                # limit the denominator to a maximum of 64
        x = float(num) / den; n = math.floor(x);  # when just above an integer n
        if x - n < 0.1 * x:
            num, den = n, 1;                    # round to n
        # fi
        num64 = 64. * num / den + 1.0e-15       # to get Python2 behaviour of round
        num, den = simplify(int(round(num64)), 64)
    # fi
    flt = num / den
    if flt in legal_mc_notes:
        return legal_mc_notes[flt]
    # fi
    DBGPRT(f'gcsdur value flt={flt} not in {legal_mc_notes}')
    return UNKNOWN
# End of gcsdur

# ----------------------------------------------------------------------------
def gcsMid(note):  # gcs note -> midi pitch
    DBGPRT('NOTDONEYET - gcsMid')
    r = re.search(r"([_^]*)([A-Ga-g])([',]*)", note)
    if not r:
        return -1
    # fi
    acc, n, oct = r.groups()
    nUp = n.upper()
    p = 60 + [0,2,4,5,7,9,11]['CDEFGAB'.index(nUp)] + (12 if nUp != n else 0);
    if acc:
        p += (1 if acc[0] == '^' else -1) * len(acc)
    # fi
    if oct:
        p += (12 if oct[0] == "'" else -12) * len(oct)
    # fi
    return p
# End of gcsMid

# ----------------------------------------------------------------------------
def staffStep(ptc, o, clef):
    ndif = 0
    if 'stafflines=1' in clef:
        ndif += 4                               # meaning of one line: E (xml) -> B (gcs)
    # fi
    if clef.startswith('bass'):
        ndif += 12                              # transpose bass -> treble(C3 -> A4)
    # fi
    if ndif:                                    # diatonic transposition == addition modulo 7
        nm7 = 'C,D,E,F,G,A,B'.split(',')
        n = nm7.index(ptc) + ndif
        ptc, o = nm7 [n % 7], o + n // 7        # Integer divide and truncate.
    # fi
    if o > 4:
        ptc = ptc.lower()
    # fi
    if o > 5:
        ptc = ptc + (o-5) * "'"
    # fi
    if o < 4:
        ptc = ptc + (4-o) * ","
    # fi
    return ptc
# End of staffStep

# ----------------------------------------------------------------------------
def setKey(fifths, mode):
    sharpness = ['f-', 'c-','g-','d-','a-','e-','b-','f','c','g','d','a', 'e', 'b', 'f+','c+','g+','d+','a+','e+','b+']
    offTab = {'maj':8, 'ion':8, 'm':11, 'min':11, 'aeo':11, 'mix':9, 'dor':10, 'phr':12, 'lyd':7, 'loc':13, 'non':8}
    mode = mode.lower()[:3]                     # only first three chars, no case
    key = sharpness[offTab[mode] + fifths] + (mode if offTab[mode] != 8 else '')
    accs = ['f','c','g','d','a','e','b']
    if fifths >= 0:
        msralts = dict(zip(accs[:fifths], fifths * [1]))
    else:
        msralts = dict(zip(accs[fifths:], -fifths * [-1]))
    # fi
    return key, msralts
# End of setKey

# ----------------------------------------------------------------------------
def insTup(ix, notes, fact):                    # read one nested tuplet
    DBGPRT('NOTDONEYET - insTup')
    tupcnt = 0
    nx = notes[ix]
    if 'start' in nx.tup:
        nx.tup.remove('start')                  # do recursive calls when starts remain
    # fi
    tix = ix                                    # index of first tuplet note
    fn, fd = fact                               # xml time-mod of the higher level
    fnum, fden = nx.fact                        # xml time-mod of the current level
    tupfact = fnum//fn, fden//fd                # gcs time mod of this level. Integer divide and truncate.
    while ix < len(notes):
        nx = notes[ix]
        if isinstance(nx, Elem) or nx.grace:
            ix += 1                             # skip all non tuplet elements
            continue
        # fi
        if 'start' in nx.tup:                   # more nested tuplets to start
            ix, tupcntR = insTup(ix, notes, tupfact)   # ix is on the stop note!
            tupcnt += tupcntR
        elif nx.fact:
            tupcnt += 1                         # count tuplet elements
        # fi
        if 'stop' in nx.tup:
            nx.tup.remove('stop')
            break
        # fi
        if not nx.fact:                         # stop on first non tuplet note
            ix = lastix                         # back to last tuplet note
            break
        # fi
        lastix = ix
        ix += 1
    # fi
    # put gcs tuplet notation before the recursive ones
    tup = (tupfact[0], tupfact[1], tupcnt)
    if tup == (3, 2, 3):
        tupPrefix = '(3'
    else:
        tupPrefix = '(%d:%d:%d' % tup
    # fi
    notes[tix].tupgcs = tupPrefix + notes[tix].tupgcs
    return ix, tupcnt                           # ix is on the last tuplet note
# End of insTup

# ----------------------------------------------------------------------------
def mkBroken(vs):                               # introduce broken rhythms (vs: one voice, one measure)
#--     DBGPRT('NOTDONEYET - mkBroken')
    vs = [n for n in vs if isinstance(n, Note)]
    i = 0
    while i < len(vs) - 1:
        n1, n2 = vs[i], vs[i+1]                 # scan all adjacent pairs
        # skip if note in tuplet or has no duration or outside beam
        if not n1.fact and not n2.fact and n1.dur > 0 and n2.beam:
            if n1.dur * 3 == n2.dur:
                n2.dur = (2 * n2.dur) // 3      # Integer divide and truncate.
                n1.dur = n1.dur * 2
                n1.after = '<' + n1.after
                i += 1                          # do not chain broken rhythms
            elif n2.dur * 3 == n1.dur:
                n1.dur = (2 * n1.dur) // 3      # Integer divide and truncate.
                n2.dur = n2.dur * 2
                n1.after = '>' + n1.after
                i += 1                          # do not chain broken rhythms
            # fi
        # fi
        i += 1
    # elihw
# End of mkBroken

# ----------------------------------------------------------------------------
def outVoice(measure, divs, im, ip):    # note/elem objects of one measure in one voice
    ix = 0
    while ix < len(measure):                    # set all(nested) tuplet annotations
        nx = measure[ix]
        if isinstance(nx, Note) and nx.fact and not nx.grace:
            ix, tupcnt = insTup(ix, measure, (1, 1))   # read one tuplet, insert annotation(s)
        # fi
        ix += 1 
    # elihw
    vs = []
    for nx in measure:
        if isinstance(nx, Note):
            durstr = gcsdur(nx, divs)           # xml -> gcs duration string
            chord = len(nx.ns) > 1
#--             cns = [nt[:-1] for nt in nx.ns if nt.endswith('-')]
            cns = [nt[:-1] for nt in nx.ns if nt.endswith('t')]
            tie = ''
            if chord and len(cns) == len(nx.ns):      # all chord notes tied
                nx.ns = cns                     # chord notes without tie
                tie = 't'                       # one tie for whole chord
            # fi
            s = nx.tupgcs + ''.join(nx.before)
            if chord:
                s += '['
            # fi
            cnt = 0
            for nt in nx.ns:
                if cnt != 0:
                    s += ' '
                # fi
                s += nt
                cnt += 1
            # rof
            if chord:
                s += ']' + tie
            # fi
            if s.endswith('t'):
                s, tie = s[:-1], 't'            # split off tie
            # fi
            s += durstr + tie                   # and put it back again
            s += nx.after
#--            nospace = nx.beam
        else:
            if isinstance(nx.str, listtype):
                nx.str = nx.str[0]
            s = nx.str
#--            nospace = 1
        # fi
#--        if nospace:
#--            vs.append(s)
#--        else:
        vs.append(' ' + s)
#--     # fi
    # rof
    vs = ''.join(vs)                            # ad hoc: remove multiple pedal directions
    while vs.find('!ped!!ped!') >= 0:
        vs = vs.replace('!ped!!ped!','!ped!')
    # elihw
    while vs.find('!ped-up!!ped-up!') >= 0:
        vs = vs.replace('!ped-up!!ped-up!','!ped-up!')
    # elihw
    while vs.find('!8va(!!8va)!') >= 0:
        vs = vs.replace('!8va(!!8va)!','')      # remove empty ottava's
    # elihw
    return vs
# End of mkBroken

# ----------------------------------------------------------------------------
def sortMeasure(voice, m):
#--     DBGPRT('NOTDONEYET - sortMeasure')
    voice.sort(key=lambda o: o.tijd)            # sort on time
    time = 0
    v = []
    rs = []                                     # holds rests in between notes
    for i, nx in enumerate(voice):              # establish sequentiality
        if nx.tijd > time and chkbug(nx.tijd - time, m):
            v.append(Note(nx.tijd - time, 'r')) # fill hole with invisble rest
            rs.append(len(v) - 1)
        if isinstance(nx, Elem):
            if nx.tijd < time:
                nx.tijd = time                  # shift elems without duration to where they fit
            # fi
            v.append(nx)
            time = nx.tijd
            continue
        # fi
        if nx.tijd < time:                      # overlapping element
            if nx.ns[0] == 'z':
                continue                        # discard overlapping rest
            if v[-1].tijd <= nx.tijd:           # we can do something
                if v[-1].ns[0] == 'z':          # shorten rest
                    v[-1].dur = nx.tijd - v[-1].tijd
                    if v[-1].dur == 0:
                        del v[-1]               # nothing left
                    # fi
                    info(f'overlap in part {m.ixp+1}, measure {m.ixm+1}: rest shortened')
                else:                           # make a chord of overlap
                    v[-1].ns += nx.ns
                    info(f'overlap in part {m.ixp+1}, measure {m.ixm+1}: added chord')
                    nx.dur = (nx.tijd + nx.dur) - time  # the remains
                    if nx.dur <= 0:
                        continue                # nothing left
                    # fi
                    nx.tijd = time              # append remains
                # fi
            else:                               # give up
                info(f'overlapping notes in one voice! part {m.ixp+1}, measure {m.ixm+1}, note {isinstance(nx, Note) and nx.ns or nx.str} discarded')
                continue
            # fi
        # fi
        v.append(nx)
        if isinstance(nx, Note):
            if nx.ns[0] in 'zx':
                rs.append(len(v) - 1)           # remember rests between notes
            elif len(rs):
                if nx.beam and not nx.grace:    # copy beam into rests
                    for j in rs:
                        v[j].beam = nx.beam
                    # rof
                # fi
                rs = []                         # clear rests on each note
            # fi
        # fi
        time = nx.tijd + nx.dur
    # rof
    # When a measure contains no elements and no forwards -> no incTime -> s.maxtime = 0 -> right barline
    # is inserted at time == 0 (in addbar) and is only element in the voice when sortMeasure is called
    if time == 0:
        info(f'empty measure in part {m.ixp+1}, measure {m.ixm+1}, it should contain at least a rest to advance the time!')
    # fi
    return v
# End of sortMeasure

# ----------------------------------------------------------------------------
def getPartlist(ps):                            # correct part-list(from buggy xml-software)
    xs = []                                     # the corrected part-list
    e = []                                      # stack of opened part-groups
    for x in list(ps):                          # insert missing stops, delete double starts
        if x.tag ==  'part-group':
            num, type = x.get('number'), x.get('type')
            if type == 'start':
                if num in e:                    # missing stop: insert one
                    xs.append(E.Element('part-group', number = num, type = 'stop'))
                    xs.append(x)
                else:                           # normal start
                    xs.append(x)
                    e.append(num)
                # fi
            else:
                if num in e:                    # normal stop
                    e.remove(num)
                    xs.append(x)
                else:
                    pass                        # double stop: skip it
                # fi
            # fi
        else:
            xs.append(x)
        # fi
    # rof
    for num in reversed(e):                     # fill missing stops at the end
        xs.append(E.Element('part-group', number = num, type = 'stop'))
    # rof
    return xs
# End of getParlist

# ----------------------------------------------------------------------------
def parseParts(xs, d, e):                       # -> [elems on current level], rest of xs
    if not xs:
        return [],[]
    # fi
    x = xs.pop(0)
    if x.tag == 'part-group':
        num, type = x.get('number'), x.get('type')
        if type == 'start':
            # go one level deeper
            s = [x.findtext(n, '') for n in['group-symbol','group-barline','group-name','group-abbreviation']]
            d[num] = s                          # remember groupdata by group number
            e.append(num)                       # make stack of open group numbers
            elemsnext, rest1 = parseParts(xs, d, e)     # parse one level deeper to next stop
            elems, rest2 = parseParts(rest1, d, e)      # parse the rest on this level
            return [elemsnext] + elems, rest2
        else:                                   # stop: close level and return group-data
            nums = e.pop()                      # last open group number in stack order
            if xs and xs[0].get('type') == 'stop':      # two consequetive stops
                if num != nums:                         # in the wrong order(tempory solution)
                    d[nums], d[num] = d[num], d[nums]   # exchange values    (only works for two stops!!!)
                # fi
            # fi
            sym = d[num]                        # retrieve an return groupdata as last element of the group
            return [sym], xs
        # fi
    else:
        elems, rest = parseParts(xs, d, e)      # parse remaining elements on current level
        name = x.findtext('part-name',''), x.findtext('part-abbreviation','')
        return [name] + elems, rest
    # fi
# End of parseParts

# ----------------------------------------------------------------------------
def bracePart(part):                            # put a brace on multistaff part and group voices
    DBGPRT('NOTDONEYET - bracePart')
    if not part:
        return []                                # empty part in the score
    # fi
    brace = []
    for ivs in part:
        if len(ivs) == 1:
            # stave with one voice
            brace.append(f'{ivs[0]}')
        else:                                   # stave with multiple voices
            brace += ['('] + ['%s' % iv for iv in ivs] + [')']
        # fi
#--         brace.append('|')
        brace.append('\n')
    # rof
    del brace[-1]                               # no barline at the end
    if len(part) > 1:
        brace = ['{'] + brace + ['}']
    # fi
    return brace
# End of bracePart

# ----------------------------------------------------------------------------
def prgroupelem(x, gnm, bar, pmap, accVce, accStf): # collect partnames(accVce) and %%score map(accStf)
    DBGPRT('NOTDONEYET - prgroupelem')
    if type(x) == tupletype:                    # partname-tuple = (part-name, part-abbrev)
        y = pmap.pop(0)
        if gnm[0]:
            x = [n1 + 'XXX:' + n2 for n1, n2 in zip(gnm, x)]   # put group-name before part-name
        # fi
        accVce.append(x)
        accStf.extend(bracePart(y))
    elif len(x) == 2 and type(x[0]) == tupletype: # misuse of group just to add extra name to stave
        y = pmap.pop(0)
        nms = [n1 + 'YYY:' + n2 for n1, n2 in zip(x[0], x[1][2:])]    # x[0] = partname-tuple, x[1][2:] = groupname-tuple
        accVce.append(nms)
        accStf.extend(bracePart(y))
    else:
        prgrouplist(x, bar, pmap, accVce, accStf)
    # fi
# End of prgroupelem

# ----------------------------------------------------------------------------
def prgrouplist(x, pbar, pmap, accVce, accStf):    # collect partnames, scoremap for a part-group
    DBGPRT('NOTDONEYET - prgrouplist')
    sym, bar, gnm, gabbr = x[-1]                # bracket symbol, continue barline, group-name-tuple
    bar = bar == 'yes' or pbar                  # pbar -> the parent has bar
    accStf.append(sym == 'brace' and '{' or '[')
    for z in x[:-1]:
        prgroupelem(z, (gnm, gabbr), bar, pmap, accVce, accStf)
        if bar:
#--             accStf.append('|')
            accStf.append('\n')
        # fi
    # rof
    if bar:
        del accStf[-1]                          # remove last one before close
    # fi
    accStf.append(sym == 'brace' and '}' or ']')
# End of prgrouplist

# ----------------------------------------------------------------------------
def perc2map(gcsIn):
    DBGPRT('NOTDONEYET - perc2map')
    fillmap = {'diamond':1, 'triangle':1, 'square':1, 'normal':1};
    gcs = map(lambda x: x.strip(), percSvg.splitlines())
    id='default'
    maps = {'default': []};
    dmaps = {'default': []}
    r1 = re.compile(r'V:\s*(\S+)')
    ls = gcsIn.splitlines()
    for x in ls:
        if '%%MIDI' in x:
            dmaps[id].append(x)
        # fi
        if 'V:' in x:
            r = r1.match(x)
            if r:
                id = r.group(1);
                if id not in maps:
                    maps[id] = []; dmaps[id] = []
                # fi
            # fi
        # fi
    # rof
    ids = sorted(maps.keys())
    for id in ids:
        gcs += maps[id]
    # rof
    id='default'
    for x in ls:
        if '%%MIDI' in x:
            continue
        # fi
        if 'V:' in x or 'key ' in x:
            r = r1.match(x)
            if r:
                id = r.group(1)
            # fi
            gcs.append(x)
            if id in dmaps and len(dmaps[id]) > 0:
                gcs.extend(dmaps[id]); del dmaps[id]
            # fi
            if 'perc' in x and 'map=' not in x:
                x += ' map=perc';
            # fi
            if 'map=perc' in x and len(maps[id]) > 0:
                gcs.append('%%voicemap perc' + id);
            # fi
            if 'map=off' in x:
                gcs.append('%%voicemap');            
            # fi
        else:
            gcs.append(x)
        # fi
    # rof
    ret = '\n'.join(gcs) + '\n'
    ret = re.sub(r'\s\n\n*', '\n', ret)
    return ret
# End of perc2map

# ----------------------------------------------------------------------------
def addoct(ptc, o):                             # xml staff step, xml octave number
    p = str(o-1) + ptc.lower()
    return p                                    # gcs pitch == gcs note without accidental
# End of addoct

# ----------------------------------------------------------------------------
def chkbug(dt, m):
    if dt > m.divs / 16:
        return 1                                # duration should be > 1/64 note
    # fi
    info(f'MuseScore bug: incorrect duration, smaller then 1/64! in measure {m.ixm}, part {m.ixp}')
    return 0
# End of chkbug

# ------------------------------------------------------------------
class Parser:
    note_alts = [                               # 3 alternative notations of the same note for tablature mapping
        [x.strip() for x in '=C,  ^C, =D, ^D, =E, =F, ^F, =G, ^G, =A, ^A, =B'.split(',')],
        [x.strip() for x in '^B,  _D,^^C, _E, _F, ^E, _G,^^F, _A,^^G, _B, _C'.split(',')],
        [x.strip() for x in '__D,^^B,__E,__F,^^D,__G,^^E,__A,_/A,__B,__C,^^A'.split(',')] ]
    step_map = {'C':0,'D':2,'E':4,'F':5,'G':7,'A':9,'B':11}
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def __init__(s):
        # unfold repeats, number of chars per line, credit filter level, volta option
        s.slurBuf = {}    # dict of open slurs keyed by slur number
        s.dirStk = {}     # {direction-type + number -> (type, voice | time)} dict for proper closing
        s.ingrace = 0     # marks a sequence of grace notes
        s.msc = Music()   # global music data abstraction
        s.gStfMap = []    # [[gcs voice numbers] for all parts]
        s.midiMap = []    # midi-settings for each gcs voice, in order
        s.drumInst = {}   # inst_id -> midi pitch for channel 10 notes
        s.drumNotes = {}  # (xml voice, gcs note) -> (midi note, note head)
        s.instMid = []    # [{inst id -> midi-settings} for all parts]
        s.midDflt = [-1,-1,-1,-91] # default midi settings for channel, program, volume, panning
        s.msralts = {}    # xml-notenames(without octave) with accidentals from the key
        s.curalts = {}    # gcs-notenames(with voice number) with passing accidentals
        s.stfMap = {}     # xml staff number -> [xml voice number]
        s.vce2stf = {}    # xml voice number -> allocated staff number
        s.clefMap = {}    # xml staff number -> gcs clef (for header only)
        s.curClef = {}    # xml staff number -> current gcs clef
        s.stemDir = {}    # xml voice number -> current stem direction
        s.clefOct = {}    # xml staff number -> current clef-octave-change
        s.curStf = {}     # xml voice number -> current xml staff number
        s.repeat_str = {} # staff number -> [measure number, repeat-text]
        s.tabVceMap = {}  # gcs voice num -> [%%map ...] for tab voices
        s.koppen = {}     # noteheads needed for %%map
    # End of __init__
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def matchSlur(s, type2, n, v2, note2, grace, stopgrace): # match slur number n in voice v2, add gcs code to before/after
#--         DBGPRT(f'NOTDONEYET - matchSlur - type2={type2}')
        if type2 not in['start', 'stop']:
            return                              # slur type continue has no gcs equivalent
        # fi
        if n == None:
            n = '1'
        # fi
        if n in s.slurBuf:
            type1, v1, note1, grace1 = s.slurBuf[n]
            if type2 != type1:                  # slur complete, now check the voice
                if v2 == v1:                    # begins and ends in the same voice: keep it
                    if type1 == 'start' and (not grace1 or not stopgrace):  # normal slur: start before stop and no grace slur
                        note1.before = ['('] + note1.before # keep left-right order!
                        note1.after += 'l'     # keep left-right order!
                        note2.after += ')'
                    # fi
                    # no else: don't bother with reversed stave spanning slurs
                # fi
                del s.slurBuf[n]                # slur finished, remove from stack
            else:                               # double definition, keep the last
                info(f'double slur numbers {type2}-{n} in part {s.msr.ixp+1}, measure {s.msr.ixm+1}, voice {v2} note {note2.ns}, first discarded')
                s.slurBuf[n] = (type2, v2, note2, grace)
            # fi
        else:                                   # unmatched slur, put in dict
            s.slurBuf[n] = (type2, v2, note2, grace)
        # fi
    # End of matchSlur
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def doNotations(s, note, nttn):
        stacc = nttn.find('articulations/staccato')
        if stacc != None:
            note.after += 's'                   # Put staccato after the note.
        # fi
    # End of doNotations
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def tabnote(s, alt, ptc, oct, v, ntrec):
        DBGPRT('NOTDONEYET - tabnote')
        p = s.step_map[ptc] + int(alt or '0') # p in -2 .. 13
        if p > 11:
            oct += 1                            # octave correction
        # fi
        if p < 0:
            oct -= 1
        # fi
        p = p % 12                              # remap p into 0..11
        snaar_nw, fret_nw = ntrec.tab           # the computed/annotated allocation of nt
        for i in range(4):                      # support same note on 4 strings
            na = s.note_alts[i % 3] [p]         # get alternative representation of same note
            o = oct
            if na in['^B', '^^B']:
                o -= 1  # because in adjacent octave
            # fi
            if na in['_C', '__C']:
                o += 1
            # fi
            if '/' in na or i == 3:
                o = 9                           # emergency notation for 4th string case
            # fi
            nt = addoct(na, o)
            snaar, fret = s.tabmap.get((v, nt), ('', ''))  # the current allocation of nt
            if not snaar:
                break                           # note not yet allocated
            # fi
            if snaar_nw == snaar:
                return nt # use present allocation
            # fi
            if i == 3:                          # new allocaion needed but none is free
                fmt = 'rejected: voice %d note %3s string %s fret %2s remains: string %s fret %s'
                info(fmt % (v, nt, snaar_nw, fret_nw, snaar, fret), 1)
                ntrec.tab = (snaar, fret)
            # fi
        # rof
        s.tabmap[v, nt] = ntrec.tab             # for tablature map(voice, note) -> (string, fret)
        return nt                               # always in key C (with midi pitch alterations)
    # End of tabnote
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def ntGCS(s, ptc, oct, note, v, ntrec):     # pitch, octave -> gcs notation
        acc2alt = {'double-flat':-2,'flat-flat':-2,'flat':-1,'natural':0,'sharp':1,'sharp-sharp':2,'double-sharp':2}
        oct += s.clefOct.get(s.curStf[v], 0)    # minus clef-octave-change value
        acc = note.findtext('accidental')       # should be the notated accidental
        alt = note.findtext('pitch/alter')      # pitch alteration(midi)
        if ntrec.tab:
            return s.tabnote(alt, ptc, oct, v, ntrec)    # implies s.tstep is true (options.t was given)
        # fi
        p = addoct(ptc, oct)
        if alt == None and s.msralts.get(ptc, 0):
            alt = 0                             # no alt but key implies alt -> natural!!
        # fi
        if alt == None and (p, v) in s.curalts:
            alt = 0                             # no alt but previous note had one -> natural!!
        # fi
        if acc == None and alt == None:
            return p                            # no acc, no alt
        elif acc != None:
            alt = acc2alt[acc]                  # acc takes precedence over the pitch here!
        else:                                   # now see if we really must add an accidental
            alt = int(float(alt))
            if (p, v) in s.curalts:             # the note in this voice has been altered before
                if alt == s.curalts[(p, v)]:
                    return p                    # alteration still the same
                # fi
            elif alt == s.msralts.get(ptc, 0):
                return p                        # alteration implied by the key
            # fi
            tieElms = note.findall('tie') + note.findall('notations/tied')    # in xml we have separate notated ties and playback ties
            if 'stop' in[e.get('type') for e in tieElms]:
                return p    # don't alter tied notes
            # fi
            info(f'accidental {alt} added in part {s.msr.ixp+1}, measure {s.msr.ixm+1}, voice {v+2} note {p}')
        # fi
        s.curalts[(p, v)] = alt
        p = p + ['--','-','n','+','++'][alt+2]  # Append the accidental.
        return p
    # End of ntGCS
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def doNote(s, n):                           # parse a musicXML note tag
        note = Note()
        v = int(n.findtext('voice', '1'))
        if s.isSib:
            v += 100 * int(n.findtext('staff', '1'))  # repair bug in Sibelius
        # fi
        chord = n.find('chord') != None
        p = n.findtext('pitch/step') or n.findtext('unpitched/display-step')
        if p:
            p = p.lower()
        # fi
        o = n.findtext('pitch/octave') or n.findtext('unpitched/display-octave')
        r = n.find('rest')
        numer = n.findtext('time-modification/actual-notes')
        if numer:
            denom = n.findtext('time-modification/normal-notes')
            note.fact = (int(numer), int(denom))
        # fi
        note.tup = [x.get('type') for x in n.findall('notations/tuplet')]
        dur = n.findtext('duration')
        grc = n.find('grace')
        note.grace = grc != None
        note.before, note.after = [], ''        # strings with stuff that goes before or after a note/chord
        if note.grace and not s.ingrace:        # open a grace sequence
            s.ingrace = 1
            note.after = 'g'
            if grc.get('slash') == 'yes':
                note.after += 'l '               # Try a slurred grace note.
            # fi
        # fi
        stopgrace = not note.grace and s.ingrace
        if stopgrace:                           # close the grace sequence
            s.ingrace = 0
        # fi
        if dur == None or note.grace:
            dur = 0
        # fi
        if r == None and n.get('print-object') == 'no':
            if chord:
                return
            # fi
            r = 1  # turn invisible notes(that advance the time) into invisible rests
        # fi
        note.dur = int(dur)
        if r == None and (not p or not o):      # not a rest and no pitch
            s.msc.cnt.inc('nopt', v)            # count unpitched notes
            o, p = 5,'e'                        # make it an E5 ??
        # fi
        nttn = n.find('notations')              # add ornaments (staccato)
        if nttn != None:
            s.doNotations(note, nttn)
        # fi
        if r != None:
            noot = 'r'                          # rest
        else:
            noot = s.ntGCS(p, int(o), n, v, note)
        # fi
        if n.find('unpitched') != None:
            clef = s.curClef[s.curStf[v]]       # the current clef for this voice
            step = staffStep(p, int(o), clef)   # (clef independent) step value of note on the staff
            instr = n.find('instrument')
            instId = instr.get('id') if instr != None else 'dummyId'
            midi = s.drumInst.get(instId, gcsMid(noot))
            nh =  n.findtext('notehead', '').replace(' ','-')   # replace spaces in xml notehead names for percmap
            s.drumNotes[(v, noot)] = (step, midi, nh)           # keep data for percussion map
        # fi
        tieElms = n.findall('tie') + n.findall('notations/tied')    # in xml we have separate notated and playback ties
        if 'start' in[e.get('type') for e in tieElms]:          # n can have stop and start tie
            note.after += 't'                                   # Tack on tie.
        # fi
        note.beam = sum([1 for b in n.findall('beam') if b.text in['continue', 'end']]) + int(note.grace)
        stemdir = n.findtext('stem')
        if chord:
            s.msc.addChord(note, noot)
        else:
            xmlstaff = int(n.findtext('staff', '1'))
            if s.curStf[v] != xmlstaff:         # the note should go to another staff
                s.curStf[v] = xmlstaff          # remember the new staff for this voice
            # fi
            s.msc.appendNote(v, note, noot)
        # fi
        for slur in n.findall('notations/slur'):    # s.msc.lastnote points to the last real note/chord inserted above
# NOTDONEYET
            s.matchSlur(slur.get('type'), slur.get('number'), v, s.msc.lastnote, note.grace, stopgrace) # match slur definitions
        # rof
    # End of doNote
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def doAttr(s, e):                           # parse a musicXML attribute tag
        teken = {'C1':'alto1','C2':'alto2','C3':'alto','C4':'tenor','F4':'bass','F3':'bass3','G2':'treble','TAB':'tab','percussion':'perc'}
        dvstxt = e.findtext('divisions')
        if dvstxt:
            s.msr.divs = int(dvstxt)
        # fi
        steps = int(e.findtext('transpose/chromatic', '0')) # for transposing instrument
        fifths = e.findtext('key/fifths')
        first = s.msc.tijd == 0 and s.msr.ixm == 0  # first attributes in first measure
        if fifths:
            key, s.msralts = setKey(int(fifths), e.findtext('key/mode','major'))
            if first and not steps and gcsOut.key == 'none':
                gcsOut.key = key                # first measure -> header, if not transposing instrument or percussion part!
            elif key != gcsOut.key or not first:
                s.msr.attr += f'\nkey {key}\n'    # otherwise -> voice
            # fi
        # fi
        beats = e.findtext('time/beats')
        if beats:
            unit = e.findtext('time/beat-type')
            mtr = beats + '/' + unit
            if first:
                gcsOut.mtr = mtr                # first measure -> header
            else:
                s.msr.attr += f'\nmeter {mtr}\n' # otherwise -> voice
            # fi
            s.msr.mtr = int(beats), int(unit)
        # fi
        s.msr.mdur = (s.msr.divs * s.msr.mtr[0] * 4) // s.msr.mtr[1]    # duration of measure in xml-divisions - divide and truncate.
        for ms in e.findall('measure-style'):
            n = int(ms.get('number', '1'))      # staff number
            voices = s.stfMap[n]                # all voices of staff n
            for mr in ms.findall('measure-repeat'):
                ty = mr.get('type')
                if ty == 'start':               # remember start measure number and text voor each staff
                    s.repeat_str[n] = [s.msr.ixm, mr.text]
                    for v in voices:            # insert repeat into all voices, value will be overwritten at stop
                        s.msc.insertElem(v, s.repeat_str[n])
                    # rof
                elif ty == 'stop':              # calculate repeat measure count for this staff n
                    start_ix, text_ = s.repeat_str[n]
                    repeat_count = s.msr.ixm - start_ix
                    if text_:
                        mid_str =  f'{text_} '
                        repeat_count /= int(text_)
                    else:
                        mid_str = ""            # overwrite repeat with final string
                    # fi
                    s.repeat_str[n][0] = '[I:repeat %s%d]' % (mid_str, repeat_count)
                    del s.repeat_str[n]         # remove closed repeats
                # fi
            # rof
        # rof
        toct = e.findtext('transpose/octave-change', '')
        if toct:
            steps += 12 * int(toct)             # extra transposition of toct octaves
        # fi
        for clef in e.findall('clef'):          # a part can have multiple staves
            n = int(clef.get('number', '1'))    # local staff number for this clef
            sgn = clef.findtext('sign')
            line = clef.findtext('line', '') if sgn not in['percussion','TAB'] else ''
            cs = teken.get(sgn + line, '')
            if cs != '':
                cs = 'clef%%' + cs
            # fi
            oct = clef.findtext('clef-octave-change', '') or '0'
            if oct:
                cs += {-2:'-15', -1:'-8', 1:'+8', 2:'+15'}.get(int(oct), '')
            # fi
            s.clefOct[n] = -int(oct);           # xml playback pitch -> gcs notation pitch
            if steps:
                if cs != '':
                    cs += '\n'
                # fi
#--                cs += ' transpose=' + str(steps)
                cs += 'xpose%%' + str(steps)
            # fi
            stfdtl = e.find('staff-details')
            if stfdtl != None and int(stfdtl.get('number', '1')) == n:
                lines = stfdtl.findtext('staff-lines')
                if lines:
                    lns= '|||' if lines == '3' and sgn == 'TAB' else lines
                    cs += f' stafflines={lns}'
                    s.stafflines = int(lines)   # remember for tab staves
                # fi
                strings = stfdtl.findall('staff-tuning')
                if strings:
                    tuning = [st.findtext('tuning-step') + st.findtext('tuning-octave') for st in strings]
                    cs += ' strings=%s' % ','.join(tuning) 
                # fi
                capo = stfdtl.findtext('capo')
                if capo:
                    cs += f' capo={capo}'
                # fi
            # fi
            s.curClef[n] = cs                   # keep track of current clef (for percmap)
            if first:
                s.clefMap[n] = cs               # clef goes to header(where it is mapped to voices)
            else:
                voices = s.stfMap[n]            # clef change to all voices of staff n
                for v in voices:
                    if n != s.curStf[v]:        # voice is not at its home staff n
                        s.curStf[v] = n         # reset current staff at start of measure to home position
                    # fi
#--                     s.msc.appendElem(v, f'\nkey {cs}\n')
                    s.msc.appendElem(v, f'\n{cs}\n')
                # rof
            # fi
        # rof
    # End of doAttr
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def findVoice(s, i, es):
        stfnum = int(es[i].findtext('staff',1)) # directions belong to a staff
        vs = s.stfMap[stfnum]                   # voices in this staff
        v1 = vs[0] if vs else 1                 # directions to first voice of staff
        for e in es[i+1:]:                      # or to the voice of the next note
            if e.tag == 'note':
                v = int(e.findtext('voice', '1'))
                if s.isSib:
                    v += 100 * int(e.findtext('staff', '1'))  # repair bug in Sibelius
                # fi
                stf = s.vce2stf[v]              # use our own staff allocation
                return stf, v, v1               # voice of next note, first voice of staff
            # fi
            if e.tag == 'backup':
                break
            # fi
        # rof
        return stfnum, v1, v1                   # no note found, fall back to v1
    # End of findVoice
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def doDirection(s, e, i, es):               # parse a musicXML direction tag
        # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
        def addDirection(x, vs, tijd, stfnum):
            if not x:
                return
            # fi
            vs = s.stfMap[stfnum] if '!8v' in x else[vs]  # ottava's go to all voices of staff
            for v in vs:
                if tijd != None:                # insert at time of encounter
                    s.msc.appendElemT(v, x.replace('(',')').replace('ped','ped-up'), tijd)
                else:
                    s.msc.appendElem(v, x)
                # fi
            # rof
        # End of addDirection
        # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
        def startStop(dtype, vs, stfnum=1):
            typmap = {'down':'!8va(!', 'up':'!8vb(!', 'crescendo':' vol< ', 'diminuendo':' vol> ', 'start':'!ped!'}
            type = t.get('type', '')
            k = dtype + t.get('number', '1')    # key to match the closing direction
            if type in typmap:                  # opening the direction
                x = typmap[type]
                if k in s.dirStk:               # closing direction already encountered
                    stype, tijd = s.dirStk[k]; del s.dirStk[k]
                    if stype == 'stop':
                        addDirection(x, vs, tijd, stfnum)
                    else:
                        info(f'{dtype} direction {stype} has no stop in part {s.msr.ixp+1}, measure {s.msr.ixm+1}, voice {vs+1}')
                        s.dirStk[k] = ((type , vs)) # remember voice and type for closing
                    # fi
                else:
                    s.dirStk[k] = ((type , vs)) # remember voice and type for closing
                # fi
            elif type == 'stop':
                if k in s.dirStk:               # matching open direction found
                    type, vs = s.dirStk[k]; del s.dirStk[k]   # into the same voice
                    if type == 'stop':
                        info(f'{dtype} direction {type} has double stop in part {s.msr.ixp+1}, measure {s.msr.ixm+1}, voice {vs+1}')
                        x = ''
                    else:
                        x = typmap[type].replace('(',')').replace('ped','ped-up')
                    # fi
                else:                           # closing direction found before opening
                    s.dirStk[k] = ('stop', s.msc.tijd)
                    x = ''                      # delay code generation until opening found
                # fi
            else:
                raise ValueError('wrong direction type')
            # fi
            addDirection(x, vs, None, stfnum)
        # End of startStop
        # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

        tempo, wrdstxt = None, ''
        plcmnt = e.get('placement')
        stf, vs, v1 = s.findVoice(i, es)
        jmp = ''                                # for jump sound elements: dacapo, dalsegno and family
        jmps = [('dacapo','D.C.'),('dalsegno','D.S.'),('tocoda','dacoda'),('fine','fine'),('coda','O'),('segno','S')]
        t = e.find('sound')                     # there are many possible attributes for sound
        if t != None:
            minst = t.find('midi-instrument')
            if minst:
                prg = t.findtext('midi-instrument/midi-program')
                chn = t.findtext('midi-instrument/midi-channel')
                vids = [v for v, id in s.vceInst.items() if id == minst.get('id')]
                if vids:
                    vs = vids[0]                # direction for the indentified voice, not the staff
                # fi
                parm, inst = ('program', str(int(prg) - 1)) if prg else('channel', chn)
            # fi
            tempo = t.get('tempo')              # look for tempo attribute
            if tempo:
                tempo = f'{float(tempo):.0f}'   # hope it is a number and insert in voice 1
                tempo_units = 4                 # always 1/4 for sound elements!
            # fi
            for r, v in jmps:
                if t.get(r, ''):
                    jmp = v; break
                # fi
            # rof
        # fi
        dirtypes = e.findall('direction-type')
        for dirtyp in dirtypes:
            units = { 'whole': 1, 'half': 2, 'quarter': 4, 'eighth': 8 }
            metr = dirtyp.find('metronome')
            if metr != None:
                t = metr.findtext('beat-unit', '')
                if t in units:
                    tempo_units = units[t]
                else:
                    tempo_units = units['quarter']
                # fi
                if metr.find('beat-unit-dot') != None:
                    tempo_units = simplify(3, tempo_units * 2)
                    flt = tempo_units[0] / tempo_units[1]
                    if flt in legal_mc_notes:
                        tempo_units = legal_mc_notes[flt]
                    else:
                        tempo_units = 'UNKNOWN'
                    # fi
                # fi
                tmpro = re.search(r'[.\d]+', metr.findtext('per-minute'))  # look for a number
                if tmpro:
                    tempo = tmpro.group()       # overwrites the value set by the sound element of this direction
                # fi
            # fi
            t = dirtyp.find('wedge')
            if t != None:
                startStop('wedge', vs)
            # fi
#--             allwrds = dirtyp.findall('words')   # insert text annotations
#--             if not allwrds:
#--                 allwrds = dirtyp.findall('rehearsal')   # treat rehearsal mark as text annotation
#--             # fi
#--             for wrds in allwrds:
#--                 if jmp:                         # ignore words when a jump sound element is present this direction
#-- # NOTDONEYET this whole business.
#--                     s.msc.appendElem(vs, '!%s!' % jmp , 1)  # to voice
#--                     break
#--                 # fi
#--                 plc = plcmnt == 'below' and '_' or '^'
#--                 if float(wrds.get('default-y', '0')) < 0:
#--                     plc = '_'
#--                 # fi
#--                 DBGPRT(f'HERE#1 - wrds.text={wrds.text}')
#--                 wrdstxt += (wrds.text or '').replace('"','\\"').replace('\n', '\\n')
#--             # rof
#--             wrdstxt = wrdstxt.strip()
            for key, val in dynamics_map.items():
                if dirtyp.find('dynamics/' + key) != None:
                    s.msc.appendElem(vs, val, 1)    # to voice
                # fi
            # rof
            if dirtyp.find('coda') != None:
                s.msc.appendElem(vs, 'O', 1)
            # fi
            if dirtyp.find('segno') != None:
                s.msc.appendElem(vs, 'S', 1)
            # fi
            t = dirtyp.find('octave-shift')
            if t != None:
                startStop('octave-shift', vs, stf)  # assume size == 8 for the time being
            # fi
            t = dirtyp.find('pedal')
            if dirtyp.findtext('other-direction') == 'diatonic fretting':
                s.diafret = 1;
            # fi
        # rof
        if tempo:
            tempo = f'{float(tempo):.0f}'           # hope it is a number and insert in voice 1
            if s.msc.tijd == 0 and s.msr.ixm == 0:  # first measure -> header
                gcsOut.tempo = tempo
                gcsOut.tempo_units = tempo_units
            else:
                s.msc.appendElem(v1, f'\ntempo   {tempo},{tempo_units}\n')  # otherwise -> 1st voice
            # fi
        # fi
#--         if wrdstxt:
#--             s.msc.appendElem(vs, f'"{plc}{wrdstxt}"', 1) # to voice, but after tempo
#--         # fi
    # End of doDirection
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def doHarmony(s, e, i, es):                     # parse a musicXMl harmony tag
        DBGPRT('NOTDONEYET - doHarmony')
        _, vt, _ = s.findVoice(i, es)
        short = {'major':'', 'minor':'m', 'augmented':'+', 'diminished':'dim', 'dominant':'7', 'half-diminished':'m7b5'}
        accmap = {'major':'maj', 'dominant':'', 'minor':'m', 'diminished':'dim', 'augmented':'+', 'suspended':'sus'}
        modmap = {'second':'2', 'fourth':'4', 'seventh':'7', 'sixth':'6', 'ninth':'9', '11th':'11', '13th':'13'}
        altmap = {'1':'#', '0':'', '-1':'b'}
        root = e.findtext('root/root-step','')
        alt = altmap.get(e.findtext('root/root-alter'), '')
        sus = ''
        kind = e.findtext('kind', '')
        if kind in short:
            kind = short[kind]
        elif '-' in kind:                           # xml chord names: <triad name>-<modification>
            triad, mod = kind.split('-')
            kind = accmap.get(triad, '') + modmap.get(mod, '')
            if kind.startswith('sus'):
                kind, sus = '', kind                # sus-suffix goes to the end
            # fi
        elif kind == 'none':
            kind = e.find('kind').get('text','')
        # fi
        degrees = e.findall('degree')
        for d in degrees:                           # chord alterations
            kind += altmap.get(d.findtext('degree-alter'),'') + d.findtext('degree-value','')
        kind = kind.replace('79','9').replace('713','13').replace('maj6','6')
        bass = e.findtext('bass/bass-step','') + altmap.get(e.findtext('bass/bass-alter'),'') 
        s.msc.appendElem(vt, '"%s%s%s%s%s"' % (root, alt, kind, sus, bass and '/' + bass), 1)
    # End of doHarmony
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def doBarline(s, e):                            # 0 = no repeat, 1 = begin repeat, 2 = end repeat
        rep = e.find('repeat')
        if rep != None:
            rep = rep.get('direction')
        # fi
        loc = e.get('location', 'right')            # right is the default
        if loc == 'right':                          # only change style for the right side
            style = e.findtext('bar-style')
            if style == 'light-light':
                s.msr.rline = '||'
            elif style == 'light-heavy':
                s.msr.rline = '|]'
            # fi
        # fi
        if rep != None:                             # repeat found
            if rep == 'forward':
                s.msr.lline = '$$ goto comes to here'   # goto comes to here
            else:
                s.msr.rline = '$$ goto someplace from here' # override barline style
            # fi
        # fi
        end = e.find('ending')
        if end != None:
            if end.get('type') == 'start':
                n = end.get('number', '1').replace('.','').replace(' ','')
                try:
                    list(map(int, n.split(',')))    # should be a list of integers
                except:
                    n = '"%s"' % n.strip()          # illegal musicXML
                # yrt
                s.msr.lnum = n                      # assume a start is always at the beginning of a measure
            elif s.msr.rline == '|':                # stop and discontinue the same  in GCS ?
                s.msr.rline = '||'                  # to stop on a normal barline use || in GCS ?
            # fi
        # fi
        return 0
    # End of doBarline
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def doPrint(s, e):                              # print element, measure number -> insert a line break
        # This is really bars per page.
        if e.get('new-system') == 'yes' or e.get('new-page') == 'yes':
            return '\n* end of page'                # NOTDONEYET - is this the end of the bars for staves?
        # fi
    # End of doPrint
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def doPartList(s, e):                           # translate the start/stop-event-based xml-partlist into proper tree
        for sp in e.findall('part-list/score-part'):
            midi = {}
            for m in sp.findall('midi-instrument'):
                x = [m.findtext(p, s.midDflt[i]) for i,p in enumerate(['midi-channel','midi-program','volume','pan'])]
                pan = float(x[3])
                if pan >= -90 and pan <= 90:        # would be better to map behind-pannings
                    pan = (float(x[3]) + 90) / 180 * 127    # xml between -90 and +90
                # fi
                midi[m.get('id')] = [int(x[0]), int(x[1]), float(x[2]) * 1.27, pan] # volume 100 -> midi 127
                up = m.findtext('midi-unpitched')
                if up:
                    s.drumInst[m.get('id')] = int(up) - 1   # store midi-pitch for channel 10 notes
                # fi
            # rof
            s.instMid.append(midi)
        # rof
        ps = e.find('part-list')                    # partlist  = [groupelem]
        xs = getPartlist(ps)                        # groupelem = partname | grouplist
        partlist, _ = parseParts(xs, {}, [])        # grouplist = [groupelem, ..., groupdata]
        return partlist                             # groupdata = [group-symbol,group-barline,group-name,group-abbrev]
    # End of doPartList
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def mkTitle(s, e):
        def filterCredits(y):                       # y == filter level, higher filters less
            cs = []
            for x in credits:                       # skip redundant credit lines
                if y < 6 and (x in title or x in mvttl):
                    continue                        # sure skip
                # fi
                if y < 5 and (x in composer or x in lyricist):
                    continue                        # almost sure skip
                # fi
                if y < 4 and ((title and title in x) or (mvttl and mvttl in x)):
                    continue                        # may skip too much
                # fi
                if y < 3 and ([1 for c in composer if c in x] or[1 for c in lyricist if c in x]):
                    continue                        # skips too much
                # fi
                if y < 2 and re.match(r'^[\d\W]*$', x):
                    continue                        # line only contains numbers and punctuation
                # fi
                cs.append(x)
            # rof
            if y == 0 and (title + mvttl):
                cs = ''                             # default: only credit when no title set
            # fi
            return cs
        # End of filterCredits
        # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

        title = e.findtext('work/work-title', '').strip()
        mvttl = e.findtext('movement-title', '').strip()
        composer = []
        lyricist = []
        credits = ''
        for creator in e.findall('identification/creator'):
            if creator.text:
                if creator.get('type') == 'composer':
                    composer += [line.strip() for line in creator.text.split('\n')]
                elif creator.get('type') in ('lyricist', 'transcriber'):
                    lyricist += [line.strip() for line in creator.text.split('\n')]
                else:
                    DBGPRT(f'creator.text={creator.text}')
                # fi
            # fi
        # rof
        for rights in e.findall('identification/rights'):
            if rights.text:
                lyricist += [line.strip() for line in rights.text.split('\n')]
            # fi
        # rof
        for credit in e.findall('credit'):
            cs = ''.join(e.text or '' for e in credit.findall('credit-words'))
            cs = re.sub(r'\s\n\n*', '\n', cs)
            credits += cs
        # rof
        title = '\n'.join(i.strip() for i in title.split('\n'))
        mvttl = '\n'.join(i.strip() for i in mvttl.split('\n'))
        credits = [i.strip() for i in credits.split('\n')]
        credits = filterCredits(0)
        credits = '\n'.join(credits)
        if title:
            title = 'title   %s\n' % title.replace('\n', '\ntitle   ')
        # fi
        if mvttl:
            title += 'title   %s\n' % mvttl.replace('\n', '\ntitle   ')
        # fi
        if credits:
            title += 'title   %s\n' % credits.replace('\n', '\ntitle   ')
        # fi
        if composer:
            title += 'title   %s\n' % composer.replace('\n', '\ntitle   ')
        # fi
        if lyricist:
            title += 'title   %s\n' % lyricist.replace('\n', '\ntitle   ')
        # fi
        if title:
            gcsOut.title = title[:-1]
        # fi
        s.isSib = 'Sibelius' in (e.findtext('identification/encoding/software') or '')
        if s.isSib:
            info('Sibelius MusicXMl is unreliable')
        # fi
    # End of mkTitle
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def doDefaults(s, e):
        d = e.find('defaults');
        if d == None:
            return;
        # fi
        mils = d.findtext('scaling/millimeters')    # mills == staff height (mm)
        tenths = d.findtext('scaling/tenths')       # staff height in tenths
        if not mils or not tenths:
            return
        # fi
        xmlScale = float(mils) / float(tenths) / 10 # tenths -> mm
        space = 10 * xmlScale                       # space between staff lines == 10 tenths
        gcsScale = space / 0.2117                   # 0.2117 cm = 6pt = space between staff lines for scale = 1.0
        gcsOut.pageFmt['scale'] = gcsScale
        eks = 2 * ['page-layout/'] + 4 * ['page-layout/page-margins/']
        eks = [a+b for a,b in zip(eks, 'page-height,page-width,left-margin,right-margin,top-margin,bottom-margin'.split(','))]
        for i in range(6):
            v = d.findtext(eks[i])
            k = gcsOut.pagekeys[i+1]                # pagekeys[0] == scale already done, skip it
            if not gcsOut.pageFmt[k] and v:
                try:
                    gcsOut.pageFmt[k] = float(v) * xmlScale     # -> cm
                except:
                    info('illegal value %s for xml element %s', (v, eks[i])); continue  # just skip illegal values
                # yrt
            # fi
        # rof
    # End of doDefaults
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def locStaffMap(s, part, maten):                # map voice to staff with majority voting
        vmap = {}                                   # {voice -> {staff -> n}} count occurrences of voice in staff
        s.vceInst = {}                              # {voice -> instrument id} for this part
        s.msc.vnums = {}                            # voice id's
        s.hasStems = {}                             # XML voice nums with at least one note with a stem(for tab key)
        s.stfMap, s.clefMap = {}, {}                # staff -> [voices], staff -> clef
        ns = part.findall('measure/note')
        for n in ns:                                # count staff allocations for all notes
            v = int(n.findtext('voice', '1'))
            if s.isSib:
                v += 100 * int(n.findtext('staff', '1'))    # repair bug in Sibelius
            # fi
            s.msc.vnums[v] = 1                      # collect all used voice id's in this part
            sn = int(n.findtext('staff', '1'))
            s.stfMap[sn] = []
            if v not in vmap:
                vmap[v] = {sn:1}
            else:
                d = vmap[v]                         # counter for voice v
                d[sn] = d.get(sn, 0) + 1            # ++ number of allocations for staff sn
            # fi
            x = n.find('instrument')
            if x != None:
                s.vceInst[v] = x.get('id')
            # fi
            x, noRest = n.findtext('stem'), n.find('rest') == None
            if noRest and (not x or x != 'none'):
                s.hasStems[v] = 1                   # XML voice v has at least one stem
            # fi
        # rof
        vks = list(vmap.keys())
        if s.isSib:
            vks.sort()
        # fi
        for v in vks:                               # choose staff with most allocations for each voice
            xs = [(n, sn) for sn, n in vmap[v].items()]
            xs.sort()
            stf = xs[-1][1]                         # the winner: staff with most notes of voice v
            s.stfMap[stf].append(v)
            s.vce2stf[v] = stf                      # reverse map
            s.curStf[v] = stf                       # current staff of XML voice v
        # rof
    # End of locStaffMap
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def addStaffMap(s, vvmap): # vvmap: xml voice number -> global gcs voice number
        part = [] # default: brace on staffs of one part
        for stf, voices in sorted(s.stfMap.items()):  # s.stfMap has xml staff and voice numbers
            locmap = [vvmap[iv] for iv in voices if iv in vvmap]
            nostem = [(iv not in s.hasStems) for iv in voices if iv in vvmap]   # same order as locmap
            if locmap:          # gcs voice number of staff stf
                part.append(locmap)
                clef = s.clefMap.get(stf, 'clef%%treble') # {xml staff number -> clef}
                for i, iv in enumerate(locmap):
                    clef_attr = ''
                    if clef.startswith('tab'):
                        if nostem[i] and 'nostems' not in clef:
                            clef_attr = ' nostems'
                        # fi
                        if s.diafret and 'diafret' not in clef:
                            clef_attr += ' diafret' # for all voices in the part
                        # fi
                    # fi
                    gcsOut.clefs[iv] = clef + clef_attr # add nostems when all notes of voice had no stem
                # rof
            # fi
        # rof
        s.gStfMap.append(part)
    # End of addStaffMap
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def addMidiMap(s, ip, vvmap):                   # map gcs voices to midi settings
        DBGPRT('NOTDONEYET - addMidiMap')
        instr = s.instMid[ip]                       # get the midi settings for this part
        if instr.values():
            defInstr = list(instr.values())[0]      # default settings = first instrument
        else:
            defInstr = s.midDflt                    # no instruments defined
        # fi
        xs = []
        for v, vgcs in vvmap.items():               # xml voice num, gcs voice num
            ks = sorted(s.drumNotes.items())
            ds = [(nt, step, midi, head) for (vd, nt), (step, midi, head) in ks if v == vd] # map perc notes
            id = s.vceInst.get(v, '')               # get the instrument-id for part with multiple instruments
            if id in instr:                         # id is defined as midi-instrument in part-list
                   xs.append((vgcs, instr[id] + ds))  # get midi settings for id 
            else:
                xs.append((vgcs, defInstr   + ds))  # only one instrument for this part
            # fi
        # rof
        xs.sort()                                   # put gcs voices in order
        s.midiMap.extend([midi for v, midi in xs])
        snaarmap = ['E','G','B','d', 'f', 'a', "c'", "e'"]
        diamap = '0,1-,1,1+,2,3,3,4,4,5,6,6+,7,8-,8,8+,9,10,10,11,11,12,13,13+,14'.split(',')
        for k in sorted(s.tabmap.keys()):           # add %%map's for all tab voices
            v, noot = k;
            snaar, fret = s.tabmap[k];
            if s.diafret:
                fret = diamap[int(fret)]
            # fi
            vgcs = vvmap[v]
            snaar = s.stafflines - int(snaar)
            xs = s.tabVceMap.get(vgcs, [])
            xs.append(f'%%map tab{vgcs} {noot} print={snaarmap[snaar]} heads=kop{fret}\n')
            s.tabVceMap[vgcs] = xs
            s.koppen[fret] = 1                      # collect noteheads for SVG defs
        # rof
    # End of addMidiMap
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    def parse(s, xmltxt):
        vvmapAll = {}                               # collect xml->gcs voice maps(vvmap) of all parts
        e = E.fromstring(xmltxt)
        s.mkTitle(e)
        s.doDefaults(e)
        partlist = s.doPartList(e)
        parts = e.findall('part')
        for ip, p in enumerate(parts):
            maten = p.findall('measure')
            s.locStaffMap(p, maten)                 # {voice -> staff} for this part
            s.drumNotes = {}                        # (xml voice, gcs note) -> (midi note, note head)
            s.clefOct = {}                          # xml staff number -> current clef-octave-change
            s.curClef = {}                          # xml staff number -> current gcs clef
            s.stemDir = {}                          # xml voice number -> current stem direction
            s.tabmap = {}                           # (xml voice, gcs note) -> (string, fret)
            s.diafret = 0                           # use diatonic fretting
            s.stafflines = 5
            s.msc.initVoices(newPart = 1)           # create all voices
            aantalHerhaald = 0                      # keep track of number of repititions
            herhaalMaat = 0                         # target measure of the repitition
            divisions = []                          # current value of <divisions> for each measure
            s.msr = Measure(ip)                     # various measure data
            while s.msr.ixm < len(maten):
                maat = maten[s.msr.ixm]
                herhaal, lbrk = 0, ''
                s.msr.reset()
                s.curalts = {}                      # passing accidentals are reset each measure
                es = list(maat)
                for i, e in enumerate(es):
                    if e.tag == 'note':
                        s.doNote(e)
                    elif e.tag == 'attributes':
                        s.doAttr(e)
                    elif e.tag == 'direction':
                        s.doDirection(e, i, es)
                    elif e.tag == 'sound':
                        s.doDirection(maat, i, es)  # sound element directly in measure!
                    elif e.tag == 'harmony':
                        s.doHarmony(e, i, es)
                    elif e.tag == 'barline':
                        herhaal = s.doBarline(e)
                    elif e.tag == 'backup':
                        dt = int(e.findtext('duration'))
                        if chkbug(dt, s.msr):
                            s.msc.incTime(-dt)
                        # fi
                    elif e.tag == 'forward':
                        dt = int(e.findtext('duration'))
                        if chkbug(dt, s.msr):
                            s.msc.incTime(dt)
                        # fi
                    elif e.tag == 'print':
                        lbrk = s.doPrint(e)
                    else:
                        DBGPRT(f'  Not processed e.tag={e.tag}')
                    # fi
                # rof
                s.msc.addBar(lbrk, s.msr)
                divisions.append(s.msr.divs)
                if herhaal == 1:
                    herhaalMaat = s.msr.ixm
                    s.msr.ixm += 1
                elif herhaal == 2:
                    if aantalHerhaald < 1:          # jump
                        s.msr.ixm = herhaalMaat
                        aantalHerhaald += 1
                    else:
                        aantalHerhaald = 0          # reset
                        s.msr.ixm += 1              # just continue
                    # fi
                else:
                    s.msr.ixm += 1                  # on to the next measure
                # fi
            # elihw
            for rv in s.repeat_str.values():        # close hanging measure-repeats without stop
                rv[0] = '[I:repeat %s %d]' % (rv[1], 1)
            # rof
            vvmap = s.msc.outVoices(divisions, ip, s.isSib)
            s.addStaffMap(vvmap)                    # update global staff map
            s.addMidiMap(ip, vvmap)
            vvmapAll.update(vvmap)
        # rof
        if vvmapAll:                                # skip output if no part has any notes
            gcsOut.mkHeader(s.gStfMap, partlist, s.midiMap, s.tabVceMap, s.koppen)
        else:
            info(f'nothing written, {gcsOut.fnmext} has no notes ...')
        # fi
    # End of parse
# End of class Parser

# ------------------------------------------------------------------
if __name__ == '__main__':
    from optparse import OptionParser
    from glob import glob
    from zipfile import ZipFile 

    ustr = '%prog [-h] [-p f] [-m 2] <file1> [<file2> ...]'
    parser = OptionParser(usage=ustr)
    options, args = parser.parse_args()
    if len(args) == 0:
        parser.error('no input file given')
    # fi
    if len(args) != 1:
        parser.error('Only one input file allowed.')
    # fi
    fnmext_list = glob(args[0])
    if len(fnmext_list) != 1:
        parser.error('Only one input file allowed.')
    # fi
    fnmext = fnmext_list[0]
    fnm, ext = os.path.splitext(fnmext)
    if ext.lower() not in ('.xml','.mxl','.musicxml'):
        parser.error(f'input file {fnmext}, should have extension .xml or .mxl')
    # fi
    if os.path.isdir(fnmext):
        parser.error(f'directory {fnmext}. Only files are accepted')
    # fi
    if ext.lower() == '.mxl':                       # extract .xml file from .mxl file
        z = ZipFile(fnmext)
        for n in z.namelist():                      # assume there is always an xml file in a mxl archive !!
            if (n[:4] != 'META') and (n[-4:].lower() == '.xml'):
                fobj = z.open(n)
                break                               # assume only one MusicXML file per archive
            # fi
        # rof
    else:
        fobj = open(fnmext, 'rb')                   # open regular xml file
    # fi

    gcsOut = GCSoutput(fnm + '.gcs')                # create global gcs output object
    psr = Parser()                                  # xml parser
    psr.parse(fobj.read())                          # parse file fobj and write gcs to <fnm>.gcs
    gcsOut.writeall()
# fi -- __main__

# ----------------------------------------------------------------------------
# End of xml2gcs.py
# ----------------------------------------------------------------------------
