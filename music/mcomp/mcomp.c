/*
   vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
 */
/* ------------------------------------------------------------------------ */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
/* ------------------------------------------------------------------------ */
#include <termios.h>
static struct termios original_termios;
static char     original_termios_gotten = 0;
/* ------------------------------------------------------------------------ */
// #define DEBUG    fprintf(stderr, "%s:%u:%s\n", __FILE__,__LINE__,__func__);
/* ------------------------------------------------------------------------ */
#define NORETURN __attribute__((noreturn))
#define UNUSED   __attribute__((unused)) /*@unused@*/
/* ------------------------------------------------------------------------ */
#define TRUE            1
#define FALSE           0
/* ------------------------------------------------------------------------ */
/* Default note length - input to MIDI. */
#define SHORTNOTEPRINT  4
/* The 4 is for quarternotes in MIDI (for tempo). */
#define XUNIT           (int)(division * 4.0 / SHORTNOTEPRINT)
/* Number of midi ticks per bar/measure. */
#define TICKSPERBAR     (int)(4.0 * division * meter_n / meter_d)
/* ------------------------------------------------------------------------ */
#define MINSTUFF    654321
/* ------------------------------------------------------------------------ */
/* Can cope with up to 64 track MIDI files */
#define MAXTRACKS   64
/* ------------------------------------------------------------------------ */
/* not static, because might not be used in debugging. */
char *nt[] = {
    "r  ",
    "ooo", "ooo", "ooo", "ooo", "ooo", "ooo", "ooo", "ooo", "ooo", "ooo",
    "ooo", "ooo", "ooo", "ooo", "ooo", "ooo", "ooo", "ooo", "ooo", "ooo",
    "ooo", "ooo", "ooo", "ooo", "0c ", "0d-", "0d ", "0e-", "0e ", "0f ",
    "0g-", "0g ", "0a-", "0a ", "0b-", "0b ", "1c ", "1d-", "1d ", "1e-",
    "1e ", "1f ", "1g-", "1g ", "1a-", "1a ", "1b-", "1b ", "2c ", "2d-",
    "2d ", "2e-", "2e ", "2f ", "2g-", "2g ", "2a-", "2a ", "2b-", "2b ",
    "3c ", "3d-", "3d ", "3e-", "3e ", "3f ", "3g-", "3g ", "3a-", "3a ",
    "3b-", "3b ", "4c ", "4d-", "4d ", "4e-", "4e ", "4f ", "4g-", "4g ",
    "4a-", "4a ", "4b-", "4b ", "5c ", "5d-", "5d ", "5e-", "5e ", "5f ",
    "5g-", "5g ", "5a-", "5a ", "5b-", "5b ", "6c ", "6d-", "6d ", "6e ",
    "6e-", "6f ", "6g-", "6g ", "6a-", "6a ", "6b-", "6b ", "7c ", "7d-",
    "7d ", "7e-", "7e ", "7f ", "7g-", "7g ", "7a-", "7a ", "7b-", "7b ",
    "8c ", "ooo", "ooo", "ooo", "ooo", "ooo", "ooo", "ooo", "ooo", "ooo", "ooo",
};
/* ------------------------------------------------------------------------ */
NORETURN static void exit_perror(const char *str);
/* ------------------------------------------------------------------------ */
static void restore_original_termios(void)
{
    if (original_termios_gotten == 0)
    {
        return;
    }
    original_termios.c_lflag |= ICANON;
    original_termios.c_lflag |= ECHO;
    original_termios_gotten = 0;        /* Make sure no fatal loop. */
    if (tcsetattr(0, TCSADRAIN, &original_termios) < 0)
    {
        exit_perror("tcsetattr ICANON");
    }
}   /* End of restore_original_termios */

/* ------------------------------------------------------------------------ */
NORETURN static void mc_exit(int value)
{
    restore_original_termios();
    exit(value);
}   /* End of mc_exit */

/* ------------------------------------------------------------------------ */
NORETURN static void exit_perror(const char *str)
{
    perror(str);
    mc_exit(1);
}   /* End of exit_perror */

/* ------------------------------------------------------------------------ */
static void get_original_termios(void)
{
    if (isatty(0))
    {
        if (tcgetattr(0, &original_termios) < 0)
        {
            perror("tcgetattr()");
            return;
        }
        original_termios_gotten = 1;

        original_termios.c_lflag &= ~ICANON;
        original_termios.c_lflag &= ~ECHO;
        original_termios.c_cc[VMIN] = 1;
        original_termios.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &original_termios) < 0)
        {
            exit_perror("tcsetattr ICANON");
        }
    }
}   /* End of get_original_termios */

/* ------------------------------------------------------------------------ */
#if 0
static char get_single_ch(void)
{
    char            buf = 0;
    int             i;

    fprintf(stderr, "> ");
    i = read(0, &buf, 1);
    fprintf(stderr, "\n");
    if (i < 0)
    {
        exit_perror("read()");
    }
    return (buf);
}   /* End of get_single_ch */
#endif  /* 0 */

/* ------------------------------------------------------------------------ */
#define MAXFRAC     7
#define MAXFRAC3    0
// #define MAXFRAC3    1
// #define MAXFRAC3    2
// #define MAXFRAC3    3
// #define MAXFRAC3    4
// #define MAXFRAC3    5
// #define MAXFRAC3    6
// #define MAXFRAC3    7
static int fractionsdotted[MAXFRAC];            /* Ticks for 2^x notes dotted. */
static int fractions[MAXFRAC];                  /* Ticks for 2^x notes. */
static int fractionsthirddotted[MAXFRAC3];      /* Ticks for 2^x notes dotted divide by 3. */
static int fractionsthird[MAXFRAC3];            /* Ticks for 2^x notes divide by 3. */
#define EXTRA   (fractions[MAXFRAC - 1] / 2)    /* +/- EXTRA to align notes. */
/* ------------------------------------------------------------------------ */
struct time_signature
{
    struct time_signature *tsnext;              /* Next in link. */
    long            timesig_time;               /* time that signature changes. */
    int             numer;
    int             denom;
};
static struct time_signature *time_signature_start = NULL;
static struct time_signature *time_signature_last = NULL;
static int          meter_n = 4;                /* meter numerator - Default. */
static int          meter_d = 4;                /* meter denominator - Default. */
static int          keysig = 0;                 /* -6 to 6 sharps */
/* ------------------------------------------------------------------------ */
struct tempo_signature
{
    struct tempo_signature *temponext;  /* Next in link. */
    long            tempo_time;         /* time that signature changes. */
    long            tempo;
};
static struct tempo_signature *tempo_signature_start = NULL;
static struct tempo_signature *tempo_signature_last = NULL;
static long     tempo = 500000;         /* Default tempo 120 beats/minute for a quarternote. */
/*                                         This is given in microseconds ( divide by 1,000,000 ). */
/* ------------------------------------------------------------------------ */
struct bars_location
{
    struct bars_location *barsnext;     /* Next in link. */
    long            bar_time;           /* time that signature changes. */
    int             bar_number;
};
static struct bars_location *bars_location_start = NULL;
static struct bars_location *bars_location_last = NULL;
static long     bar_number = 0;
/* ------------------------------------------------------------------------ */
#define MC_MAXVOICE 64
struct voice_notes
{
    struct voice_notes *vcnext;         /* Next in link. */
    struct voice_notes *previous;       /* Next in link. */
    long            vc_start_time;      /* time that key starts. */
    long            vc_stop_time;        /* time that key stops. */
    char           *note;
    char           *lth;
    int             next_tied;
    int             volume;             /* Note volume. */
};
static struct voice_notes *voice_notes_start[MC_MAXVOICE];
static struct voice_notes *voice_notes_last[MC_MAXVOICE];
static int          voicecount;
static int          voicesbeyondthis;  /* For chords ... */
/* ------------------------------------------------------------------------ */
#define to32bit(a, b, c, d)                     \
    ((a & 0xff) << 24) | ((b & 0xff) << 16) | ((c & 0xff) << 8) | (d & 0xff);
/* ------------------------------------------------------------------------ */
struct note
{
    struct note    *notenext;
    int             pitch;              /* Note pitch. */
    int             chan;               /* Note channel for track (track number). */
    int             vel;                /* Note velocity (volume). */
    long            n_start_time;       /* When note starts playing. */
    long            n_stop_time;        /* MIDI says note starts - if +10 from last, then chord. */
    int             playnum;            /* Amount left to play commented. */
} *track[MAXTRACKS] = { NULL };
static int          trackcount = 0;
static long         maxtracktime = 0;
/* ------------------------------------------------------------------------ */
static FILE           *F;
static int             division;
static int             trans[256];
static int             back[256];
static char            atog[256];
static char            symbol[256];
static int             key[12];
/* ------------------------------------------------------------------------ */
/* malloc with error checking */
static void           *checkmalloc(int bytes)
{
    void           *p;

    p = (int *)malloc(bytes);
    if (p == NULL)
    {
        fprintf(stderr, "Out of memory error - cannot malloc!\n");
        mc_exit(1);
    }
    return (p);
}   /* End of checkmalloc */

/* ------------------------------------------------------------------------ */
/* MIDI note stops. */
static void stop_note(int p_trackno, long currtime, struct note **playing, int p, int ch)
{
    struct note    *i;
    struct note    *j;
    struct note    *prev;
    int             found;

    found = 0;
    for (i = *playing, prev = NULL; i != NULL; i = i->notenext)
    {
        if ((i->pitch == p) && (i->chan == ch))
        {
            found = 1;
            break;
        }
        prev = i;                              /* previous for playing. */
    }
    if (found == 0)
    {
        fprintf(stderr, "Note terminated when not on - pitch %d - currtime=%ld track=%d", p, currtime, p_trackno);
        return;
    }

    i->playnum = currtime - i->n_start_time + 1;
    /* Remove note from list. If at the end of a list. */
    if (i->notenext == NULL)
    {
        if (prev == NULL)                      /* If no previous pointer. */
        {
            *playing = NULL;                 /* Nothing in list any more. */
        }
        else
        {
            prev->notenext = NULL;
        }
    }
    else
    {
        if (prev != NULL)
        {
            prev->notenext = i->notenext;
        }
        else
        {
            *playing = i->notenext;
        }
    }
    /* i is not in playing anymore. */
    j = track[p_trackno];
    i->n_stop_time = currtime;
    i->notenext = NULL;
    /* Tack onto the end of track. */
    if (track[p_trackno] == NULL)
    {
        track[p_trackno] = i;
    }
    else
    {
        for (struct note *o = track[p_trackno]; o != NULL; o = o->notenext)
        {
            if (o->notenext == NULL)
            {
                o->notenext = i;
                break;
            }
        }
    }
}   /* End of stop_note */

