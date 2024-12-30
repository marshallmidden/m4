/* $Id: chap_new.c 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       chap_new.c
**
**  @brief      CHAP (Challenge Handshake Authentication Protocol) based on
**              RFC 1994. Intended for use with iSCSI exclusively.
**
**  Copyright (c) 2005 - 2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "XIO_Types.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "iscsi_common.h"
#include "chap_new.h"
#include "chap_base.h"


static char chap_name_str[CHAP_MAX_NAME_LEN] = "meow meow";

void print_chap_context_st (CHAP_CONTEXT_ST *cc);
int chap_fill_challenge_buf (CHAP_CONTEXT_ST *cc , UINT8 *buf , int   *p_valid_len , int    encoding_base);

int chap_fill_resp_buf(CHAP_CONTEXT_ST *cc , UINT8 *buf , int   *p_valid_len , int    encoding_base , UINT8* targetName);
static void compute_md5 (UINT8 *result , int result_len , UINT8 *message , int message_len);
static UINT8 get_next_auth_id (void);

extern int chapGetSecret (UINT16 tid,UINT8 *name,UINT8 *secret1, UINT8* secret2);


/****************************************************************************
 ** NOTES:    allocates memory for a new struct, and inits it.
 **
 **           the users are expected to initialize all fields except
 **           cookie before using them. This function deliberately fills
 **           the struct with junk to catch bugs. See "writing solid code"
 **           for more info.
 **
 ** RETURNS:  pointer to the newly allocated struct.
 **
 **
 ****************************************************************************/
CHAP_CONTEXT_ST *chap_create_context_st(void)
{
  CHAP_CONTEXT_ST *cc;

  cc = (CHAP_CONTEXT_ST *) s_MallocC (sizeof (*cc), __FILE__, __LINE__);

  /*
  ** to cause bugs!
  */
  memset (cc, CHAP_JUNK_UINT8, sizeof (*cc));

  /*
  ** set the cookie for later validations
  */
  cc->cookie = CHAP_CONTEXT_COOKIE;
  cc->chal_recvd_len = 0;
  cc->id_recvd = 0;
  cc->state = CHAP_INIT;
  return cc;
}


/****************************************************************************
 ** NOTES:    frees the struct after filling the memory with garbage.
 **
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
void chap_release_context_st (CHAP_CONTEXT_ST *cc)
{
  if(cc != NULL)
  {
      /*
      ** to catch bugs!
      */
      memset (cc, CHAP_JUNK_UINT8, sizeof (*cc));

      /*
      ** spoil the cookie to ensure that we don't accidentally reuse the
      ** struct.
      */
      cc->cookie = CHAP_JUNK_INT;
      s_Free(cc, (sizeof (*cc)), __FILE__, __LINE__);
  }
}




/****************************************************************************
 ** NOTES:    prints an array of bytes in hex
 **
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
static void print_uint8_arr (UINT8 *p, int p_size)
{
  int i;
  for (i = 0; i < p_size; i++)
    {
      chap_log_mid ("%02x ", p[i]);

      if (i % 4 == 3) chap_log_mid ("  ");
    }

  chap_log_mid ("\n");

}


/****************************************************************************
 ** NOTES:    prints the struct on the screen
 **
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
void print_chap_context_st (CHAP_CONTEXT_ST *cc)
{
  char temp[CHAP_MAX_RESP_RECVD_LEN + 1];

  chap_log_mid ("CHAP CONTEXT ST:\n");
  chap_log_mid ("cookie             --> %d\n",   cc->cookie         );
  chap_log_mid ("id                 --> %d\n",   cc->id           );
  chap_log_mid ("algorithm          --> %d\n",   cc->algorithm    );

  chap_log_mid ("chal               --> ");
  print_uint8_arr (cc->chal,   cc->chal_len);
  chap_log_mid ("chal_len           --> %d\n",   cc->chal_len     );

  chap_log_mid ("name               --> ");
  print_uint8_arr (cc->name,   cc->name_len);
  chap_log_mid ("name_len           --> %d\n",   cc->name_len     );

  /*
  ** print as a binary string.
  */
  chap_log_mid ("resp_expected_bin  --> ");
  print_uint8_arr (cc->resp_expected_bin, cc->resp_expected_len);
  chap_log_mid ("resp_expected_len  --> %d\n",   cc->resp_expected_len     );

  /*
  ** print as an ascii string.
  */
  memcpy (temp, cc->resp_recvd_encoded, cc->resp_recvd_len);
  temp[cc->resp_recvd_len] = '\0';
  chap_log_mid ("resp_recvd_encoded A--> %s\n", temp);

  chap_log_mid ("resp_recvd_encoded B-->");
  print_uint8_arr (cc->resp_recvd_encoded, cc->resp_recvd_len);

  chap_log_mid ("resp_recvd_len     --> %d\n",   cc->resp_recvd_len     );
}



