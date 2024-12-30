#ifndef HEADER_BF_LOCL_H
#define HEADER_BF_LOCL_H

/* NOTE - c is not incremented as per n2l */
#define n2ln(c,l1,l2,n) { \
                            c+=n; \
                            l1=l2=0; \
                            switch (n) { \
                                case 8: l2 =((*(--(c))))    ; \
                                case 7: l2|=((*(--(c))))<< 8; \
                                case 6: l2|=((*(--(c))))<<16; \
                                case 5: l2|=((*(--(c))))<<24; \
                                case 4: l1 =((*(--(c))))    ; \
                                case 3: l1|=((*(--(c))))<< 8; \
                                case 2: l1|=((*(--(c))))<<16; \
                                case 1: l1|=((*(--(c))))<<24; \
                            } \
                        }
/* NOTE - c is not incremented as per l2n */
#define l2nn(l1,l2,c,n) { \
                            c+=n; \
                            switch (n) { \
                                case 8: *(--(c))=(unsigned char)(((l2)    )&0xff); \
                                case 7: *(--(c))=(unsigned char)(((l2)>> 8)&0xff); \
                                case 6: *(--(c))=(unsigned char)(((l2)>>16)&0xff); \
                                case 5: *(--(c))=(unsigned char)(((l2)>>24)&0xff); \
                                case 4: *(--(c))=(unsigned char)(((l1)    )&0xff); \
                                case 3: *(--(c))=(unsigned char)(((l1)>> 8)&0xff); \
                                case 2: *(--(c))=(unsigned char)(((l1)>>16)&0xff); \
                                case 1: *(--(c))=(unsigned char)(((l1)>>24)&0xff); \
                            } \
                        }
#undef n2l
#define n2l(c,l)        (l =((*((c)++)))<<24L, \
                         l|=((*((c)++)))<<16L, \
                         l|=((*((c)++)))<< 8L, \
                         l|=((*((c)++))))
#undef l2n
#define l2n(l,c)        (*((c)++)=(unsigned char)(((l)>>24L)&0xff), \
                         *((c)++)=(unsigned char)(((l)>>16L)&0xff), \
                         *((c)++)=(unsigned char)(((l)>> 8L)&0xff), \
                         *((c)++)=(unsigned char)(((l)     )&0xff))
/*
 * This is a *generic* version. Seem to perform best on platforms that
 * offer explicit support for extraction of 8-bit nibbles preferably
 * complemented with "multiplying" of array index by sizeof(BF_LONG).
 * For the moment of this writing the list comprises Alpha CPU featuring
 * extbl and s[48]addq instructions.
 */
#define BF_ENC(LL,R,S,P) ( \
                            LL^=P, \
                            LL^=((( S[   ((R>>24)&0xff)] + \
                                S[0x0100+((R>>16)&0xff)])^ \
                                S[0x0200+((R>> 8)&0xff)])+ \
                                S[0x0300+((R    )&0xff)])&(~0) \
                          )
#endif /* HEADER_BF_LOCL_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
