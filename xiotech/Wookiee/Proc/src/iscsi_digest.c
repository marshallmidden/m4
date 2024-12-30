/* $Id: iscsi_digest.c 144139 2010-07-14 19:46:01Z m4 $ */
/**
 ******************************************************************************
 **
 **  @file       iscsi_digest.c
 **
 **  @brief      iSCSI Digest functions
 **
 **  This file provides API's for Header and data digest in iSCSI
 **
 **  Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>
#include "XIO_Types.h"
#include "iscsi_common.h"
#include "iscsi_pdu.h"
#include "iscsi_tsl.h"
#include "iscsi_digest.h"

/**
** There are two lookup table being used in this file. For iscsi implementation
** the POLY should be 0x1EDC6F41L
**
**/

#define INIT 0xFFFFFFFF
#define  XOROT 0xFFFFFFFF

#define DATA_DIGEST_ERROR  1
#define HEADER_DIGEST_ERROR 2
#define FORMAT_ERROR 3

#define F_BIT_SCSI 0x80    /*Fbit position in SCSI request*/
#define R_BIT_SCSI 0x40    /*Rbit position in SCSI request*/
#define W_BIT_SCSI 0x20    /*Wbit position in SCSI request*/
#define IS_SCSI_F_BIT_SET(flag) (flag & F_BIT_SCSI)
#define IS_SCSI_R_BIT_SET(flag) (flag & R_BIT_SCSI)
#define IS_SCSI_W_BIT_SET(flag) (flag & W_BIT_SCSI)

#define PAYLOAD_DIGEST_ERROR 0x02
#define HEADER_LENGTH 48

/*****************************************************************/
/*                                                               */
/* CRC LOOKUP TABLE                                              */
/* ================                                              */
/* The following CRC lookup table was generated automagically    */
/* by the Rocksoft^tm Model CRC Algorithm Table Generation       */
/* Program V1.0 using the following model parameters:            */
/*                                                               */
/*    Width   : 4 bytes.                                         */
/*    Poly    : 0x1EDC6F41L                                      */
/*    Reverse : TRUE.                                            */
/*                                                               */
/* For more information on the Rocksoft^tm Model CRC Algorithm,  */
/* see the document titled "A Painless Guide to CRC Error        */
/* Detection Algorithms" by Ross Williams                        */
/* (ross@guest.adelaide.edu.au.). This document is likely to be  */
/* in the FTP archive "ftp.adelaide.edu.au/pub/rocksoft".        */
/*                                                               */
/*****************************************************************/

