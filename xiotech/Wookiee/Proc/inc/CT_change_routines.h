/* These routines force using the Bigfoot system provided routines. */

extern int CT_isspace_CT(int);
extern int CT_strlen_CT(const char *);
extern int CT_strncmp_CT(char *, const char *, int);
extern char *CT_strncpy_CT(char *, char *, int);
extern void *CT_memcpy_CT(void *, void *, int);
extern void *CT_memset_CT(void *, int, int);
extern void *CT_memmove_CT(void *, void *, int);

#undef isspace
#define isspace(x)      CT_isspace_CT(x)
#undef strncmp
#define strncmp(x,y,z)  CT_strncmp_CT(x,y,z)
#undef strlen
#define strlen(x)       CT_strlen_CT(x)
#undef memcpy
#define memcpy(x,y,z)   CT_memcpy_CT(x,y,z)
#undef memset
#define memset(x,y,z)   CT_memset_CT(x,y,z)
#undef memmove
#define memmove(x,y,z)  CT_memmove_CT(x,y,z)
#undef strncpy
#define strncpy(x,y,z)  CT_strncpy_CT(x,y,z)

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