/****************************************************************************
 ** NOTES:    computes the MD5 value for the message.
 **
 **
 ** RETURNS:  the result indirectly via the result pointer
 **
 ** good
 ****************************************************************************/
static void compute_md5 (UINT8 *result, int result_len UNUSED, UINT8 *message, int message_len)
{
    MD5_CTX context;

    MD5Init (&context);
    MD5Update (&context, message, message_len);
    MD5Final (result, &context);
}


/****************************************************************************
 ** NOTES:    creates a random stream of bytes of the required size.
 **
 **
 ** RETURNS:
 **
 ** good
 ****************************************************************************/
static int create_random_stream (UINT8 *buf, int nbytes)
{
  int i;

  for (i = 0; i < nbytes; i++)
    {
      int r = rand();

      /*
      ** take the last byte of the random value
      */
      *buf++ = r & 0xFF;
    }

  return 0;
}


/****************************************************************************
 ** NOTES:    gets the next free identifier needed for sending a challenge
 **           packet.
 **
 ** RETURNS:  a valid identifier
 **
 ** good
 ****************************************************************************/
static UINT8 get_next_auth_id (void)
{
  int                       id = CHAP_JUNK_INT;
  static int curr_auth_free_id = 0;

  /*
  ** send a proper id in a cyclic manner.
  */
  id = curr_auth_free_id++ % (CHAP_MAX_ID);

  return id;
}


/****************************************************************************
 ** NOTES:    Builds the message using which the peer will compute the
 **           MD5 value, and then computes it.
 **           chap response is the MD5 value for "id + secret +
 **           challenge".
 **           Refer to RFC 1994: section 4.1
 **
 **
 **
 **
 ** RETURNS:  the expected MD5 value for the <id, secret, challenge> triple
 **           via a value-return argument.
 ** good
 ****************************************************************************/
static void compute_expected_resp (UINT8 *resp, int resp_len, UINT8 id,
                                   UINT8 *secret, int secret_len, UINT8 *chal,
                                   int chal_len)
{
  UINT8 *message     = (void *)CHAP_JUNK_POINTER;
  int    message_len = CHAP_JUNK_INT;

  /*
  ** First, build the message whose MD5 will be sent by the peer.
  */
  message_len = CHAP_ID_SIZE_BYTES + secret_len + chal_len;
  message = (UINT8 *) s_MallocC (message_len, __FILE__, __LINE__);

  memcpy (message, &id, CHAP_ID_SIZE_BYTES);
  memcpy (message + CHAP_ID_SIZE_BYTES, secret, secret_len);
  memcpy (message + CHAP_ID_SIZE_BYTES + secret_len, chal, chal_len);

  /*
  ** okay, the message is ready, now compute its MD5 value. This is the
  ** value that the peer has to respond back with.
  */
  compute_md5 (resp, resp_len, message, message_len);
  /*
  ** Free message pointer
  */
  s_Free(message,message_len, __FILE__, __LINE__);
}