unsigned long  crctable[256] =
{
 0x00000000L, 0xF26B8303L, 0xE13B70F7L, 0x1350F3F4L,
 0xC79A971FL, 0x35F1141CL, 0x26A1E7E8L, 0xD4CA64EBL,
 0x8AD958CFL, 0x78B2DBCCL, 0x6BE22838L, 0x9989AB3BL,
 0x4D43CFD0L, 0xBF284CD3L, 0xAC78BF27L, 0x5E133C24L,
 0x105EC76FL, 0xE235446CL, 0xF165B798L, 0x030E349BL,
 0xD7C45070L, 0x25AFD373L, 0x36FF2087L, 0xC494A384L,
 0x9A879FA0L, 0x68EC1CA3L, 0x7BBCEF57L, 0x89D76C54L,
 0x5D1D08BFL, 0xAF768BBCL, 0xBC267848L, 0x4E4DFB4BL,
 0x20BD8EDEL, 0xD2D60DDDL, 0xC186FE29L, 0x33ED7D2AL,
 0xE72719C1L, 0x154C9AC2L, 0x061C6936L, 0xF477EA35L,
 0xAA64D611L, 0x580F5512L, 0x4B5FA6E6L, 0xB93425E5L,
 0x6DFE410EL, 0x9F95C20DL, 0x8CC531F9L, 0x7EAEB2FAL,
 0x30E349B1L, 0xC288CAB2L, 0xD1D83946L, 0x23B3BA45L,
 0xF779DEAEL, 0x05125DADL, 0x1642AE59L, 0xE4292D5AL,
 0xBA3A117EL, 0x4851927DL, 0x5B016189L, 0xA96AE28AL,
 0x7DA08661L, 0x8FCB0562L, 0x9C9BF696L, 0x6EF07595L,
 0x417B1DBCL, 0xB3109EBFL, 0xA0406D4BL, 0x522BEE48L,
 0x86E18AA3L, 0x748A09A0L, 0x67DAFA54L, 0x95B17957L,
 0xCBA24573L, 0x39C9C670L, 0x2A993584L, 0xD8F2B687L,
 0x0C38D26CL, 0xFE53516FL, 0xED03A29BL, 0x1F682198L,
 0x5125DAD3L, 0xA34E59D0L, 0xB01EAA24L, 0x42752927L,
 0x96BF4DCCL, 0x64D4CECFL, 0x77843D3BL, 0x85EFBE38L,
 0xDBFC821CL, 0x2997011FL, 0x3AC7F2EBL, 0xC8AC71E8L,
 0x1C661503L, 0xEE0D9600L, 0xFD5D65F4L, 0x0F36E6F7L,
 0x61C69362L, 0x93AD1061L, 0x80FDE395L, 0x72966096L,
 0xA65C047DL, 0x5437877EL, 0x4767748AL, 0xB50CF789L,
 0xEB1FCBADL, 0x197448AEL, 0x0A24BB5AL, 0xF84F3859L,
 0x2C855CB2L, 0xDEEEDFB1L, 0xCDBE2C45L, 0x3FD5AF46L,
 0x7198540DL, 0x83F3D70EL, 0x90A324FAL, 0x62C8A7F9L,
 0xB602C312L, 0x44694011L, 0x5739B3E5L, 0xA55230E6L,
 0xFB410CC2L, 0x092A8FC1L, 0x1A7A7C35L, 0xE811FF36L,
 0x3CDB9BDDL, 0xCEB018DEL, 0xDDE0EB2AL, 0x2F8B6829L,
 0x82F63B78L, 0x709DB87BL, 0x63CD4B8FL, 0x91A6C88CL,
 0x456CAC67L, 0xB7072F64L, 0xA457DC90L, 0x563C5F93L,
 0x082F63B7L, 0xFA44E0B4L, 0xE9141340L, 0x1B7F9043L,
 0xCFB5F4A8L, 0x3DDE77ABL, 0x2E8E845FL, 0xDCE5075CL,
 0x92A8FC17L, 0x60C37F14L, 0x73938CE0L, 0x81F80FE3L,
 0x55326B08L, 0xA759E80BL, 0xB4091BFFL, 0x466298FCL,
 0x1871A4D8L, 0xEA1A27DBL, 0xF94AD42FL, 0x0B21572CL,
 0xDFEB33C7L, 0x2D80B0C4L, 0x3ED04330L, 0xCCBBC033L,
 0xA24BB5A6L, 0x502036A5L, 0x4370C551L, 0xB11B4652L,
 0x65D122B9L, 0x97BAA1BAL, 0x84EA524EL, 0x7681D14DL,
 0x2892ED69L, 0xDAF96E6AL, 0xC9A99D9EL, 0x3BC21E9DL,
 0xEF087A76L, 0x1D63F975L, 0x0E330A81L, 0xFC588982L,
 0xB21572C9L, 0x407EF1CAL, 0x532E023EL, 0xA145813DL,
 0x758FE5D6L, 0x87E466D5L, 0x94B49521L, 0x66DF1622L,
 0x38CC2A06L, 0xCAA7A905L, 0xD9F75AF1L, 0x2B9CD9F2L,
 0xFF56BD19L, 0x0D3D3E1AL, 0x1E6DCDEEL, 0xEC064EEDL,
 0xC38D26C4L, 0x31E6A5C7L, 0x22B65633L, 0xD0DDD530L,
 0x0417B1DBL, 0xF67C32D8L, 0xE52CC12CL, 0x1747422FL,
 0x49547E0BL, 0xBB3FFD08L, 0xA86F0EFCL, 0x5A048DFFL,
 0x8ECEE914L, 0x7CA56A17L, 0x6FF599E3L, 0x9D9E1AE0L,
 0xD3D3E1ABL, 0x21B862A8L, 0x32E8915CL, 0xC083125FL,
 0x144976B4L, 0xE622F5B7L, 0xF5720643L, 0x07198540L,
 0x590AB964L, 0xAB613A67L, 0xB831C993L, 0x4A5A4A90L,
 0x9E902E7BL, 0x6CFBAD78L, 0x7FAB5E8CL, 0x8DC0DD8FL,
 0xE330A81AL, 0x115B2B19L, 0x020BD8EDL, 0xF0605BEEL,
 0x24AA3F05L, 0xD6C1BC06L, 0xC5914FF2L, 0x37FACCF1L,
 0x69E9F0D5L, 0x9B8273D6L, 0x88D28022L, 0x7AB90321L,
 0xAE7367CAL, 0x5C18E4C9L, 0x4F48173DL, 0xBD23943EL,
 0xF36E6F75L, 0x0105EC76L, 0x12551F82L, 0xE03E9C81L,
 0x34F4F86AL, 0xC69F7B69L, 0xD5CF889DL, 0x27A40B9EL,
 0x79B737BAL, 0x8BDCB4B9L, 0x988C474DL, 0x6AE7C44EL,
 0xBE2DA0A5L, 0x4C4623A6L, 0x5F16D052L, 0xAD7D5351L
};


