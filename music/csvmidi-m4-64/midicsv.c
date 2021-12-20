#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "midifile.h"
#include "midio.h"
#include "getopt.h"

#define FALSE	0
#define TRUE	1

static int      verbose = FALSE;                /* Debug output */

/* ------------------------------------------------------------------------ */
/*  VLENGTH  --  Parse variable length item from in-memory track  */
static unsigned long vlength(unsigned char **trk, long *trklen)
{
    unsigned long   value;
    unsigned char   ch;
    unsigned char  *cp = *trk;

    trklen--;
    if ((value = *cp++) & 0x80)
    {
        value &= 0x7F;
        do
        {
            value = (value << 7) | ((ch = *cp++) & 0x7F);
            trklen--;
        } while (ch & 0x80);
    }
    *trk = cp;
    return value;
}

/* ------------------------------------------------------------------------ */
/* TEXTCSV -- Convert text field to CSV, quoting as necessary. */
static void textcsv(FILE *fo, const unsigned char *t, const int len)
{
    unsigned char   c;

    putc('"', fo);
    for (int i = 0; i < len; i++)
    {
        c = *t++;
        if (c < ' ' || (c > '~' && c <= 160))
        {
            putc('\\', fo);
            fprintf(fo, "%03o", c);
        }
        else
        {
            if (c == '"')
            {
                putc('"', fo);
            }
            else if (c == '\\')
            {
                putc('\\', fo);
            }
            putc(c, fo);
        }
    }
    putc('"', fo);
}