/* ------------------------------------------------------------------------ */
/* MIDI note starts. */
static void start_note(long currtime, struct note **playing, int p, int ch, int v)
{
    struct note    *o;
    struct note    *newnote = (struct note *)checkmalloc(sizeof(struct note));
    newnote->notenext = NULL;
    newnote->pitch = p;
    newnote->chan = ch;
    newnote->vel = v;
    newnote->n_start_time = currtime;
    newnote->n_stop_time = 0;

    o = *playing;
    newnote->notenext = o;
    *playing = newnote;
}   /* End of start_note */

/* ------------------------------------------------------------------------ */
/* Add structure for text. */
static void chanmessage(int p_trackno, long currtime, struct note **playing, int status, int c1, int c2)
{
    int             chan = status & 0xf;

    switch (status & 0xf0)
    {
        case 0x80:
            stop_note(p_trackno, currtime, playing, c1, chan);
            break;
        case 0x90:
            if (c2 != 0)
            {
                start_note(currtime, playing, c1, chan, c2);
            }
            else
            {
                stop_note(p_trackno, currtime, playing, c1, chan);
            }
            break;
        case 0xa0:      /* polyphonic key pressure (after-touch) */
        case 0xb0:      /* Control change - pedals/levels - channel mode messages */
            break;
        case 0xc0:      /* Program change - */
            break;
        case 0xd0:      /* Channel pressure - after-touch different than 0xb0 */
        case 0xe0:      /* Pitch Wheel Change - */
fprintf(stderr, " chanmessage track=%d status=0x%02x c1=0x%02x c2=0x%02x\n", p_trackno, status, c1, c2);
            break;
    }
}   /* End of chanmessage */

/* ------------------------------------------------------------------------ */
/* Read a single character and abort on EOF. */
static int egetc(long *ToBeRead)
{
    int             c = getc(F);

    if (c == EOF)
    {
        fprintf(stderr, "Error: premature EOF!\n");
        mc_exit(1);
    }
    (*ToBeRead)--;
    return (c);
}   /* End of egetc */

/* ------------------------------------------------------------------------ */
static void metaevent(long currtime, char *m, int type)
{
    switch (type)
    {
        case 0x51:                              /* Set tempo */
            /* Set tempo (in microseconds per MIDI quarter note)  */
            /*           (24ths of a microsecond per MIDI clock) */
            {
                struct tempo_signature *ts;
                long p_tempo = to32bit(0, m[0], m[1], m[2]);
                ts = checkmalloc(sizeof(*ts));
                ts->temponext = NULL;
                ts->tempo_time = currtime;
                ts->tempo = p_tempo;
                if (tempo_signature_start == NULL)
                {
                    tempo_signature_start = ts;
                }
                else
                {
                    tempo_signature_last->temponext = ts;
                }
                tempo_signature_last = ts;
            }
            break;
        case 0x58:
            {
                struct time_signature *ts;
                int dd = m[1];
                int denom = 1;
                int numer = m[0];
                while (dd-- > 0)
                {
                    denom *= 2;
                }
                for (ts = time_signature_start; ts != NULL; ts = ts->tsnext)
                {
                    if (ts->timesig_time == currtime)
                    {
                        /* Replace first with latest one seen. */
                        if (ts->numer != numer || ts->denom != denom)   /* No difference. */
                        {
                            if (currtime == 1)
                            {
                                ts->numer = numer;
                                ts->denom = denom;
                            }
                            else
                            {
                                fprintf(stderr, "%ld meter changing multiple times in same place: from(%d/%d) to (%d/%d)\n", ts->timesig_time, ts->numer, ts->denom, numer, denom);
                            }
                        }
                        return;
                    }
                    /* if after this and before next, insert into place. */
                    if (ts->tsnext != NULL && currtime > ts->timesig_time && currtime < ts->tsnext->timesig_time)
                    {
                        struct time_signature *nts = checkmalloc(sizeof(*ts));
                        nts->tsnext = ts->tsnext;
                        nts->timesig_time = currtime;
                        nts->numer = numer;
                        nts->denom = denom;
                        ts->tsnext = nts;
                        return;
                    }
                }
                /* Put at end of list. */
                ts = checkmalloc(sizeof(*ts));
                ts->tsnext = NULL;
                ts->timesig_time = currtime;
                ts->numer = numer;
                ts->denom = denom;
                if (time_signature_start == NULL)
                {
                    time_signature_start = ts;
                }
                else
                {
                    time_signature_last->tsnext = ts;
                }
                time_signature_last = ts;
            }
            break;
        case 0x59:
            keysig = m[0];
            break;
        case 0x00:
        case 0x01:                              /* Text event */
        case 0x02:                              /* Copyright notice */
        case 0x03:                              /* Sequence/Track name */
        case 0x04:                              /* Instrument name */
        case 0x05:                              /* Lyric */
        case 0x06:                              /* Marker */
        case 0x07:                              /* Cue point */
        case 0x08:
        case 0x09:
        case 0x0a:
        case 0x0b:
        case 0x0c:
        case 0x0d:
        case 0x0e:
        case 0x0f:
        case 0x20:                              /* MIDI Channel Prefix */
        case 0x2f:                              /* End of Track */
        case 0x54:
        case 0x7f:
        default:
            break;
    }
}   /* End of metaevent */

/* ------------------------------------------------------------------------ */
/* readvarinum - read a varying-length number, and return the number of characters it took. */
static long readvarinum(long *ToBeRead)
{
    long            value;
    int             c;

    c = egetc(ToBeRead);
    value = c;
    if (c & 0x80)
    {
        value = c & 0x7f;
        do
        {
            c = egetc(ToBeRead);
            value = (value << 7) + (c & 0x7f);
        } while (c & 0x80);
    }
    return (value);
}   /* End of readvarinum */

/* ------------------------------------------------------------------------ */
/* read through the "MThd" or "MTrk" header string */
static int readmt(char *s)
{
    unsigned int    n;
    int             c;

    for (n = 0; n < strlen(s); n++)
    {
        c = getc(F);
        if (c == EOF)
        {
            break;
        }
        if (c != s[n])
        {
            fprintf(stderr, "Error: expecting %s\n", s);
            mc_exit(1);
        }
    }
    return (c);
}   /* End of readmt */

/* ------------------------------------------------------------------------ */
static int read16bit(long *ToBeRead)
{
    int             c[2];
    int             value;

    c[0] = egetc(ToBeRead);
    c[1] = egetc(ToBeRead);
    value = ((c[0] & 0xff) << 8) + (c[1] & 0xff);
    return (value);
}   /* End of read16bit */

/* ------------------------------------------------------------------------ */
static long read32bit(long *ToBeRead)
{
    int             c[4];
    long            value;

    c[0] = egetc(ToBeRead);
    c[1] = egetc(ToBeRead);
    c[2] = egetc(ToBeRead);
    c[3] = egetc(ToBeRead);
    value = to32bit(c[0], c[1], c[2], c[3]);
    return (value);
}   /* End of read32bit */

/* ------------------------------------------------------------------------ */
static void readheader(long *ToBeRead)
{
    int             ntrks;
    int             format;

    if (readmt("MThd") == EOF)
    {
        return;
    }
    *ToBeRead = read32bit(ToBeRead);
    format = read16bit(ToBeRead);           /* Ignore format. */
    ntrks = read16bit(ToBeRead);            /* Ignore, reading tracks tells us this. */
    division = read16bit(ToBeRead);
    /* Flush any extra stuff, in case the length of header is not 6. */
    while (*ToBeRead > 0)
    {
        (void)egetc(ToBeRead);              /* Toss bytes. */
    }
}   /* End of readheader */

/* ------------------------------------------------------------------------ */
/* Read all tracks. */
static void readtracks(void)
{
    /* This array is indexed by the high half of a status byte.  It's */
    /* value is either the number of bytes needed (1 or 2) for a channel */
    /* message, or 0 (meaning it's not  a channel message). */
    static const int chantype[] = {
        0, 0, 0, 0, 0, 0, 0, 0,                     /* 0x00 through 0x70 */
        2, 2, 2, 2, 1, 1, 2, 0                      /* 0x80 through 0xf0 */
    };
    long            lookfor;
    int             c;
    int             c1;
    int             type;
    int             running;                        /* 1 when running status used */
    int             status;                         /* status value (e.g. 0x90==note-on) */
    int             needed;
    long            varinum;
    int             trackno;
#define MSGINCREMENT 128
    char            msgbuf[MSGINCREMENT];           /* message buffer */
    int             msgindex;                       /* index into msgbuf */
    int             t;
    long            ToBeRead = 0;                   /* Number of bytes to be read ... */
    long            currtime = 0L;                  /* current time in midi-time units */
    struct note    *playing;

    readheader(&ToBeRead);

    for (trackno = 0; ; trackno++)
    {
        playing = NULL;
        running = 0;
        status = 0;
        if (readmt("MTrk") == EOF)
        {
            return;
        }

        ToBeRead = read32bit(&ToBeRead);

        currtime = 0;
        track[trackno] = NULL;
        while (ToBeRead > 0)
        {
            varinum = readvarinum(&ToBeRead);
            currtime += varinum;                    /* midi time */
            maxtracktime = MAX(maxtracktime, currtime);
            c = egetc(&ToBeRead);
            if ((c & 0x80) == 0)
            {                                       /* running status? */
                if (status == 0)
                {
                    fprintf(stderr, "Error: unexpected running status\n");
                    mc_exit(1);
                }
                running = 1;
            }
            else
            {
                status = c;
                running = 0;
            }
            needed = chantype[(status >> 4) & 0xf];
            if (needed)
            {                                       /* ie. is it a channel message? */
                if (running)
                {
                    c1 = c;
                }
                else
                {
                    c1 = egetc(&ToBeRead);
                }
                if (needed > 1)
                {
                    t = egetc(&ToBeRead);
                }
                else
                {
                    t = 0;
                }
                chanmessage(trackno, currtime, &playing, status, c1, t);
                continue;
            }

            switch (c)
            {
                case 0xff:                          /* meta event */
                    type = egetc(&ToBeRead);
                    varinum = readvarinum(&ToBeRead);
                    lookfor = ToBeRead - varinum;
                    msgindex = 0;
                    while (ToBeRead > lookfor)
                    {
                        t = egetc(&ToBeRead);
                        if (msgindex < MSGINCREMENT-1)  /* Allow terminating zero.*/
                        {
                            msgbuf[msgindex++] = t;
                        }
                    }
                    msgbuf[msgindex] = '\0';
                    metaevent(currtime & ~7, msgbuf, type);
                    break;
                case 0xf0:                          /* start of system exclusive */
fprintf(stderr, "system exclusive\n");
                    varinum = readvarinum(&ToBeRead);
                    lookfor = ToBeRead - varinum;
                    while (ToBeRead > lookfor)
                    {
                        egetc(&ToBeRead);           /* Toss bytes. */
                    }
                    break;
                case 0xf7:                          /* sysex continuation or arbitrary stuff */
fprintf(stderr, "sysex or arbitary stuff\n");
                    varinum = readvarinum(&ToBeRead);
                    lookfor = ToBeRead - varinum;
                    while (ToBeRead > lookfor)
                    {
                        egetc(&ToBeRead);           /* Toss bytes. */
                    }
                    break;
                default:
                    fprintf(stderr, "Error: unexpected byte: 0x%02x\n", c);
                    mc_exit(1);
            }
        }

        /* check for unfinished notes */
        if (playing != NULL)
        {
            fprintf(stderr, "Error in MIDI file - notes still on at end of track %d!\n", trackno);
        }
        trackcount = trackcount + 1;
        if (trackcount >= MAXTRACKS)
        {
            fprintf(stderr, "Too many MIDI tracks -- more than %d!\n", MAXTRACKS);
            mc_exit(1);
        }
    }
}   /* End of readtracks */