/****************************************************************************
 ** NOTES:    fill the buffer in the format that iSCSI uses.
 **
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
int chap_fill_challenge_buf(CHAP_CONTEXT_ST *cc, UINT8 *buf, int *p_valid_len,
                            int encoding_base)
{
  int   encoded_chal_len = CHAP_JUNK_INT;
  int   length = 0; /* for freeing encoded_chal pointer */
  char *encoded_chal = (void *) CHAP_JUNK_POINTER;
#define FILL_format_str     "CHAP_A=%d%cCHAP_I=%d%cCHAP_C=%s%s%c%c"
  char  pattern      = 'Z';    /* to help get size of buf */
  int   i = CHAP_JUNK_INT;
  char  base_type_str[3] =     /* to hold the 0x or 0b. */
    {CHAP_JUNK_CHAR,
     CHAP_JUNK_CHAR,
     CHAP_JUNK_CHAR
    };

  /*
  ** this is very safe. It takes into consideration, the string, and as a
  ** safe side, has the format specifiers included too! Also, the size of
  ** the challenge string can be twice as big as the challenge (in base16
  ** encoding). Leave space for a trailing zero to make it a proper C
  ** string.
  */
  encoded_chal_len = strlen (FILL_format_str) + 2 * cc->chal_len;

  /*
  ** get the length to allocate enceded_resp
  */
  length = (cc->chal_len * 2 + 1);

  encoded_chal = (char *) s_MallocC (length, __FILE__, __LINE__);

  switch (encoding_base)
    {
    case 16:
      binary_to_base16 (cc->chal, cc->chal_len, encoded_chal, &encoded_chal_len);
      strcpy (base_type_str, "0x");
      break;
    case 64:
      binary_to_base64 (cc->chal, cc->chal_len, encoded_chal, &encoded_chal_len);
      strcpy (base_type_str, "0b");
      break;
    default:
      break;
    }

  /*
  ** now, null terminate the encoded_chal.
  */
  encoded_chal[encoded_chal_len] = '\0';

  /*
  ** fill up the buffer. The memset routines helps to identify the last '>'
  ** character. This size of the message is this + 1 (for the terminating
  ** NUL).
  */
  memset (buf, 0, *p_valid_len);
  snprintf ((char *)buf, *p_valid_len, FILL_format_str, cc->algorithm, '\0',
            cc->id, '\0', base_type_str, encoded_chal, '\0', pattern);
  /*
  ** Free encoded_chal pointer
  */
  s_Free(encoded_chal,length, __FILE__, __LINE__);

  /*
  ** get the location of the last "pattern" char.
  */
  for (i = *p_valid_len - 1; i >= 0; i--)
    {
      if (buf[i])
    {
      /*
      ** we found the pattern. remove it and update valid len to
      ** return the right size of our string.
      */
      buf[i] = '\0';
      *p_valid_len = i;
      break;
    }
    }

  return 0;
}


/****************************************************************************
 ** NOTES:    Creates a challenge message string in iSCSI format.
 **           Needs a valid algorithm code and name (and name_len) in the
 **           context struct; it gets the secret from the database by
 **           calling a global function. The name value is ignored by this
 **           function, but maybe needed in the future.
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
int chap_create_chal (CHAP_CONTEXT_ST *cc, UINT8 *buf, int *p_valid_len,
                      int encoding_base)
{
  /*
  ** get the next id.
  */
  cc->id = get_next_auth_id();

  /*
  ** create a challenge of the required size.
  */
  cc->chal_len = CHAP_MIN_CHAL_LEN;
  create_random_stream (cc->chal, cc->chal_len);

  /*
  ** clear the response recvd field.
  */
  cc->resp_recvd_len = 0;
  memset (cc->resp_recvd_encoded, 0, sizeof (cc->resp_recvd_encoded));

  chap_fill_challenge_buf (cc, buf, p_valid_len, encoding_base);

 cc->state = CHAP_CHAL_SENT;
  return XIO_SUCCESS;
}