/*
    iscsi_CalculateCRC32 calculates the digest of the given message.
    If the message does not conforms to 4-byte boundary. This function
    pads the message and then takes the digest.
*/
void findoffset(SGL_DESC *pSgl,unsigned long sglCount,UINT32 start,UINT32 *start_i, UINT32 *start_offset);
void findoffset(SGL_DESC *pSgl,unsigned long sglCount,UINT32 start,UINT32 *start_i, UINT32 *start_offset)
{
    unsigned long length = 0,total_length = 0;
    UINT32 i=0;
    for (i = 0; i < sglCount; i++)
    {
        length = (pSgl + i)->len & 0x00FFFFFF;
        total_length += length;
        if(start <= total_length)
        {
            *start_offset =  (start - (total_length - length));
            *start_i = i;
            return;
        }
    }
}

unsigned long iscsi_CalculateCRC32_sgl(SGL_DESC *pSgl, UINT32 sglCount, UINT32 start, UINT32 end)
{
    unsigned long crc32 = INIT;
    UINT8 zero = 0;
    UINT8 *message = NULL;
    UINT32 i=0;
    UINT32 start_i = 0;
    UINT32 start_offset = 0;
    UINT32 end_i = 0;
    UINT32 end_offset = 0;
    unsigned long length=0;
    int pad = 0;
    unsigned long total_length = 0;

    findoffset(pSgl,sglCount,start,&start_i,&start_offset);
    findoffset(pSgl,sglCount,end,&end_i,&end_offset);


    for (i = 0; i < sglCount; i++)
    {
        if(i >= start_i && i <= end_i)
        {
            length = (pSgl + i)->len & 0x00FFFFFF;
            if(i == start_i)
            {
                message = (UINT8*)((pSgl + i)->addr) + start_offset;
                length = length - start_offset;
            }else
            {
                message = (pSgl + i)->addr;
            }
            if(i == end_i)
            {
                length = end_offset;
                if(i == start_i)
                {
                    length = end_offset - start_offset + 1;
                }
            }

            total_length += length;
            while (length--  )
                crc32 = crctable[(crc32 ^ *message++) & 0xFFL] ^ (crc32 >> 8);
        }
    }
    pad = (-total_length)&3;
    while(pad-- )
        crc32 = crctable[(crc32 ^ zero ) & 0xFFL] ^ (crc32 >> 8);

    return crc32 ^ XOROT;
}

unsigned long iscsi_CalculateCRC32(const UINT8 *message,unsigned long length)
{
    unsigned long crc32 = INIT;
    UINT8 zero = 0;
    INT32 pad = 0;

    pad = (-length)&3;

    while (length-- )
        crc32 = crctable[(crc32 ^ *message++) & 0xFFL] ^ (crc32 >> 8);
    while(pad-- )
        crc32 = crctable[(crc32 ^ zero ) & 0xFFL] ^ (crc32 >> 8);


    return crc32 ^ XOROT;
}

/**
 * iscsi_isDigestError
 * The function takes message  and its length
 * The digest length is known to function. The message and
 * digest should be in network byte order.
 *
 * */
