/* $Id: ipc_security.c 156532 2011-06-24 21:09:44Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_security.c
** MODULE TITLE:    IPC packet security features
**
** DESCRIPTION:     Implementation of the ipc packet security features.
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "XIO_Std.h"

#include "debug_files.h"
#include "ipc_common.h"
#include "ipc_packets.h"
#include "md5.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "ipc_security.h"

/*****************************************************************************
** Code Start
*****************************************************************************/

static void DumpMD5(const char *fname, const UINT8 *key)
{
    if (key)
    {
        dprintf(DPRINTF_ENCRYPTION_MD5,
                "%25s = %02hhx%02hhx%02hhx%02hhx %02hhx%02hhx%02hhx%02hhx "
                "%02hhx%02hhx%02hhx%02hhx %02hhx%02hhx%02hhx%02hhx\n", fname, key[0],
                key[1], key[2], key[3], key[4], key[5], key[6], key[7], key[8], key[9],
                key[10], key[11], key[12], key[13], key[14], key[15]);
    }
    else
    {
        dprintf(DPRINTF_ENCRYPTION_MD5, "%25s key is NULL\n", fname);
    }
}

/*----------------------------------------------------------------------------
**  Function Name: bool CheckHeaderMD5( IPC_PACKET *packet )
**
**  Description:    Checks packet header for correct MD5 signature
**
**  Inputs: IPC_PACKET *packet      Packet pointer
**          UINT8 *key              key pointer
**
**  Returns:    Bool true if Md5 OK, else returns false
**--------------------------------------------------------------------------*/
bool CheckHeaderMD5(IPC_PACKET *packet, UINT8 *key)
{
    bool        rc = FALSE;
    MD5_CONTEXT md5;

    DumpMD5("CheckHeaderMD5:  key", key);

    /*
     * Check header
     */
    if (packet && packet->header && key)
    {
        /*
         * Init the md5 structure for the data
         */
        MD5Init(&md5);
        MD5Update(&md5,
                  (UINT8 *)packet->header, sizeof(*packet->header) - IPC_DIGEST_LENGTH);
        MD5Update(&md5, key, IPC_DIGEST_LENGTH);

        MD5Final(&md5);

        DumpMD5("CheckHeaderMD5: HDig", packet->header->headerMD5);
        DumpMD5("CheckHeaderMD5: CDig", md5.digest);

        if (memcmp(packet->header->headerMD5, md5.digest, IPC_DIGEST_LENGTH) == 0)
        {
            rc = TRUE;
        }
    }
    return rc;
}

/*----------------------------------------------------------------------------
**  Function Name: bool CheckDataMD5( IPC_PACKET *packet )
**
**  Description:    Checks packet data for correct MD5 signature
**
**  Inputs: IPC_PACKET *packet      Packet pointer
**
**  Returns:    Bool true if Md5 OK, else returns false
**--------------------------------------------------------------------------*/
bool CheckDataMD5(IPC_PACKET *packet)
{
    bool        rc = FALSE;
    MD5_CONTEXT md5;

    /*
     * Check for valid  header
     */
    if (packet && packet->header)
    {
        /*
         * Check data portion
         */
        if (packet->header->length)
        {
            MD5Init(&md5);
            MD5Update(&md5, (UINT8 *)packet->data, packet->header->length);
            MD5Final(&md5);

            DumpMD5("CheckDataMD5: DDig", packet->header->dataMD5);
            DumpMD5("CheckDataMD5: CDig", md5.digest);

            if (memcmp(packet->header->dataMD5, md5.digest, IPC_DIGEST_LENGTH) == 0)
            {
                rc = TRUE;
            }
        }
        else
        {
            rc = TRUE;
        }
    }
    return rc;
}

/*----------------------------------------------------------------------------
**  Function Name:  bool CreateMD5Signature( IPC_PACKET *packet )
**
**  Description:    Creates the MD5 signature
**
**  Inputs:         IPC_PACKET *packet      Packet pointer
**                  UINT8 *key              key pointer
**
**  Returns:        Bool true if no errors occured, else returns false
**--------------------------------------------------------------------------*/
bool CreateMD5Signature(IPC_PACKET *packet, UINT8 *key)
{
    bool        rc = FALSE;
    MD5_CONTEXT md5;

    DumpMD5("CreateMD5Signature:  key", key);

    /*
     * If the packet pointer is NULL or if the length filed in the header
     * is > 0 and the the data pointer is  NULL then we will return FALSE
     */
    if ((packet && packet->header) &&
        (key) && !(packet->header->length && packet->data == NULL))
    {
        /*
         * Init the md5 structure for the data
         */
        MD5Init(&md5);

        /*
         * Check to see if we have a data portion
         */
        if (packet->header->length)
        {
            /*
             * Calculate the md5 for the data portion
             */
            MD5Update(&md5, (UINT8 *)packet->data, packet->header->length);
            MD5Final(&md5);
            memcpy((char *)packet->header->dataMD5, (char *)md5.digest, IPC_DIGEST_LENGTH);
            DumpMD5("CreateMD5Signature: DDig", md5.digest);
        }
        else
        {
            memset(packet->header->dataMD5, 0, IPC_DIGEST_LENGTH);
        }

        /*
         * Init the md5 structure for the header
         */
        MD5Init(&md5);

        /*
         * Generate the md5 for the header - the key space
         */
        MD5Update(&md5, (UINT8 *)packet->header, sizeof(*packet->header) - IPC_DIGEST_LENGTH);

        MD5Update(&md5, key, IPC_DIGEST_LENGTH);
        MD5Final(&md5);
        memcpy((UINT8 *)packet->header->headerMD5, (UINT8 *)md5.digest, IPC_DIGEST_LENGTH);
        DumpMD5("CreateMD5Signature: HDig", md5.digest);

        rc = TRUE;
    }
    return rc;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