/*
** @name chap_create_response
** @brief create chap_response in base64 encoding
**
*/

int chap_create_response(CHAP_CONTEXT_ST *cc, UINT8* buf, int *p_valid_len, int encoding_base UNUSED, UINT8 *targetName)
{

  /*
  ** + 1 is added to support for 16 Bytes Secret + NULL
  */
  UINT8 secret1[CHAP_MAX_SECRET_LEN + 1] = {0};
  UINT8 secret2[CHAP_MAX_SECRET_LEN + 1] = {0};

  chapGetSecret(cc->tid,cc->name,secret1,secret2);

  cc->resp_recvd_len = CHAP_MD5_DIGEST_LEN;
  compute_expected_resp(cc->resp_recvd_encoded, cc->resp_recvd_len,
                        cc->id_recvd, secret2, strlen((char *)secret2),
                        cc->chal_recvd_decoded, cc->chal_recvd_len);

// chap_fill_resp_buf(cc,buf,p_valid_len,encoding_base,targetName);
  chap_fill_resp_buf(cc,buf,p_valid_len,64,targetName);
  cc->state = CHAP_COMPLETE;
  return 1;
}


int chap_fill_resp_buf(CHAP_CONTEXT_ST *cc, UINT8 *buf, int *p_valid_len,
                       int encoding_base, UINT8* targetName)
{
  int   encoded_resp_len = CHAP_JUNK_INT;
  int   length = 0; /* To free encoded_resp */
  char *encoded_resp = (void *) CHAP_JUNK_POINTER;
#define RESP_FILL_format_str    "CHAP_N=%s%cCHAP_R=%s%s%c%c"
  char  pattern      = 'Z';    /* to help get size of buf */
  int   i = CHAP_JUNK_INT;
  char  base_type_str[3] =     /* to hold the 0x or 0b. */
    { CHAP_JUNK_CHAR,
      CHAP_JUNK_CHAR,
      CHAP_JUNK_CHAR
    };

  /*
  ** this is very safe. It takes into consideration, the string, and as a
  ** safe side, has the format specifiers included too! Also, the size of
  ** the challenge string can be twice as big as the challenge (in base16
  ** encoding). Leave space for a trailing zero to make it a proper C
  ** string.
  */
  encoded_resp_len = strlen (RESP_FILL_format_str) + 2 * cc->resp_recvd_len;

  /*
  ** get the length to allocate enceded_resp
  */
  length = (cc->resp_recvd_len* 2 + 1);

  encoded_resp = (char *) s_MallocC (length, __FILE__, __LINE__);

  switch (encoding_base)
    {
    case 16:
      binary_to_base16 (cc->resp_recvd_encoded, cc->resp_recvd_len, encoded_resp, &encoded_resp_len);
      strcpy (base_type_str, "0x");
      break;
    case 64:
      binary_to_base64 (cc->resp_recvd_encoded, cc->resp_recvd_len, encoded_resp, &encoded_resp_len);
      strcpy (base_type_str, "0b");
      break;
    default:
      break;
    }

  /*
  ** now, null terminate the encoded_chal.
  */
  encoded_resp[encoded_resp_len] = '\0';

  /*
  ** fill up the buffer. The memset routines helps to identify the last '>'
  ** character. This size of the message is this + 1 (for the terminating
  ** NUL).
  */
  memset (buf, 0, *p_valid_len);
  snprintf((char *)buf, *p_valid_len, RESP_FILL_format_str, targetName, '\0',
           base_type_str, encoded_resp, '\0', pattern);
  /*
  ** free encoded_resp
  */
  s_Free(encoded_resp,length, __FILE__, __LINE__);

  /*
  ** get the location of the last "pattern" char.
  */
  for (i = *p_valid_len - 1; i >= 0; i--)
  {
      if (buf[i])
      {
          /*
           ** we found the pattern. remove it and update valid len to
           ** return the right size of our string.
           */
          buf[i] = '\0';
          *p_valid_len = i;
          break;
      }
  }

  return 0;

}