/* ------------------------------------------------------------------------ */
/* The compare function. */
static int cmp(struct note *a, struct note *b)
{
    if (a->n_start_time == b->n_start_time)
    {
        if (a->n_stop_time == b->n_stop_time)
        {
            return(a->pitch - b->pitch);
        }
        return(a->n_stop_time - b->n_stop_time);
    }
    return (a->n_start_time - b->n_start_time);
}   /* End of cmp */

/* ------------------------------------------------------------------------ */
/* Sorted list returned. */
static struct note *list_sort(struct note *list)
{
    struct note    *p;
    struct note    *q;
    struct note    *e;
    struct note    *tail;
    int             nmerges;
    int             psize;
    int             qsize;
    int             i;
    int             insize = 1;

    if (!list)
    {
        return (NULL);
    }
    while (1)
    {
        p = list;
        list = NULL;
        tail = NULL;
        nmerges = 0;
        while (p)
        {
            nmerges++;
            q = p;
            psize = 0;
            for (i = 0; i < insize; i++)
            {
                psize++;
                q = q->notenext;
                if (!q)
                {
                    break;
                }
            }
            qsize = insize;
            while (psize > 0 || (qsize > 0 && q))
            {
                if (psize == 0)
                {
                    e = q;
                    q = q->notenext;
                    qsize--;
                }
                else if (qsize == 0 || !q)
                {
                    e = p;
                    p = p->notenext;
                    psize--;
                }
                else if (cmp(p, q) <= 0)
                {
                    e = p;
                    p = p->notenext;
                    psize--;
                }
                else
                {
                    e = q;
                    q = q->notenext;
                    qsize--;
                }
                if (tail)
                {
                    tail->notenext = e;
                }
                else
                {
                    list = e;
                }
                tail = e;
            }
            p = q;
        }
        tail->notenext = NULL;
        if (nmerges <= 1)
        {
            return (list);
        }
        insize *= 2;
    }
}   /* End of list_sort */

/* ------------------------------------------------------------------------ */
static void free_note(struct note *n)
{
    memset(n, -1, sizeof(struct note));
}   /* End of free_note */

/* ------------------------------------------------------------------------ */
/* Try to fix slurs. */
/* Start time is assumed okay. */
/* For slur, delta = n_stop_time - n_start_time is not a note fraction - know something fishy.          */
/* If delta > 256 (quarter note), is the delta about 12, then make it a quarter note.               */
/* If delta > 384 (quarter dotted note), is the delta about 19, then make it a quarter dotted note. */
/* If delta > 128 (8th note), is the abs(delta) about 3, then make it an 8th note.                  */

#define round_down(a, b)    { int q; q = a / b; a = q * b; }

static void fix_possible_slur(struct note *o, int k)
{
// fractionsdotted     [0] = 1536  f=  1d       30
// fractions           [0] = 1024  f=  1        28
// fractionsdotted     [1] =  768  f=  2d       25
// fractions           [1] =  512  f=  2        20
// fractionsthirddotted[0] =  512  f=  1d/3             20
// fractionsdotted     [2] =  384  f=  4d       20
// fractionsthird      [0] =  341  f=  1/3              16
// fractions           [2] =  256  f=  4        12
// fractionsthirddotted[1] =  256  f=  2d/3             12
// fractionsdotted     [3] =  192  f=  8d       10
// fractionsthird      [1] =  170  f=  2/3              8
// fractions           [3] =  128  f=  8        6
// fractionsthirddotted[2] =  128  f=  4d/3             5
// fractionsdotted     [4] =   96  f= 16d       5
// fractionsthird      [2] =   85  f=  4/3              4
// fractions           [4] =   64  f= 16        3
// fractionsthirddotted[3] =   64  f=  8d/3             3
// fractionsdotted     [5] =   48  f= 32d       3
// fractionsthird      [3] =   42  f=  8/3          2
// fractions           [5] =   32  f= 32        2
// fractionsthirddotted[4] =   32  f= 16d/3         2
// fractionsdotted     [6] =   24  f= 64d       2
// fractionsthird      [4] =   21  f= 16/3          1
// fractions           [6] =   16  f= 64        1
// fractionsthirddotted[5] =   16  f= 32d/3         1
// fractionsthird      [5] =   10  f= 32/3          1
// fractionsthirddotted[6] =    8  f= 64d/3         1
// fractionsthird      [6] =    5  f= 64/3          1

    static const int stop_round_dotted[MAXFRAC] =
    {
        30,     /*  1d */
        25,     /*  2d */
        20,     /*  4d */
        10,     /*  8d */
         5,     /* 16d */
         3,     /* 32d */
         2      /* 64d */
    };
    static const int stop_round[MAXFRAC] =
    {
        28,     /*  1 */
        20,     /*  2 */
        12,     /*  4 */
         6,     /*  8 */
         3,     /* 16 */
         2,     /* 32 */
         1      /* 64 */
    };
    static const int stop_third_dotted[MAXFRAC] =
    {
        20,     /*  1d  512 */   
        12,     /*  2d  256 */
         5,     /*  4d  128 */
         3,     /*  8d   64 */
         2,     /* 16d   32 */
         1,     /* 32d   16 */
         1      /* 64d    8 */
    };
    static const int stop_third[MAXFRAC] =
    {
        16,     /*  1  341 */
         8,     /*  2  170 */
         4,     /*  4   85 */
         2,     /*  8   42 */
         1,     /* 16   21 */
         1,     /* 32   10 */
         1      /* 64    5 */
    };
    int dt_o;
    int dt_f;
    int miner = MINSTUFF;
    int mines = MINSTUFF;
    int minet = 0;

    round_down(o->n_start_time, k);
    dt_o = o->n_stop_time - o->n_start_time;

    /* Find smallest difference from a predefined note length. */
    for (int v = 0; v < MAXFRAC; v++)
    {
        dt_f = dt_o - fractionsdotted[v];
        if (abs(dt_f) < abs(miner))
        {
            miner = dt_f;
            mines = v;
            minet = 1;
        }
    }
    for (int v = 0; v < MAXFRAC; v++)
    {
        dt_f = dt_o - fractions[v];
        if (abs(dt_f) < abs(miner))
        {
            miner = dt_f;
            mines = v;
            minet = 0;
        }
    }
    for (int v = 0; v < MAXFRAC3; v++)
    {
        dt_f = dt_o - fractionsthirddotted[v];
        if (abs(dt_f) < abs(miner))
        {
            miner = dt_f;
            mines = v;
            minet = 3;
        }
    }
    for (int v = 0; v < MAXFRAC3; v++)
    {
        dt_f = dt_o - fractionsthird[v];
        if (abs(dt_f) < abs(miner))
        {
            miner = dt_f;
            mines = v;
            minet = 2;
        }
    }

    /* We have the smallest difference to a "known" expected note length -- up or down. */
    if (minet == 0)                         /* If note without "dot". */
    {
        if (abs(miner) <= stop_round[mines])      /* Reasonable amount to round down... */
        {
            o->n_stop_time = o->n_stop_time - miner;
        }
        else
        {
            round_down(o->n_stop_time, k);
        }
    }
    else if (minet == 1)
    {
        if (abs(miner) <= stop_round_dotted[mines])  /* Reasonable amount to round down... */
        {
            o->n_stop_time = o->n_stop_time - miner;
        }
        else
        {
            round_down(o->n_stop_time, k);
        }
    }
    else if (minet == 2)
    {
        if (abs(miner) <= stop_third[mines])  /* Reasonable amount to round down... */
        {
            o->n_stop_time = o->n_stop_time - miner;
        }
        else
        {
            // fprintf(stderr, "rounding excessive... miner=%d stop_third[%d]=%d\n", miner, mines, stop_third[mines]);
            round_down(o->n_stop_time, k);
        }
    }
    else /* if (minet == 3) */
    {
        if (abs(miner) <= stop_third_dotted[mines])  /* Reasonable amount to round down... */
        {
            o->n_stop_time = o->n_stop_time - miner;
        }
        else
        {
            // fprintf(stderr, "rounding excessive... miner=%d stop_third_dotted[%d]=%d\n", miner, mines, stop_third_dotted[mines]);
            round_down(o->n_stop_time, k);
        }
    }

// fprintf(stderr, "    %s dt_o=%d mines=%d miner=%d minet=%d frac=%d\n", __func__, dt_o,mines,miner,minet, (minet==0)?fractions[mines]:fractionsdotted[mines]);
// fprintf(stderr, "               after  o:pitch=%d n_start_time=%ld,n_stop_time=%ld delta=%ld\n", o->pitch, o->n_start_time,o->n_stop_time, o->n_stop_time - o->n_start_time);
}   /* End of fix_possible_slur */