/* ------------------------------------------------------------------------ */
/*  TRACKCSV  --  Compile track into CSV written to fo.  */
static void trackcsv(FILE *fo, const int trackno, unsigned char *trk, long trklen, const int ppq)
{
    int             levt = 0;
    int		    evt;
    int 	    channel;
    int 	    note;
    int 	    vel;
    int 	    control;
    int 	    value;
    int 	    type;
    unsigned long   len;
    unsigned char  *titem;
    unsigned long   abstime = 0;                /* Absolute time in track */

    while (trklen > 0)
    {
        unsigned long   tlapse = vlength(&trk, &trklen);

        abstime += tlapse;

        fprintf(fo, "%d, %ld, ", trackno, abstime);

        /* Handle running status; if the next byte is a data byte,
         * reuse the last command seen in the track. */

        if (*trk & 0x80)
        {
            evt = *trk++;

            /* One subtlety: we only save channel voice messages
             * for running status.  System messages and file
             * meta-events (all of which are in the 0xF0-0xFF
             * range) are not saved, as it is possible to carry a
             * running status across them.  You may have never seen
             * this done in a MIDI file, but I have. */

            if ((evt & 0xF0) != 0xF0)
            {
                levt = evt;
            }
            trklen--;
        }
        else
        {
            evt = levt;
        }

        channel = evt & 0xF;
        /* Channel messages */
        switch (evt & 0xF0)
        {
            case NoteOff:                      /* Note off */
                if (trklen < 2)
                {
                    return;
                }
                trklen -= 2;
                note = *trk++;
                vel = *trk++;
                fprintf(fo, "Note_off_c, %d, %d, %d\n", channel, note, vel);
                continue;

            case NoteOn:                       /* Note on */
                if (trklen < 2)
                {
                    return;
                }
                trklen -= 2;
                note = *trk++;
                vel = *trk++;
                /*  A note on with a velocity of 0 is actually a note
                 * off.  We do not translate it to a Note_off record
                 * in order to preserve the original structure of the
                 * MIDI file.   */
                fprintf(fo, "Note_on_c, %d, %d, %d\n", channel, note, vel);
                continue;

            case PolyphonicKeyPressure:        /* Aftertouch */
                if (trklen < 2)
                {
                    return;
                }
                trklen -= 2;
                note = *trk++;
                vel = *trk++;
                fprintf(fo, "Poly_aftertouch_c, %d, %d, %d\n", channel, note, vel);
                continue;

            case ControlChange:                /* Control change */
                if (trklen < 2)
                {
                    return;
                }
                trklen -= 2;
                control = *trk++;
                value = *trk++;
                fprintf(fo, "Control_c, %d, %d, %d\n", channel, control, value);
                continue;

            case ProgramChange:                /* Program change */
                if (trklen < 1)
                {
                    return;
                }
                trklen--;
                note = *trk++;
                fprintf(fo, "Program_c, %d, %d\n", channel, note);
                continue;

            case ChannelPressure:              /* Channel pressure (aftertouch) */
                if (trklen < 1)
                {
                    return;
                }
                trklen--;
                vel = *trk++;
                fprintf(fo, "Channel_aftertouch_c, %d, %d\n", channel, vel);
                continue;

            case PitchBend:                    /* Pitch bend */
                if (trklen < 1)
                {
                    return;
                }
                trklen--;
                value = *trk++;
                value = value | ((*trk++) << 7);
                fprintf(fo, "Pitch_bend_c, %d, %d\n", channel, value);
                continue;

            default:
                break;
        }

        switch (evt)
        {
	    /* System exclusive messages */
            case SystemExclusive:
            case SystemExclusivePacket:
                len = vlength(&trk, &trklen);
                fprintf(fo, "System_exclusive%s, %lu", evt == SystemExclusivePacket ? "_packet" : "", len);
                {
                    for (unsigned long i = 0; i < len; i++)
                    {
                        fprintf(fo, ", %d", *trk++);
                    }
                    fprintf(fo, "\n");
                }
                break;

	    /* File meta-events */
            case FileMetaEvent:
                if (trklen < 2)
                {
                    return;
                }
                trklen -= 2;
                type = *trk++;
                len = vlength(&trk, &trklen);
                titem = trk;
                trk += len;
                trklen -= len;

                switch (type)
                {
                    case SequenceNumberMetaEvent:
                        fprintf(fo, "Sequence_number, %d\n", (titem[0] << 8) | titem[1]);
                        break;

                    case TextMetaEvent:
                        fputs("Text_t, ", fo);
                        textcsv(fo, titem, len);
                        putc('\n', fo);
                        break;

                    case CopyrightMetaEvent:
                        fputs("Copyright_t, ", fo);
                        textcsv(fo, titem, len);
                        putc('\n', fo);
                        break;

                    case TrackTitleMetaEvent:
                        fputs("Title_t, ", fo);
                        textcsv(fo, titem, len);
                        putc('\n', fo);
                        break;

                    case TrackInstrumentNameMetaEvent:
                        fputs("Instrument_name_t, ", fo);
                        textcsv(fo, titem, len);
                        putc('\n', fo);
                        break;

                    case LyricMetaEvent:
                        fputs("Lyric_t, ", fo);
                        textcsv(fo, titem, len);
                        putc('\n', fo);
                        break;

                    case MarkerMetaEvent:
                        fputs("Marker_t, ", fo);
                        textcsv(fo, titem, len);
                        putc('\n', fo);
                        break;

                    case CuePointMetaEvent:
                        fputs("Cue_point_t, ", fo);
                        textcsv(fo, titem, len);
                        putc('\n', fo);
                        break;

                    case ChannelPrefixMetaEvent:
                        fprintf(fo, "Channel_prefix, %d\n", titem[0]);
                        break;

                    case PortMetaEvent:
                        fprintf(fo, "MIDI_port, %d\n", titem[0]);
                        break;

                    case EndTrackMetaEvent:
                        fprintf(fo, "End_track\n");
                        trklen = -1;
                        break;

                    case SetTempoMetaEvent:
                        fprintf(fo, "Tempo, %d\n", (titem[0] << 16) |
                                (titem[1] << 8) | titem[2]);
                        break;

                    case SMPTEOffsetMetaEvent:
                        fprintf(fo, "SMPTE_offset, %d, %d, %d, %d, %d\n",
                                titem[0], titem[1], titem[2], titem[3], titem[4]);
                        break;

                    case TimeSignatureMetaEvent:
                        fprintf(fo, "Time_signature, %d, %d, %d, %d\n",
                                titem[0], titem[1], titem[2], titem[3]);
                        break;

                    case KeySignatureMetaEvent:
                        fprintf(fo, "Key_signature, %d, \"%s\"\n", ((signed char)titem[0]),
                                titem[1] ? "minor" : "major");
                        break;

                    case SequencerSpecificMetaEvent:
                        fprintf(fo, "Sequencer_specific, %lu", len);
                        {
                            for (unsigned long i = 0; i < len; i++)
                            {
                                fprintf(fo, ", %d", titem[i]);
                            }
                            fprintf(fo, "\n");
                        }
                        break;

                    default:
                        if (verbose)
                        {
                            fprintf(stderr, "Unknown meta event type 0x%02X, %ld bytes of data.\n", type, len);
                        }
                        fprintf(fo, "Unknown_meta_event, %d, %lu", type, len);
                        {
                            for (unsigned long i = 0; i < len; i++)
                            {
                                fprintf(fo, ", %d", titem[i]);
                            }
                            fprintf(fo, "\n");
                        }
                        break;
                }
                break;

            default:
                if (verbose)
                {
                    fprintf(stderr, "Unknown event type 0x%02X.\n", evt);
                }
                fprintf(fo, "Unknown_event, %02Xx\n", evt);
                break;
        }
    }
}