/****************************************************************************
 ** NOTES:    compares the response expected and the response received.
 **
 **
 ** RETURNS:  TRUE if valid, FALSE otherwise.
 **
 ** good
 ****************************************************************************/
static int is_response_valid (CHAP_CONTEXT_ST *cc, UINT8 *bin, int bin_len UNUSED)
{
    int is_valid = CHAP_FALSE;

    /*
     ** memcmp returns zero if they match!
     */
    if (!memcmp (cc->resp_expected_bin, bin, sizeof (cc->resp_expected_len)))
    {
        is_valid = CHAP_TRUE;
    }
    else
    {
        fprintf(stderr,"Mismatch in CHAP Secret Comparision\n");
    }

    if(cc->chal_recvd_len == 0)
    {
        cc->state = CHAP_COMPLETE;
    }
    return is_valid;
}

/*
** @name decode_string
** brief This function decodes the string into binary values
**
*/
int decode_string(UINT8 *encoded, INT32 encoded_length ,UINT8* decoded,int *decoded_length )
{
  char  base_char;

  /*
  ** convert the response to binary; make sure you skip the first two bytes
  ** which will be of the form '0x', '0X', '0B' or '0b'.
  */

  base_char = tolower (encoded[1]);
  switch (base_char)
    {
    case 'x':
      base16_to_binary((char *)(encoded + 2), encoded_length -2, decoded, decoded_length);
      break;
    case 'b':
      base64_to_binary((char *)(encoded + 2), encoded_length -2, decoded, decoded_length);
      break;
    default:
      break;
    }

 return 1;
}


/****************************************************************************
 ** NOTES:    checks the response to see if it is valid.
 **           Expects the response from the peer in a relevant field in cc
 **           struct. Returns TRUE if response is okay, FALSE otherwise.
 **
 ** RETURNS:  TRUE/FALSE
 **
 **
 ****************************************************************************/
int chap_is_valid_resp (CHAP_CONTEXT_ST *cc)
{
  UINT8 bin[CHAP_MAX_RESP_EXPECTED_LEN];
  int   valid_bin_len;
  char  base_char;

  /*
  ** + 1 is added to support for 16 Bytes Secret + NULL
  */
  UINT8 secret1[CHAP_MAX_SECRET_LEN + 1] = {0};
  UINT8 secret2[CHAP_MAX_SECRET_LEN + 1] = {0};

  /*
  ** convert the response to binary; make sure you skip the first two bytes
  ** which will be of the form '0x', '0X', '0B' or '0b'.
  */
  valid_bin_len = sizeof (bin);
  base_char = tolower (cc->resp_recvd_encoded[1]);
  switch (base_char)
    {
    case 'x':
      base16_to_binary((char *)(cc->resp_recvd_encoded + 2), cc->resp_recvd_len - 2, bin, &valid_bin_len);
      break;
    case 'b':
      base64_to_binary((char *)(cc->resp_recvd_encoded + 2), cc->resp_recvd_len - 2, bin, &valid_bin_len);
      break;
    default:
      break;
    }

  /*
  ** now, check if the response received matches the one we are expecting.
  ** secret1 is to authenticate initiator
  */
  if(chapGetSecret(cc->tid,cc->name,secret1,secret2) == -1)
  {
    fprintf(stderr,"Secret not found for the user name %s\n",cc->name);
    return CHAP_FALSE;
  }

  cc->resp_expected_len = CHAP_MD5_DIGEST_LEN;
  compute_expected_resp(cc->resp_expected_bin, cc->resp_expected_len, cc->id,
                        secret1, strlen((char *)secret1), cc->chal, cc->chal_len);

  return is_response_valid (cc, bin, valid_bin_len);
}