/* ------------------------------------------------------------------------ */

static void fix_note_start_stop_time(void)
{
    int f = fractions[MAXFRAC - 1];
    int k = f / 2;

    if ((f % 1) != 0)
    {
        /* Does this matter? Should we round up or truncate down? */
        fprintf(stderr, "fix_note_start_stop_time 64th note is not even. %d\n", f);
        return;
    }
    if (k < 0)
    {
        return;
    }
    /* ------------------------------------------------------------------------ */
// fprintf(stderr, "fix_note_start_stop_time: f=%d  k=%d\n", f, k);
    for (int t = 0; t < MAXTRACKS; t++)
    {
        struct note *o;
        struct note *prev = NULL;

        o = track[t];
        while (o != NULL)
        {
            /* Fix start_time and stop_time, round down or up. */
            fix_possible_slur(o, k);     // m4 doesn't think so -- 2021-04-01.

            /* Delete zero length notes. */
            if (o->n_stop_time - o->n_start_time == 0)
            {
                if (prev == 0)                      /* If first note in track. */
                {
                    track[t] = o->notenext;         /* Track will now start with second note. */
                    free_note(o);
                    o = track[t];
                }
                else
                {
                    prev->notenext = o->notenext;   /* deleted. */
                    free_note(o);
                    o = prev->notenext;
                }
                continue;
            }
            prev = o;
            o = o->notenext;
        }
    }
    /* Fix time signatures. */
    for (struct time_signature *ts = time_signature_start; ts != NULL; ts= ts->tsnext)
    {
        /* Fix time, round down. */
        long j = (ts->timesig_time + k) / f;
        ts->timesig_time = j * f;
    }
    /* Fix tempo signatures. */
    for (struct tempo_signature *tp = tempo_signature_start; tp != NULL; tp= tp->temponext)
    {
        /* Fix time, round down. */
        long j = (tp->tempo_time + k) / f;
        tp->tempo_time = j * f;
    }
}   /* End of fix_note_start_stop_time */

/* ------------------------------------------------------------------------ */
static void set_starting_rest_times(void)
{

    struct note    *i;
    int             p_trackno;
    long            when;
    struct note    *newnote;
    struct note    *nextnote;
    struct note    *j;

    for (p_trackno = 0; p_trackno < trackcount; p_trackno++)
    {
        i = track[p_trackno];
        if (i == NULL)
        {
            continue;
        }
        else if (i->n_start_time > 1)
        {
            when = i->n_start_time;
            newnote = (struct note *)checkmalloc(sizeof(struct note));
            newnote->notenext = track[p_trackno];
            newnote->pitch = -1;            /* rest */
            newnote->chan = 0;              /* Not needed here. */
            newnote->vel = 0;               /* No sound volume. */
            newnote->n_start_time = 0;
            newnote->n_stop_time = when;
            track[p_trackno] = newnote;
        }
        for (j = track[p_trackno]; j != NULL && j->notenext != NULL; j = j->notenext)
        {
            when = j->n_stop_time;
            if (when < j->notenext->n_start_time)
            {
                nextnote = j->notenext;
                newnote = (struct note *)checkmalloc(sizeof(struct note));
                newnote->notenext = j->notenext;
                newnote->pitch = -1;            /* rest */
                newnote->chan = 0;              /* Not needed here. */
                newnote->vel = 0;               /* No sound volume. */
                newnote->n_start_time = when;
                newnote->n_stop_time = j->notenext->n_start_time;
                j->notenext = newnote;
            }
        }
        /* Now put a rest at the end of the track, if needed. */
        if (j != NULL)                          /* last note in track. */
        {
            when = j->n_stop_time;
            if ((when + 1) < maxtracktime)
            {
                newnote = (struct note *)checkmalloc(sizeof(struct note));
                newnote->notenext = NULL;
                newnote->pitch = -1;            /* rest */
                newnote->chan = 0;              /* Not needed here. */
                newnote->vel = 0;               /* No sound volume. */
                newnote->n_start_time = when;
                newnote->n_stop_time = maxtracktime;
                j->notenext = newnote;
            }
        }
    }
}   /* End of set_starting_rest_times */

/* ------------------------------------------------------------------------ */
/* set up variables related to key signature */
static void setupkey(int p_sharps, int printit)
{
    char            sharp[13];
    char            flat[13];
    char            shsymbol[13];
    char            flsymbol[13];
    int             j;
    int             t;
    int             issharp;
    int             minkey;

    minkey = (p_sharps + 12) % 12;
    if (minkey % 2 != 0)
    {
        minkey = (minkey + 6) % 12;
    }
    strcpy(sharp, "ccddeffggaab");
    strcpy(shsymbol, "n+n+nn+n+n+n");
    if (p_sharps == 6)
    {
        sharp[6] = 'e';
        shsymbol[6] = '+';
    }
    strcpy(flat, "cddeefggaabb");
    strcpy(flsymbol, "n-n-nn-n-n-n");
    /* Print out key */
    if (p_sharps >= 0)
    {
        if (printit)
        {
            if (p_sharps == 6)
            {
                printf("key    f+");
            }
            else
            {
                printf("key     %c", sharp[minkey]);
            }
        }
        issharp = 1;
    }
    else
    {
        if (printit)
        {
            if (p_sharps == -1)
            {
                printf("key     %c", flat[minkey]);
            }
            else
            {
                printf("key     %c-", flat[minkey]);
            }
        }
        issharp = 0;
    }
    if (printit)
    {
        if (p_sharps >= 0)
        {
            printf("        $$ %d sharps\n", p_sharps);
        }
        else
        {
            printf("        $$ %d flats\n", -p_sharps);
        }
    }
    key[(minkey + 1) % 12] = 1;
    key[(minkey + 3) % 12] = 1;
    key[(minkey + 6) % 12] = 1;
    key[(minkey + 8) % 12] = 1;
    key[(minkey + 10) % 12] = 1;
    for (j = 0; j < 256; j++)
    {
        t = j % 12;
        if (issharp)
        {
            atog[j] = sharp[t];
            symbol[j] = shsymbol[t];
        }
        else
        {
            atog[j] = flat[t];
            symbol[j] = flsymbol[t];
        }
        trans[j] = 7 * (j / 12) + ((int)atog[j] - 'a');
        if (key[t] == 0)
        {
            back[trans[j]] = j;
        }
    }
}   /* End of setupkey */

/* ------------------------------------------------------------------------ */
static void addtochord(struct note *p, struct note **chords)
{
    struct note *place;
    struct note *prev = NULL;

    struct note *newnote = (struct note *)checkmalloc(sizeof(struct note));
    newnote->notenext = NULL;
    newnote->pitch = p->pitch;
    newnote->chan = p->chan;
    newnote->vel = p->vel;
    newnote->n_start_time = p->n_start_time;
    newnote->n_stop_time = p->n_stop_time;
    newnote->playnum = p->n_stop_time - p->n_start_time;

    if (*chords == NULL)
    {
        *chords = newnote;
        return;
    }
    for (place = *chords; place != NULL; place = place->notenext)
    {
        if (place->pitch > newnote->pitch)
        {
            continue;
        }
        prev = place;
        place = place->notenext;
        break;
    }
    if (prev == NULL)                               /* Insert at beginning. */
    {
        newnote->notenext = (*chords)->notenext;    /* Insert before. */
        *chords = newnote;                          /* Start of chord is new. */
        return;
    }
    newnote->notenext = place;                      /* Insert in middle or end. */
    prev->notenext = newnote;
}   /* End of addtochord */

/* ------------------------------------------------------------------------ */
/* Work out a tick step which can be expressed as a musical time. */
static int valid_note_length(int n)
{
//--     if ((n % 2) == 1)
//--     {
//--         n = n + 1;                          /* Round up to multiple of 2. */
//--     }
#if MAXFRAC != 7
#error MAXFRAC must be 7 in valid_note_length.
#endif
    if (n >= fractionsdotted     [0]) { return (fractionsdotted     [0]); }
    if (n >= fractions           [0]) { return (fractions           [0]); }
    if (n >= fractionsdotted     [1]) { return (fractionsdotted     [1]); }
    if (n >= fractions           [1]) { return (fractions           [1]); }

#if MAXFRAC3 > 0
    if (n >= fractionsthirddotted[0]) { return (fractionsthirddotted[0]); }
#endif /* MAXFRAC3 > 0 */
    if (n >= fractionsdotted     [2]) { return (fractionsdotted     [2]); }
#if MAXFRAC3 > 0
    if (n >= fractionsthird      [0]) { return (fractionsthird[0]); }
#endif /* MAXFRAC3 > 0 */
    if (n >= fractions           [2]) { return (fractions           [2]); }

#if MAXFRAC3 > 1
    if (n >= fractionsthirddotted[1]) { return (fractionsthirddotted[1]); }
#endif /* MAXFRAC3 > 1 */
    if (n >= fractionsdotted     [3]) { return (fractionsdotted     [3]); }
#if MAXFRAC3 > 1
    if (n >= fractionsthird      [1]) { return (fractionsthird[1]); }
#endif /* MAXFRAC3 > 1 */
    if (n >= fractions           [3]) { return (fractions           [3]); }

#if MAXFRAC3 > 2
    if (n >= fractionsthirddotted[2]) { return (fractionsthirddotted[2]); }
#endif /* MAXFRAC3 > 2 */
    if (n >= fractionsdotted     [4]) { return (fractionsdotted     [4]); }
#if MAXFRAC3 > 2
    if (n >= fractionsthird      [2]) { return (fractionsthird[2]); }
#endif /* MAXFRAC3 > 2 */
    if (n >= fractions           [4]) { return (fractions           [4]); }

#if MAXFRAC3 > 3
    if (n >= fractionsthirddotted[3]) { return (fractionsthirddotted[3]); }
#endif /* MAXFRAC3 > 3 */
    if (n >= fractionsdotted     [5]) { return (fractionsdotted     [5]); }
#if MAXFRAC3 > 3
    if (n >= fractionsthird      [3]) { return (fractionsthird[3]); }
#endif /* MAXFRAC3 > 3 */
    if (n >= fractions           [5]) { return (fractions           [5]); }

#if MAXFRAC3 > 4
    if (n >= fractionsthirddotted[4]) { return (fractionsthirddotted[4]); }
#endif /* MAXFRAC3 > 4 */
    if (n >= fractionsdotted     [6]) { return (fractionsdotted     [6]); }
#if MAXFRAC3 > 4
    if (n >= fractionsthird      [4]) { return (fractionsthird[4]); }
#endif /* MAXFRAC3 > 4 */
    if (n >= fractions           [6]) { return (fractions           [6]); }

#if MAXFRAC3 > 5
    if (n >= fractionsthirddotted[5]) { return (fractionsthirddotted[5]); }
    if (n >= fractionsthird      [5]) { return (fractionsthird[5]); }
#endif  /* MAXFRAC3 > 5 */

#if MAXFRAC3 > 6
    if (n >= fractionsthirddotted[6]) { return (fractionsthirddotted[6]); }
    if (n >= fractionsthird      [6]) { return (fractionsthird[6]); }
#endif  /* MAXFRAC3 > 6 */

fprintf(stderr, "valid_note_length strange note length n=%d\n", n);
    return (-n);
}   /* End of valid_note_length */