/* ------------------------------------------------------------------------ */
/*  Main program.  */
int main(int argc, char *argv[])
{
    struct mhead    mh;
    FILE           *fp;
    FILE           *fo;
    long            track1;
    int             i;
    int             track1l;


    if (argc == 3)
    {
fprintf(stderr, "argv[2] = '%s'\n", argv[2]);
	fo = fopen(argv[2], "w");
	if (fo == NULL)
	{
	    fprintf(stderr, "midicsv: Unable to to open MIDI input file %s\n", argv[2]);
	    exit(2);
	}
    }
    else
    {
	fo = stdout;
    }
    if (argc >= 2)
    {
fprintf(stderr, "argv[1] = '%s'\n", argv[1]);
	fp = fopen(argv[1], "rb");
	if (fp == NULL)
	{
	    fprintf(stderr, "midicsv: Unable to to create CSV output file %s\n", argv[1]);
	    exit(2);
	}
    }
    else
    {
	fp = stdin;
    }

    /* Read and validate header */

    readMidiFileHeader(fp, &mh);
    if (memcmp(mh.chunktype, "MThd", sizeof mh.chunktype) != 0)
    {
        fprintf(stderr, "%s is not a Standard MIDI File.\n", argv[1]);
        exit(2);
    }
    if (verbose)
    {
        fprintf(stderr, "Format %d MIDI file.  %d tracks, %d ticks per quarter note.\n",
                mh.format, mh.ntrks, mh.division);
    }

    /*  Output header  */

    fprintf(fo, "0, 0, Header, %d, %d, %d\n", mh.format, mh.ntrks, mh.division);

    /*  Process tracks */

    for (i = 0; i < mh.ntrks; i++)
    {
        struct mtrack   mt;
        unsigned char  *trk;

        if (i == 0)
        {
            track1 = ftell(fp);
        }

        readMidiTrackHeader(fp, &mt);
        if (memcmp(mt.chunktype, "MTrk", sizeof mt.chunktype) != 0)
        {
            fprintf(stderr, "Track %d header is invalid.\n", i + 1);
            exit(2);
        }

        if (verbose)
        {
            fprintf(stderr, "Track %d: length %ld.\n", i + 1, mt.length);
        }
        fprintf(fo, "%d, 0, Start_track\n", i + 1);

        trk = (unsigned char *)malloc(mt.length);
        if (trk == NULL)
        {
            fprintf(stderr, "midicsv: Cannot allocate %ld bytes for track.\n", mt.length);
            exit(2);
        }

        fread((char *)trk, (int)mt.length, 1, fp);
        if (i == 0)
        {
            track1l = (int)(ftell(fp) - track1);
        }

        trackcsv(fo, i + 1, trk, mt.length, mh.division);
        free(trk);
    }
    fprintf(fo, "0, 0, End_of_file\n");
    exit(0);
}

/* ------------------------------------------------------------------------ */
