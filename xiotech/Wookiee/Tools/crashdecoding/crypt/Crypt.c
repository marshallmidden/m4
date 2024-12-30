#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include "blowfish.h"

#define INFILE      (argv[1])
#define OUTFILE     (argv[2])
#define DSC         (argv[3])
#define MODE        (argv[4])

/* MODE values */
#define DECRYPT 0
#define ENCRYPT 1

int main(int argc, char *argv[])
{
    FILE           *in = NULL;
    FILE           *out = NULL;
    int             rc;
    size_t          src;
    struct stat     statbuf;
    unsigned char  *inbuf = NULL;
    unsigned char  *outbuf = NULL;
    off_t           size;
    unsigned char   pad = 0;
    int             mode;
    off_t           i;
    BF_KEY          bfctx;
    /* iVec used for Cipher Block Chaining */
    unsigned char   iVec[16] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
    size_t          passwordLen = 0;
    off_t           statbufsize = 0;
    unsigned char   UPASSWORD[128];

    if (argc != 5)
    {
        (void)printf("Usage: crypt input-filename output-filename DSC_Number mode\n");
        (void)printf("       where mode is:\n");
        (void)printf("       0 for DECRYPT\n");
        (void)printf("       1 for ENCRYPT\n");
        exit(0);
    }

    memcpy(UPASSWORD, "b1gXXXXXf00t", 13);
    passwordLen = strlen(DSC);
    memcpy(UPASSWORD+3, DSC, passwordLen);
    passwordLen = 12;

    BF_set_key(&bfctx, (int)passwordLen, UPASSWORD);

    in = fopen(INFILE, "rb");
    if (!in)
    {
        (void)printf("Could not open %s for read. errno %d\n", INFILE, errno);
        exit(0);
    }

    out = fopen(OUTFILE, "wb");
    if (!out)
    {
        (void)printf("Could not open %s for write. errno %d\n", OUTFILE, errno);
        exit(0);
    }

    rc = fstat(fileno(in), &statbuf);
    if (rc)
    {
        (void)printf("Could not fstat %s. errno %d\n", OUTFILE, errno);
        exit(0);
    }

    size = (statbuf.st_size + 7) & ~0x7;

    if (size < 0)
    {
        (void)printf("Invalid size encountered\n");
        exit(0);
    }

    mode = atoi(MODE);
    if (mode != 0 && mode != 1)
    {
        (void)printf("'mode' must be 0 or 1\n");
        exit(0);
    }

    if (mode == ENCRYPT)
    {
        pad = (unsigned char)(size - statbuf.st_size);
        if (!pad)
        {
            pad = 8;
            size += 8;
        }
    }
    else
    {
        if (size != statbuf.st_size)
        {
            (void)printf("Something is wrong; input file size not a multiple of 8 (%zd)\n", size);
            exit(0);
        }
    }

    inbuf = (unsigned char *)malloc((size_t)size);
    outbuf = (unsigned char *)malloc((size_t)size);

    statbufsize = statbuf.st_size;

    if (statbufsize < 0)
    {
        (void)printf("Unexpected error, invalid size\n");
        exit(0);
    }

    src = fread(inbuf, 1, (size_t)statbufsize, in);
    if (src != (size_t)statbufsize)
    {
        (void)printf("Could not read all of %s (%zd/%zd).\n", INFILE, src, statbufsize);
        exit(0);
    }

    if (mode == ENCRYPT)
    {
        /* pad the buffer */
        for (i = statbufsize; i < size; i++)
        {
            inbuf[i] = pad;
        }

        (void)printf("Encrypting: %s -> %s\n", INFILE, OUTFILE);
        BF_cbc_encrypt(inbuf, outbuf, (int)size, &bfctx, iVec, BF_ENCRYPT);
        src = fwrite(outbuf, 1, (unsigned int)size, out);
        (void)printf("Writing %zd bytes\n", size);
        if (src != (size_t)size)
        {
            (void)printf("Could not write all of %s (%zd/%zd).\n", OUTFILE, src, statbufsize);
            exit(0);
        }
    }
    else
    {
        (void)printf("Decrypting: %s -> %s\n", INFILE, OUTFILE);
        BF_cbc_encrypt(inbuf, outbuf, (int)size, &bfctx, iVec, BF_DECRYPT);

        /* strip the buffer padding */
        pad = outbuf[size - 1];

        if ((size - pad) < 0)
        {
            (void)printf("Pad is greater than size.\n");
            exit(0);
        }

        src = fwrite(outbuf, 1, (unsigned int)(size - pad), out);
        (void)printf("Writing %zd bytes\n", size - pad);
        if (src != (size_t)(size - pad))
        {
            (void)printf("Could not write all of %s (%zd/%zd).\n", OUTFILE, src, statbufsize);
            exit(0);
        }
    }
    return(0);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