/* ------------------------------------------------------------------------ */
static struct note *removefromchord(struct note *i, struct note **chords)
{
    struct note  *prev = NULL;
    struct note  *ret = NULL;

    for (struct note *l = *chords; l != NULL; l = l->notenext)
    {
        if (l == i)
        {
            break;
        }
        prev = l;
    }
    ret = i->notenext;
    /* remove note from list */
    if (prev == NULL)
    {
        *chords = ret;
    }
    else
    {
        prev->notenext = i->notenext;
    }
    free_note(i);
    return (ret);
}   /* End of removefromchord */

/* ------------------------------------------------------------------------ */
/* adjust note lengths for all notes in the chord */
static void advancechord(struct note **chords, int len)
{
    struct note  *p;

    p = *chords;
    while (p != NULL)
    {
        if (p->playnum <= len)
        {
            if (p->playnum < len)
            {
                fprintf(stderr, "Error - note too short!'\n");
                mc_exit(1);
            }
            /* remove note */
            p = removefromchord(p, chords);
            continue;
        }
        /* shorten note */
        p->playnum = p->playnum - len;
        p = p->notenext;
    }
}   /* End of advancechord */

/* ------------------------------------------------------------------------ */
/* find the first note in the chord to terminate */
static int findshortest(struct note **chords)
{
    int             min;
    int             v;
    struct note    *p;

    min = MINSTUFF;
    for (p = *chords; p != NULL; p = p->notenext)
    {
        v = p->playnum;
        if (v < min)
        {
            min = v;
        }
    }
    return (min);
}   /* End of findshortest */

/* ------------------------------------------------------------------------ */
static struct voice_notes *get_new_voice_note(void)
{
    struct voice_notes *vn;

    vn = checkmalloc(sizeof(*vn));
    vn->vcnext = NULL;
    vn->previous = NULL;
    vn->vc_start_time = 0;
    vn->vc_stop_time = 0;
    vn->note = NULL;
    vn->lth = NULL;
    vn->next_tied = 0;
    vn->volume = 0;
    return(vn);
}   /* End of get_new_voice_notes */

/* ------------------------------------------------------------------------ */
static void add_note_to_voice(int p_voice, long p_now, long p_stop, char *p_txt)
{
    struct voice_notes *vn = get_new_voice_note();

    vn->previous = voice_notes_last[p_voice];
    vn->vc_start_time = p_now;
    vn->vc_stop_time = p_stop;
    vn->note = p_txt;
    if (voice_notes_start[p_voice] == NULL)
    {
        voice_notes_start[p_voice] = vn;
    }
    else
    {
        voice_notes_last[p_voice]->vcnext = vn;
    }
    voice_notes_last[p_voice] = vn;
}   /* End of add_note_to_voice */

/* ------------------------------------------------------------------------ */
static void printnotelength(struct voice_notes *p_vn, int a, int p_volume)
{
    int n;
    char buf[20];

    p_vn->volume = p_volume;
    n = 1;
    for (int j = 0; j < MAXFRAC; j++)                   /* Find legal lengths. */
    {
        if (a == fractionsdotted[j])
        {
            snprintf(buf, 20, "%dd", n);
            p_vn->lth = strdup(buf);
            return;
        }
        n = n * 2;
    }
    n = 1;
    for (int j = 0; j < MAXFRAC; j++)                   /* Find legal lengths. */
    {
        if (a == fractions[j])
        {
            snprintf(buf, 20, "%d", n);
            p_vn->lth = strdup(buf);
            return;
        }
        n = n * 2;
    }
    n = 1;
    for (int j = 0; j < MAXFRAC3; j++)                   /* Find legal lengths. */
    {
        if (a == fractionsthirddotted[j])
        {
            snprintf(buf, 20, "(1.5/(%d*3.0))", n);
            p_vn->lth = strdup(buf);
            return;
        }
        n = n * 2;
    }
    n = 1;
    for (int j = 0; j < MAXFRAC3; j++)                   /* Find legal lengths. */
    {
        if (a == fractionsthird[j])
        {
            snprintf(buf, 20, "(1.0/(%d*3.0))", n);
            p_vn->lth = strdup(buf);
            return;
        }
        n = n * 2;
    }

    snprintf(buf, 20, "?%d?", a);
    p_vn->lth = strdup(buf);
    return;
}   /* End of printnotelength */

/* ------------------------------------------------------------------------ */
/* convert numerical value to abc pitch */
static void printpitch(int p_voice, long p_now, long p_stop, struct note *j)
{
    int             p;
    int             po;
    int             octave;
    char            buf[20];

    p = j->pitch;
    if (p == -1)
    {
        add_note_to_voice(p_voice, p_now, p_stop, "r");
        return;
    }
    octave = (p / 12) - 2;              /* Offset so 3c = middle c. */
    po = p % 12;

    if ((back[trans[p]] != p) || (key[po] == 1))
    {
        snprintf(buf, 20, "%d%c%c", octave, atog[p], symbol[po]);
        back[trans[p]] = p;             /* pitch is not normal for key. (i.e. +/-/n) */
    }
    else
    {
        snprintf(buf, 20, "%d%c", octave, atog[p]);
    }
    add_note_to_voice(p_voice, p_now, p_stop, strdup(buf));
}   /* End of printpitch */

/* ------------------------------------------------------------------------ */
/* Print out the current chord. Any notes that haven't finished at the end of */
/* the chord are tied into the next chord. */
static void printchord(struct note **chords, int p_voice, long p_now, int len)
{
    struct note  *i;
    int j;

    i = *chords;
    if (i == NULL)
    {
        /* no notes in chord */
        add_note_to_voice(p_voice, p_now, p_now + len, "r");
        printnotelength(voice_notes_last[p_voice], len, -1);          /* Volume -1 for rest, ignore. */
        return;
    }
    if (i->notenext == NULL)
    {
        /* only one note in chord */
        printpitch(p_voice, p_now, p_now + len, i);
        printnotelength(voice_notes_last[p_voice], len, i->vel);      /* Volume of note. */
        if (len < i->playnum && i->pitch != -1)
        {
            if (i->notenext != NULL && i->pitch == i->notenext->pitch)
            {
                /* Tie to next note. */
                voice_notes_last[p_voice]->next_tied = 1;
            }
        }
        return;
    }
    j = 0;
    for ( ; i != NULL; i = i->notenext)
    {
        printpitch(p_voice + j, p_now, p_now + len, i);
        printnotelength(voice_notes_last[p_voice + j], len, i->vel);  /* Volume of note. */
        if (len < i->playnum && i->pitch != -1)
        {
            /* Tie to next note. */
            voice_notes_last[p_voice + j]->next_tied = 1;
        }
        j++;
    }
    if (voicesbeyondthis < (j - 1))
    {
        voicesbeyondthis = j - 1;
    }
    return;
}   /* End of printchord */

/* ------------------------------------------------------------------------ */
static void printtr(int p_trackno, int tickspernotes, int barcount, long now,
                    struct note *i, char *s, int print_note)
{
    fprintf(stderr, "%d t/n=%4d bar=%d now=%5ld %s\n", p_trackno, tickspernotes, barcount, now, s);
    if (print_note == 1 && i != NULL)
    {
        fprintf(stderr, "   n=%3s p=%2d c=%1d v=%2d st=%5ld sp=%5ld\n", nt[i->pitch+1], i->pitch, i->chan,
        i->vel, i->n_start_time, i->n_stop_time);
    }
}   /* End of printtr */

/* ------------------------------------------------------------------------ */
static void printtrack(int p_trackno)
{
    struct note    *i;
    int             step = MINSTUFF;
    int             tickspernotes;
    long            now = 0;
    struct note    *chords = NULL;
    int             nl;
    struct bars_location *bars = bars_location_start;
    int             barcount = 0;

    struct time_signature *ts = time_signature_start;
    meter_n = 4;                                    /* meter numerator */
    meter_d = 4;                                    /* meter denominator */
    /* Get the first time signature (last in list for this time). */
    while (ts != NULL && ts->timesig_time <= now)
    {
        meter_n = ts->numer;
        meter_d = ts->denom;
        ts = ts->tsnext;
    }

    i = track[p_trackno];
    tickspernotes = 0;
    while (now < maxtracktime)
    {
        if (tickspernotes == 0)
        {
            if (bars != NULL && bars->bar_time <= now)
            {
                if (bars->bar_number != (barcount+1))
                {
                    fprintf(stderr, "Bars messed up in printtrack - expect to be on %d and linked list gives us %d\n", barcount+1, bars->bar_number);
                    mc_exit(1);
                }
                bars = bars->barsnext;
                barcount = barcount + 1;
            }
            else
            {
                fprintf(stderr, "bars->bar_time(%ld) <= now(%ld)\n", bars->bar_time, now);
                fprintf(stderr, "Beyond the last measure, but no bars?\n");
                printtr(p_trackno, tickspernotes, barcount, now, i, "bars not correct", 0);
                mc_exit(1);
            }
            /* Get the first time signature (last in list for this time). */
            while (ts != NULL && ts->timesig_time <= now)
            {
                meter_n = ts->numer;
                meter_d = ts->denom;
                ts = ts->tsnext;
            }
            /* Set from meter_n and meter_d, etc. */
            tickspernotes = TICKSPERBAR;
        }

        /* Determine which note(s) to play, or if "nothing" (i.e. should be
           rests -- fix that elsewhere. Must check next note starttime, to add
           to chord, or pull from chord. */

        /* Add notes to chord here. */
        while (i != NULL && now >= i->n_start_time)
        {
            struct note *nn;

            /* add notes to chord */
            addtochord(i, &chords);
            nn = i;
            i = i->notenext;
            if (i != NULL && now <= i->n_start_time)
            {
                continue;
            }
            advancechord(&chords, 0);                   /* get rid of any zero length notes */
        }

        step = findshortest(&chords);
        if (i != NULL && (now + step) > i->n_start_time)
        {
            step = i->n_start_time - now;               /* In case chord overlap. */
        }
        if (step > tickspernotes)                       /* Must not be longer than "bar". */
        {
            step = tickspernotes;
        }
        if (step > 1)                                   /* This tosses off-by-1 mistakes. */
        {
            nl = valid_note_length(step);
            if (nl < 0)
            {
                fprintf(stderr, "printtrack #not valid_note_length? step=%d nl=%d\n", step, nl);
            }
            nl = abs(nl);                               /* Undo valid_note_length error return. */
        }
        else
        {
            fprintf(stderr, "step(%d) <= 0 ... ??? \n", step);
            mc_exit(1);
        }
        if (nl == 0)
        {
            fprintf(stderr, "Advancing by 0 in printtrack!\n");
            mc_exit(1);
        }
        printchord(&chords, voicecount, now, nl);       /* NOTDONEYET - choose voice. */
        advancechord(&chords, step);
        tickspernotes = tickspernotes - step;
        now = now + step;
    }
}   /* End of printtrack */