#define true 1
#define false 0

int iscsi_isDigestError(const UINT8 *message, unsigned long length,unsigned long digest)
{
    unsigned long crc = iscsi_CalculateCRC32(message, length);

    if(digest == crc )
    {
        return false;
    }
    fprintf(stderr,"DIGEST ERROR - calculated %lx got %lx\n",crc,digest);

    return true;
}

/**
******************************************************************************
**  @name     isDigestCheckValid
**  @brief      It checks whether there is any need for Digest Check
**        only some states of connection digest check is valid
**
**  @param      ISCSI_TPD* - pointer to ISCSI_TPD
**
**  @return
**
**              XIO_SUCCESS - if digest check valid
**              XIO_FAILURE - if invalid
**
******************************************************************************
**/

UINT32 isDigestCheckValid(ISCSI_TPD *pTPD)
{
    CONNECTION *pConnection;
#ifdef DEBUG
    if(!pTPD || !pTPD->pConn) return XIO_FAILURE;

#endif
    pConnection = pTPD->pConn;

    if(pConnection->state ==  CONNST_FREE ||
        pConnection->state ==  CONNST_XPT_WAIT ||
        pConnection->state ==  CONNST_XPT_UP ||
        pConnection->state ==  CONNST_IN_LOGIN)
    {
        return XIO_FAILURE;
    }
    return XIO_SUCCESS;
}

/*
*    @name     ReadDataDigestAndCheck
*    @brief    The function reads the data digest and then check whether it is correct or not
*    @param    CONNECTION *pConn- pointer to connection
*            UINT8* pointer to data
*            UINT32 length of data
*
*    @return -1 if digest error
*            1 if digest ok
*/

int ReadDataDigestAndCheck(CONNECTION *pConn, SGL_DESC* data, UINT8 sglCount, UINT32 start, UINT32 end)
{
    UINT32 dataDigest=0xabababab;
    unsigned long crc;

    if ((pConn == NULL) || (data == NULL))
    {
        fprintf(stderr,"ReadDataDigestAndCheck: Invalid Input Params\n");
        return -1;
    }

    if(stringCompare(pConn->params.dataDigest.strval,(UINT8*) "CRC32C") == 0  )
    {
        memcpy((UINT8*)&dataDigest,((ISCSI_GENERIC_HDR*)(pConn->pHdr))->headerDigest,4);

        crc = iscsi_CalculateCRC32_sgl((SGL_DESC*)data, sglCount, start, end);

        if(dataDigest != crc)
        {
            fprintf(stderr,"ReadDataDigestAndCheck: calculated digest = %lx, recvd digest = %x\n",crc, dataDigest);
            fprintf(stderr,"sglcount = %d, desc addr = %x, desc len = %d start = %d end = %d\n",sglCount, (UINT32)data->addr, data->len,start,end);
            return -1;
        }
    }
    return 1;
}

INT32 HandleDataDigestError(CONNECTION *pConn,ISCSI_GENERIC_HDR *pRejectedHdr)
{
    /*
    ** For ErrorRecoveryLevel = 0, We will only send Reject PDU
    */
    INT32 status;

    if ((pConn == NULL) || (pRejectedHdr == NULL))
    {
        fprintf(stderr,"HandleDataDigestError: Invalid Input Params\n");
        return -1;
    }
    status = iscsiBuildAndSendReject(pConn,REJECT_DIGEST_ERROR,pRejectedHdr);

    return status;
}


void AppendHeaderDigest(ISCSI_TPD *pTPD, char* buffer, int *totalLength)
{
    if(isDigestCheckValid(pTPD)== XIO_SUCCESS &&
        stringCompare(pTPD->pConn->params.headerDigest.strval,(UINT8*) "CRC32C") == 0
        && GET_ISCSI_OPCODE(buffer[0])!= TARGOP_LOGIN_RESP )
    {
        char headerDigest[4];

        UINT32 digest = iscsi_CalculateCRC32((UINT8 *)buffer,ISCSI_HDR_LEN);

        memcpy(headerDigest,(UINT8*)&digest,4);
        memcpy(buffer + *totalLength, headerDigest, DIGEST_LENGTH);
        *totalLength += DIGEST_LENGTH;
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