/****************************************************************************
 ** NOTES:    tests the create_chal function
 **
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
static int chap_test_create_chal(CHAP_CONTEXT_ST *cc, UINT8 *buf,
                                 int *p_valid_len, int encoding_base)
{
  chap_create_chal (cc, buf, p_valid_len, encoding_base);
 // print_chap_context_st (cc);

  /*
  ** the buffer consists of three strings back to back, with an ascii NUL
  ** after each. So we have to print them carefully.
  */
  chap_log_mid("<%s><%s><%s>\n", (char *)buf, (char *)buf + strlen((char *)buf) + 1,
               (char *)buf + strlen((char *)buf) + 1 + strlen((char *)buf + strlen((char *)buf) + 1) + 1);

  return 0;
}




/****************************************************************************
 ** NOTES:    tests chap and prints results on the screen. Not a
 **           comprehensive test, but something's better than...
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
int chap_test (void)
{
  UINT8 buf[1024];
  CHAP_CONTEXT_ST *cc;
  int   valid_len;
  int   encoding_base;
  char *pfname = (char *) "chap_test";

  /*
  ** PART 1:
  ** chal; try with base = 16;
  */
  valid_len = sizeof (buf);
  encoding_base = 16;
  cc = chap_create_context_st ();
  cc->algorithm = CHAP_MD5_ALGORITHM_CODE;
  cc->name_len = CHAP_MAX_NAME_LEN;
  strncpy ((char *)cc->name, chap_name_str, cc->name_len);

  chap_test_create_chal (cc, buf, &valid_len, encoding_base);

  /*
  ** validate resp. Take the expected response, and store it in the recvd
  ** response field, as if we received a message from someone. This should,
  ** of course, be in either base16 or base64 encoding with a proper
  ** prefix.
  */
  cc->resp_recvd_len = sizeof (cc->resp_recvd_encoded);

  /*
  ** leave space for the prefix.
  */
  binary_to_base16(cc->resp_expected_bin, cc->resp_expected_len,
                   (char *)cc->resp_recvd_encoded + 2, &cc->resp_recvd_len);

  cc->resp_recvd_encoded[0] = '0';
  cc->resp_recvd_encoded[1] = 'x';
  cc->resp_recvd_len += 2;

  /*
  ** now, we have the encoded response in the in the correct field of the
  ** context struct. The result should be a SUCCESS!
  */
  chap_log_mid("%s: result = %s\n", pfname,
               chap_is_valid_resp (cc) ? "SUCCESS" : "FAILURE");

  chap_release_context_st (cc);

  /*
  ** PART 2:
  ** chal; now repeat with base = 64;
  */
  valid_len = sizeof (buf);
  encoding_base = 64;
  cc = chap_create_context_st ();
  cc->algorithm = CHAP_MD5_ALGORITHM_CODE;
  cc->name_len = CHAP_MAX_NAME_LEN;
  strncpy((char *)cc->name, chap_name_str, cc->name_len);

  chap_test_create_chal (cc, buf, &valid_len, encoding_base);

  /*
  ** validate resp. Take the expected response, and store it in the recvd
  ** response field, as if we received a message from someone. This should,
  ** of course, be in either base16 or base64 encoding with a proper
  ** prefix.
  */
  cc->resp_recvd_len = sizeof (cc->resp_recvd_encoded);

  /*
  ** leave space for the prefix.
  */
  binary_to_base64(cc->resp_expected_bin, cc->resp_expected_len,
                   (char *)cc->resp_recvd_encoded + 2, &cc->resp_recvd_len);

  cc->resp_recvd_encoded[0] = '0';
  cc->resp_recvd_encoded[1] = 'b';
  cc->resp_recvd_len += 2;

  /*
  ** now, we have the encoded response in the in the correct field of the
  ** context struct. The result should be a SUCCESS!
  */
  chap_log_mid("%s: result = %s\n", pfname,
               chap_is_valid_resp (cc) ? "SUCCESS" : "FAILURE");

  chap_release_context_st (cc);

  return 0;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