/* ======================================================================== */
/* This routine converts midi times in ticks into seconds. The
 * else statement is needed because the formula is different for tracks
 * based on notes and tracks based on SMPTE times. */
static void print_tempo(long ticks, long p_tempo)
{
    float          smpte_format;
    float          smpte_resolution;
    float          t;
    int            f;

    if (division > 0)
    {
        t = p_tempo / 1000000.0;            /* tempo in seconds per quarter note. */
        f = 60.0 / t;                       /* beats for a quarternote per mintue. */
    }
    else
    {
        /* The following is wrong. Until I hit it, ignore it. */
fprintf(stderr, "Error in %s:%u:%s -- NOTDONEYET\n", __FILE__,__LINE__,__func__);
        smpte_format = (division >> 8) & 0xff;
        smpte_resolution = division & 0xff;
        f = ticks / (smpte_format * smpte_resolution);
    }
    printf("tempo   %d,%d\n",(int)(f * (SHORTNOTEPRINT / 4.0)), SHORTNOTEPRINT);
}   /* End of print_tempo */

/* ------------------------------------------------------------------------ */
#if 0
static void printallmc(char *s)
{
    fprintf(stderr, "printallmc - %s\n", s);
    for (int i = 1; i <= voicecount; i++)
    {
     for (struct voice_notes *running = voice_notes_start[i]; running != NULL; running = running->vcnext)
     {
      fprintf(stderr, "%d running->note=%3s lth=%3s  start=%ld stop=%ld\n", i, running->note, running->lth, running->vc_start_time, running->vc_stop_time);
     }
    }
}   /* End of printallmc */
#endif /* 0 */

/* ------------------------------------------------------------------------ */
/* Got to create text rest(s) and lenth(s) -- before 'running'. */
static void put_in_rests_before(int i, struct voice_notes *running, long from)
{
    long till;
    struct bars_location *bars;

    if (running == NULL)
    {
        return;
    }
    from = running->vc_start_time - from;
    till = running->vc_start_time;
    for (bars = bars_location_start; bars != NULL; bars = bars->barsnext)
    {
        if (bars->barsnext == NULL)
        {
            break;
        }
        if (from >= bars->barsnext->bar_time)
        {
            continue;
        }
        break;
    }
    /* At this point, bars points to the time for the bar before (or on) the insert point. */
    for (; bars != NULL; bars = bars->barsnext)
    {
        long ntlth;
        long n;
        long st;
        if (till <= bars->bar_time)     
        {
            break;                              /* If done. */
        }
        if (bars->barsnext != NULL)
        {
            ntlth = bars->barsnext->bar_time;   /* If in middle of bars. */
        }
        else
        {
            ntlth = maxtracktime;               /* If at end of song. */
        }
        if (ntlth > till)                       /* If bar (or end of song) > till, use till. */
        {
            ntlth = till;
        }
        if (ntlth == bars->bar_time)            /* Should have been handled with break above. */
        {
            return;
        }
        /* ntlth = end of rest, bars->bar_time = start of rest. */
        if (from > bars->bar_time)
        {
            st = from;                          /* Long note goes from from through bars. */
        }
        else
        {
            st = bars->bar_time;
        }
        ntlth = ntlth - st;
        while (ntlth > 0)
        {
            struct voice_notes *vc;

            /* Work out a tick step which can be expressed as a musical time. */
            n = valid_note_length(ntlth);
            if (n <= 0)
            {
                return;
            }
            /* Put it before the passed in note. */
            vc = get_new_voice_note();
            vc->vcnext = running;                   /* new points to passed in one. */
            vc->previous = running->previous;       /* new points to previous passed in one. */
            if (running->previous == NULL)
            {
                voice_notes_start[i] = vc;
            }
            else
            {
                running->previous->vcnext = vc;
            }
            vc->vc_start_time = st;
            vc->vc_stop_time = st + n;
            vc->note = "r";
            printnotelength(vc, n, -1);
            /* next_tied and volume are set okay. */
            running->previous = vc;
            ntlth = ntlth - n;
            st = st + n;
        }
    }
}   /* End of put_in_rests_before */

/* ------------------------------------------------------------------------ */
/* Got to create text rest(s) and lenth(s). */
static void put_in_rests_after(struct voice_notes *running)
{
    long from;
    struct bars_location *bars;

    if (running == NULL)
    {
        return;
    }
    from = running->vc_stop_time;
    for (bars = bars_location_start; bars != NULL; bars = bars->barsnext)
    {
        if (bars->barsnext == NULL)
        {
            break;
        }
        if (from > bars->barsnext->bar_time)
        {
            continue;
        }
        break;
    }
    for (; bars != NULL; bars = bars->barsnext)
    {
        long ntlth;
        long n;
        long st;

        if (bars->barsnext != NULL && from > bars->barsnext->bar_time)
        {
            continue;
        }
        if (bars->barsnext != NULL)
        {
            ntlth = bars->barsnext->bar_time;       /* max time of note to fit in this bar. */
        }
        else
        {
            /* use maxtracktime for end of song. */
            ntlth = maxtracktime;                   /* note goes to end of max of tracks. */
        }
        /* ntlth = end of rest, bars->bar_time = start of rest. */
        if (from > bars->bar_time)
        {
            st = from;                              /* Long note goes from from through bars. */
        }
        else
        {
            st = bars->bar_time;
        }
        /* ntlth = end of rest, st = start of rest. */
        ntlth = ntlth - st;
        while (ntlth > 0)
        {
            struct voice_notes *vc;

            /* Work out a tick step which can be expressed as a musical time. */
            n = valid_note_length(ntlth);
            if (n <= 0)
            {
                return;
            }
            /* Put it before the passed in note. */
            vc = get_new_voice_note();
            vc->previous = running;                 /* new points to passed in one. */
            vc->vcnext = NULL;                      /* new points to previous passed in one. */
            vc->vc_start_time = st;
            vc->vc_stop_time = st + n;
            vc->note = "r";
            printnotelength(vc, n, -1);
            /* next_tied and volume are set okay. */
            running->vcnext = vc;
            running = vc;
            ntlth = ntlth - n;
            st = st + n;
        }
    }
}   /* End of put_in_rests_after */

/* ------------------------------------------------------------------------ */
/* If any voices in a bar without rests, add. */
static void put_in_mc_rests(void)
{
    for (int i = 1; i <= voicecount; i++)
    {
        struct voice_notes *running = voice_notes_start[i];
        if (running == NULL)            /* No notes for voice. */
        {
            continue;
        }
        /* Check for start of stream rests needed. */
        if (running->vc_start_time > 0)
        {
            put_in_rests_before(i, running, running->vc_start_time);
        }
        /* Check for middle of stream rests needed. */
        for (running = voice_notes_start[i]; running != NULL; running = running->vcnext)
        {
            if (running->vcnext == NULL)
            {
                /* Check for end of stream rests needed. */
                if (running->vc_stop_time < maxtracktime)
                {
                    put_in_rests_after(running);
                }
                break;
            }
            if ((running->vcnext->vc_start_time - running->vc_stop_time) < 1)
            {
                continue;
            }
            put_in_rests_before(i, running->vcnext, running->vcnext->vc_start_time - running->vc_stop_time);
        }
    }
}   /* End of put_in_mc_rests */

