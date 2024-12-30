#ifndef HEADER_BLOWFISH_H
#define HEADER_BLOWFISH_H

#define BF_ENCRYPT	1
#define BF_DECRYPT	0

#define BF_LONG int

#define BF_ROUNDS	16

typedef struct bf_key_st
{
    BF_LONG         P[BF_ROUNDS + 2];
    BF_LONG         S[4 * 256];
} BF_KEY;

void            BF_set_key(BF_KEY *key, int len, const unsigned char *data);
void            BF_encrypt(BF_LONG *data, const BF_KEY *key);
void            BF_cbc_encrypt(const unsigned char *in, unsigned char *out, int length,
	                           const BF_KEY *schedule, unsigned char *ivec, int encrypt);
#endif	/* HEADER_BLOWFISH_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