/* ------------------------------------------------------------------------ */
static void print_v_mc(void)
{
    long            f;
    struct voice_notes *running[MC_MAXVOICE];
    int             notdone;
    struct bars_location *b;
    struct time_signature *ts;
    struct tempo_signature *tpo;
    int             extra = 1;              /* There are off-by-ones everywhere, kludge around them. */
    int             last_volume[MC_MAXVOICE];
    int             volumechanged;
    long            last_bar_time = 0;

    for (int j = 1; j < MC_MAXVOICE; j++)
    {
        last_volume[j] = 0;                  /* Volume off to start with. */
    }
    setupkey(keysig, TRUE);
    meter_n = 4;
    meter_d = 4;
    printf("meter   %d/%d\n", meter_n, meter_d);
    print_tempo((long)XUNIT, tempo);
    printf("voice   %d\n", voicecount);

    /* Initialize for loop printing of minimalistics "time". */
    for (int j = 1; j <= voicecount; j++)
    {
        running[j] = voice_notes_start[j];
    }

    notdone = TRUE;
    b = bars_location_start;
    ts = time_signature_start;
    tpo = tempo_signature_start;
    while (notdone)
    {
        /* Find minimal time. */
        f = 8589934592;                     /* A big number. */
        notdone = FALSE;
        for (int j = 1; j <= voicecount; j++)
        {
            if (running[j])
            {
                notdone = TRUE;
                if (f > running[j]->vc_start_time)
                {
                    f = running[j]->vc_start_time;
                }
            }
        }
        if (!notdone)
        {
            break;
        }

        /* First any bars needed. */
        while (b != NULL && b->bar_time <= f + extra)
        {
            printf("measure %d      $$ time %ld  f=%ld   delta=%ld\n", b->bar_number, b->bar_time, f, b->bar_time - last_bar_time);
            last_bar_time = b->bar_time;
            if (b->bar_number == 1)
            {
                /* Print out center voice number. */
                printf("*          1");
                for (int j = 2; j <= voicecount; j++)
                {
                    printf("   ,  %2d", j);
                }
                printf("\n");
            }
            b = b->barsnext;
        }
        /* Second any time signatures needed. */
        while (ts != NULL && ts->timesig_time <= f + extra)
        {
            if (meter_n != ts->numer || meter_d != ts->denom)   /* Only print out if it changes. */
            {
                printf("meter   %d/%d\n", ts->numer, ts->denom);
                meter_n = ts->numer;
                meter_d = ts->denom;
            }
            ts = ts->tsnext;
        }
        /* Third any tempo's needed. */
        while (tpo != NULL && tpo->tempo_time <= f + extra)
        {
            if (tempo != tpo->tempo)                            /* Only print out if it changes. */
            {
                print_tempo((long)XUNIT, tpo->tempo);
                tempo = tpo->tempo;
            }
            tpo = tpo->temponext;
        }

        /* Volume change? */
        volumechanged = 0;
        for (int j = 1; j <= voicecount; j++)
        {
            if (running[j])
            {
                if (running[j]->vc_start_time <= f + extra)
                {
                    if (running[j]->volume > 0 && running[j]->volume != last_volume[j])
                    {
                        volumechanged = 1;
                        break;
                    }
                }
            }
        }
        /* If it changed, print out the volume and voice line. */
        if (volumechanged)
        {
            printf("volumes ");
            for (int j = 1; j <= voicecount; j++)
            {
                if (running[j])
                {
                    if (running[j]->vc_start_time <= f + extra)
                    {
                        if (running[j]->volume >= 0 && running[j]->volume != last_volume[j])
                        {
                            last_volume[j] = running[j]->volume;
                        }
                    }
                }
                if (j < voicecount)
                {
                    printf(" %3d   ,", last_volume[j]);
                }
                else
                {
                    printf(" %3d", last_volume[j]);
                }
            }
            printf("\n");
            printf("voice   %d\n", voicecount);
        }

        /* Print out all the same on one line sort of centered, others get blanks. */
        printf("        ");
        for (int j = 1; j <= voicecount; j++)
        {
            char t = ' ';
            {
                if (running[j])
                {
                    if (running[j]->vc_start_time <= f + extra)
                    {
                        if (running[j]->previous && running[j]->previous->next_tied)
                        {
                            if (strcmp(running[j]->note, "r") != 0)
                            {
                                t = 't';
                            }
                        }
                        if (j < voicecount)
                        {
                            printf("%-3s%-3s%c,", running[j]->note, running[j]->lth, t);
                        }
                        else
                        {
                            printf("%-3s%-3s%c", running[j]->note, running[j]->lth, t);
                        }
                        running[j] = running[j]->vcnext;
                    }
                    else
                    {
                        if (j < voicecount)
                        {
                            printf("       ,");
                        }
                    }
                }
                else
                {
                    if (j < voicecount)
                    {
                        printf("       ,");
                    }
                }
            }
        }
        printf("\n");
    }
}   /* End of print_v_mc */

/* ------------------------------------------------------------------------ */
#define V_LINE_NOTES    50
static void print_h_mc(void)
{
    long            f;
    struct voice_notes *running[MC_MAXVOICE];
    int             notdone;
    struct bars_location *b;
    struct time_signature *ts;
    struct tempo_signature *tpo;
    int             extra = 1;              /* There are off-by-ones everywhere, kludge around them. */
    int             last_volume[MC_MAXVOICE];
    int             volumechanged;
    long            last_bar_time = 0;
    char           v[MC_MAXVOICE][V_LINE_NOTES];

    for (int j = 1; j < MC_MAXVOICE; j++)
    {
        last_volume[j] = 0;                  /* Volume off to start with. */
        v[j][0] = '\0';
    }
    setupkey(keysig, TRUE);
    meter_n = 4;
    meter_d = 4;
    printf("meter   %d/%d\n", meter_n, meter_d);
    print_tempo((long)XUNIT, tempo);
    printf("voice   %d\n", voicecount);

    /* Initialize for loop printing of minimalistics "time". */
    for (int j = 1; j <= voicecount; j++)
    {
        running[j] = voice_notes_start[j];
    }

    notdone = TRUE;
    b = bars_location_start;
    ts = time_signature_start;
    tpo = tempo_signature_start;
    while (notdone)
    {
        /* Find minimal time. */
        f = 8589934592;                     /* A big number. */
        notdone = FALSE;
        for (int j = 1; j <= voicecount; j++)
        {
            if (running[j])
            {
                notdone = TRUE;
                if (f > running[j]->vc_start_time)
                {
                    f = running[j]->vc_start_time;
                }
            }
        }
        if (!notdone)
        {
            break;
        }

        /* First any bars needed. */
        while (b != NULL && b->bar_time <= f + extra)
        {
            for (int j = 1; j < MC_MAXVOICE; j++)
            {
                if (v[j][0] != '\0')
                {
                    printf("v%2d: %s\n", j, v[j]);
                    v[j][0] = '\0';
                }
            }
            printf("measure %d      $$ bar_time %ld  f=%ld   delta=%ld\n", b->bar_number, b->bar_time, f, b->bar_time - last_bar_time);
            last_bar_time = b->bar_time;
            b = b->barsnext;
        }
        /* Second any time signatures needed. */
        while (ts != NULL && ts->timesig_time <= f + extra)
        {
            for (int j = 1; j < MC_MAXVOICE; j++)
            {
                if (v[j][0] != '\0')
                {
                    printf("v%2d: %s\n", j, v[j]);
                    v[j][0] = '\0';
                }
            }
            if (meter_n != ts->numer || meter_d != ts->denom)   /* Only print out if it changes. */
            {
                printf("meter   %d/%d\n", ts->numer, ts->denom);
                meter_n = ts->numer;
                meter_d = ts->denom;
            }
            ts = ts->tsnext;
        }
        /* Third any tempo's needed. */
        while (tpo != NULL && tpo->tempo_time <= f + extra)
        {
            for (int j = 1; j < MC_MAXVOICE; j++)
            {
                if (v[j][0] != '\0')
                {
                    printf("v%2d: %s\n", j, v[j]);
                    v[j][0] = '\0';
                }
            }
            if (tempo != tpo->tempo)                            /* Only print out if it changes. */
            {
                print_tempo((long)XUNIT, tpo->tempo);
                tempo = tpo->tempo;
            }
            tpo = tpo->temponext;
        }

        /* Volume change? */
        volumechanged = 0;
        for (int j = 1; j <= voicecount; j++)
        {
            if (running[j])
            {
                if (running[j]->vc_start_time <= f + extra)
                {
                    if (running[j]->volume > 0 && running[j]->volume != last_volume[j])
                    {
                        volumechanged = 1;
                        break;
                    }
                }
            }
        }
        /* If it changed, print out the volume and voice line. */
        if (volumechanged)
        {
            for (int j = 1; j < MC_MAXVOICE; j++)
            {
                if (v[j][0] != '\0')
                {
                    printf("v%2d: %s\n", j, v[j]);
                    v[j][0] = '\0';
                }
            }
            printf("volumes ");
            for (int j = 1; j <= voicecount; j++)
            {
                if (running[j])
                {
                    if (running[j]->vc_start_time <= f + extra)
                    {
                        if (running[j]->volume >= 0 && running[j]->volume != last_volume[j])
                        {
                            last_volume[j] = running[j]->volume;
                        }
                    }
                }
                if (j < voicecount)
                {
                    printf("%2d,", last_volume[j]);
                }
                else
                {
                    printf("%2d", last_volume[j]);
                }
            }
            printf("\n");
            printf("voice   %d\n", voicecount);
        }

        /* Print out all the same on one line sort of centered, others get blanks. */
        for (int j = 1; j <= voicecount; j++)
        {
            char t = ' ';
            {
                if (running[j])
                {
                    if (running[j]->vc_start_time <= f + extra)
                    {
                        char s[V_LINE_NOTES];
                        if (running[j]->previous && running[j]->previous->next_tied)
                        {
                            if (strcmp(running[j]->note, "r") != 0)
                            {
                                t = 't';
                            }
                        }
                        snprintf(s, V_LINE_NOTES, "%s%s%c", running[j]->note, running[j]->lth, t);
                        if (v[j][0] != '\0')
                        {
                            strncat(v[j], ",", V_LINE_NOTES);
                        }
                        strncat(v[j], s, V_LINE_NOTES);
                        running[j] = running[j]->vcnext;
                    }
                }
            }
        }
    }
    for (int j = 1; j < MC_MAXVOICE; j++)
    {
        if (v[j][0] != '\0')
        {
            printf("v%2d: %s\n", j, v[j]);
            v[j][0] = '\0';
        }
    }
}   /* End of print_h_mc */

/* ======================================================================== */
static void check_time_ordering(void)
{
    long first;
    int flag = FALSE;

    /* Check that tempo's are in order. */
    first = 0;
    for (struct tempo_signature *t = tempo_signature_start; t != NULL; t = t->temponext)
    {
      if (first > t->tempo_time)
      {
        fprintf(stderr, "tempo time out of positional order %ld > %ld\n", first, t->tempo_time);
        flag = TRUE;
      }
      else
      {
        first = t->tempo_time;
      }
    }

    /* Check that time signatures are in order. */
    first = 0;
    for (struct time_signature *t = time_signature_start; t != NULL; t = t->tsnext)
    {
      if (first > t->timesig_time)
      {
        fprintf(stderr, "time signature out of positional order %ld > %ld\n", first, t->timesig_time);
        flag = TRUE;
      }
      else
      {
        first = t->timesig_time;
      }
    }

    /* Check that track's have notes in order. */
    for (int t = 0; t < trackcount; t++)
    {
      first = 0;
      for (struct note *n = track[t]; n != NULL; n = n->notenext)
      {
        if (first > n->n_start_time)
        {
          fprintf(stderr, "track %d has note n_start_time out of positional order %ld > %ld\n", t, first, n->n_start_time);
            flag = TRUE;
        }
        else
        {
          first = n->n_start_time;
        }
      }
    }

    if (flag)
    {
        mc_exit(1);
    }
}   /* End of check_time_ordering */

/* ------------------------------------------------------------------------ */
static int bar_time_within_approx(long p_now, long p_check, long p_around, long p_extra)
{
    /* If past, grab it. */
    if (p_now >= p_check)
    {
        return(TRUE);
    }
    /* The approximately grab everything up to next "guessed measure", excepting short notes.*/
    if ((p_now+(p_around - p_extra)) >= p_check)
    {
        return(TRUE);
    }
    return(FALSE);
}   /* End of time_within_approx */

/* ------------------------------------------------------------------------ */
static void create_bars(void)
{
    long            now = 0;
    struct time_signature *time_s = time_signature_start;
    struct tempo_signature *tempo_s = tempo_signature_start;

    bar_number = 1;

    while (TRUE)
    {
        /* Have the measure for this area. */
        struct bars_location *newbar = checkmalloc(sizeof(*newbar));
        newbar->barsnext = NULL;
        newbar->bar_time = now;
        newbar->bar_number = bar_number;
        if (bars_location_start == NULL)
        {
            bars_location_start = newbar;
        }
        else
        {
            bars_location_last->barsnext = newbar;
        }
        bars_location_last = newbar;
        bar_number++;

        /* Do tempo before time signature, so that they stick together. :) */
        /* Check for tempo signature at this point -- slightly before or slightly after. */
        while (tempo_s != NULL && bar_time_within_approx(now, tempo_s->tempo_time, TICKSPERBAR, EXTRA))
        {
            tempo_s->tempo_time = now;      /* Normalize into measure. */
            /* Nothing to do with contents of tempo. */
            tempo_s = tempo_s->temponext;   /* Next time_signature. */
        }

        /* Check for time signature at this point -- slightly before or slightly after. */
        while (time_s != NULL && bar_time_within_approx(now, time_s->timesig_time, TICKSPERBAR, EXTRA))
        {
            time_s->timesig_time = now;     /* Normalize into measure. */
            meter_n = time_s->numer;        /* Adjust TICKSPERBAR variables. */
            meter_d = time_s->denom;
            time_s = time_s->tsnext;        /* Next time_signature. */
        }

        /* Use possible time signature above to get to next bar. */
        now = now + TICKSPERBAR;
        if (now > maxtracktime)
        {
            break;
        }
    }
}   /* End of create_bars */

/* ------------------------------------------------------------------------ */
#if 0
static void printtracks(char *s)
{
    fprintf(stderr, "-- %s -------------------------------------------\n", s);
    for (int j = 0; j < trackcount; j++)
    {
      for (struct note *k = track[j]; k != NULL; k = k->notenext)
      {
        fprintf(stderr, "%d n=%3s p=%2d c=%1d v=%2d st=%5ld sp=%5ld delta=%ld\n", j, nt[k->pitch+1], k->pitch, k->chan, k->vel, k->n_start_time, k->n_stop_time, k->n_stop_time - k->n_start_time);
      }
    }
}   /* End of printtracks */
#endif  /* 0 */

/* ------------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    int             maxvoice;
    long            f;
    int             vertical = TRUE;

    if (argc == 3)
    {
        /* For now, assume want V: lines, and not note#,note#,note#,... */
        vertical = FALSE;
    }
    if (argc >= 2)
    {
        if ((F = fopen(argv[1],"rb")) == NULL)
        {
            fprintf(stderr, "Error - Cannot open file %s\n", argv[1]);
            mc_exit(1);
        }
    }
    else if (argc == 1)
    {
        F = stdin;
    }
    else
    {
        fprintf(stderr, "No input file given, no other arguments allowed.\n");
        mc_exit(1);
    }

    /* Put into single character input mode. */
    get_original_termios();

    readtracks();
    fclose(F);

    /* Set note fractions. */
    f = 1;
    for (int j = 0; j < MAXFRAC; j++)
    {
        fractionsdotted[j] = ((division * 4) / f) * 1.5;
        fractions[j] = (division * 4) / f;
        f = f * 2;
    }
    f = 1;
    for (int j = 0; j < MAXFRAC3; j++)
    {
        fractionsthirddotted[j] = (((division * 4) / f) * 1.5) / 3.0;
        fractionsthird[j] = ((division * 4.0) / f) / 3.0;
        f = f * 2;
    }
fprintf(stderr, "fractionsdotted     [0] = %4d  f=%3dd\n"  , fractionsdotted     [0], 1);
fprintf(stderr, "fractions           [0] = %4d  f=%3d\n"   , fractions           [0], 1);
fprintf(stderr, "fractionsdotted     [1] = %4d  f=%3dd\n"  , fractionsdotted     [1], 2);
fprintf(stderr, "fractions           [1] = %4d  f=%3d\n"   , fractions           [1], 2);
#if MAXFRAC3 > 0
fprintf(stderr, "fractionsthirddotted[0] = %4d  f=%3dd/3\n", fractionsthirddotted[0], 1);
#endif /* MAXFRAC3 > 0 */
fprintf(stderr, "fractionsdotted     [2] = %4d  f=%3dd\n"  , fractionsdotted     [2], 4);
#if MAXFRAC3 > 0
fprintf(stderr, "fractionsthird      [0] = %4d  f=%3d/3\n" , fractionsthird      [0], 1);
#endif /* MAXFRAC3 > 0 */
fprintf(stderr, "fractions           [2] = %4d  f=%3d\n"   , fractions           [2], 4);
#if MAXFRAC3 > 1
fprintf(stderr, "fractionsthirddotted[1] = %4d  f=%3dd/3\n", fractionsthirddotted[1], 2);
#endif /* MAXFRAC3 > 1 */
fprintf(stderr, "fractionsdotted     [3] = %4d  f=%3dd\n"  , fractionsdotted     [3], 8);
#if MAXFRAC3 > 1
fprintf(stderr, "fractionsthird      [1] = %4d  f=%3d/3\n" , fractionsthird      [1], 2);
#endif /* MAXFRAC3 > 1 */
fprintf(stderr, "fractions           [3] = %4d  f=%3d\n"   , fractions           [3], 8);
#if MAXFRAC3 > 2
fprintf(stderr, "fractionsthirddotted[2] = %4d  f=%3dd/3\n", fractionsthirddotted[2], 4);
#endif /* MAXFRAC3 > 2 */
fprintf(stderr, "fractionsdotted     [4] = %4d  f=%3dd\n"  , fractionsdotted     [4], 16);
#if MAXFRAC3 > 2
fprintf(stderr, "fractionsthird      [2] = %4d  f=%3d/3\n" , fractionsthird      [2], 4);
#endif /* MAXFRAC3 > 2 */
fprintf(stderr, "fractions           [4] = %4d  f=%3d\n"   , fractions           [4], 16);
#if MAXFRAC3 > 3
fprintf(stderr, "fractionsthirddotted[3] = %4d  f=%3dd/3\n", fractionsthirddotted[3], 8);
#endif /* MAXFRAC3 > 3 */
fprintf(stderr, "fractionsdotted     [5] = %4d  f=%3dd\n"  , fractionsdotted     [5], 32);
#if MAXFRAC3 > 3
fprintf(stderr, "fractionsthird      [3] = %4d  f=%3d/3\n" , fractionsthird      [3], 8);
#endif /* MAXFRAC3 > 3 */
fprintf(stderr, "fractions           [5] = %4d  f=%3d\n"   , fractions           [5], 32);
#if MAXFRAC3 > 4
fprintf(stderr, "fractionsthirddotted[4] = %4d  f=%3dd/3\n", fractionsthirddotted[4], 16);
#endif /* MAXFRAC3 > 4 */
fprintf(stderr, "fractionsdotted     [6] = %4d  f=%3dd\n"  , fractionsdotted     [6], 64);
#if MAXFRAC3 > 4
fprintf(stderr, "fractionsthird      [4] = %4d  f=%3d/3\n" , fractionsthird      [4], 16);
#endif /* MAXFRAC3 > 4 */
fprintf(stderr, "fractions           [6] = %4d  f=%3d\n"   , fractions           [6], 64);
#if MAXFRAC3 > 5
fprintf(stderr, "fractionsthirddotted[5] = %4d  f=%3dd/3\n", fractionsthirddotted[5], 32);
fprintf(stderr, "fractionsthird      [5] = %4d  f=%3d/3\n" , fractionsthird      [5], 32);
#endif  /* MAXFRAC3 > 5 */
#if MAXFRAC3 > 6
fprintf(stderr, "fractionsthirddotted[6] = %4d  f=%3dd/3\n", fractionsthirddotted[6], 64);
fprintf(stderr, "fractionsthird      [6] = %4d  f=%3d/3\n" , fractionsthird      [6], 64);
#endif  /* MAXFRAC3 > 6 */
fprintf(stderr, "-----------------------------------------------------------------------------\n");

    for (int t = 0; t < MAXTRACKS; t++)
    {
        track[t] = list_sort(track[t]);
    }

    create_bars();

    /* Get rid of funny note start/stop times, and delta zero for notes. */
    /* Use "bars" to move forward/backwards, lengthen/shorten -- via EXTRA. */
    fix_note_start_stop_time();

    /* Put a rest at the start & end of tracks that start later. */
    set_starting_rest_times();

    maxvoice = 0;
    for (int j = 0; j < trackcount; j++)
    {
        if (track[j] != NULL)
        {
            maxvoice++;
        }
    }
    if (maxvoice == 0)
    {
        fprintf(stderr, "MIDI file has no notes!\n");
        mc_exit(1);
    }

    setupkey(keysig, FALSE);

    check_time_ordering();          /* Just in case -- must be sorted. */

    voicecount = 0;
    for (int j = 0; j < trackcount; j++)
    {
        if (track[j] == NULL)
        {
            continue;
        }
        voicesbeyondthis = 0;
        voicecount = voicecount + 1;
        printtrack(j);
        voicecount = voicecount + voicesbeyondthis;
    }

    put_in_mc_rests();              /* If any voices in a bar without rests, add. */
   
    if (vertical)
    {
        print_v_mc();               /* vertical notes. */
    }
    else
    {
        print_h_mc();               /* horizontal notes. */
    }
    
    restore_original_termios();

    fclose(stdout);
    return (0);
}   /* End of main */

/* ------------------------------------------------------------------------ */
