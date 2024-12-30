/* $Id: SerConNetwork.c 158811 2011-12-20 20:42:56Z m4 $ */
/*============================================================================
** FILE NAME:       SerConNetwork.c
** MODULE TITLE:    Serial Console Network Frames
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include <errno.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <byteswap.h>
#include "SerConNetwork.h"
#include "SerCon.h"
#include "SerConTime.h"
#include "debug_files.h"
#include "kernel.h"
#include "quorum.h"
#include "i82559.h"
#include "misc.h"
#include "names.h"
#include "nvram.h"
#include "PortServer.h"
#include "PortServerUtils.h"
#include "serial.h"
#include "convert.h"
#include "serial_num.h"
#include "debug_struct.h"
#include "errorCodes.h"
#include "quorum_utils.h"
#include "SerBuff.h"
#include "mode.h"
#include "timer.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "PacketInterface.h"
#include "PI_PDisk.h"
#include "PI_Utils.h"
#include "ipc_session_manager.h"
#include "CacheManager.h"
#include "cps_init.h"
#include "sm.h"
#include "xssa_structure.h"
#include "error_handler.h"

#include "L_Misc.h"

extern INT32 MfgCtrlClean_WriteSame(UINT64, UINT16);

/*****************************************************************************
** Private defines
*****************************************************************************/
#define TMO_PI_PROC_RESETQLOGIC_CMD     20000   /* 20  second tmo   */

/*****************************************************************************
** Public routines not defined in any header file
*****************************************************************************/
extern void Clean_Controller(int, UINT8);
extern void Lock_Shutdown_FE(void);
extern void ConfigCtrl(TASK_PARMS *parms);
extern void SetConfigEthernet(void);
extern void GotoFirstFrame(void);
extern void BadInput(void);

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static unsigned char TestAddress(char *StringAddress, unsigned char length);
static void AdminIPAddrFrameDisplayFunction(void);
static void AdminIPAddrFrameResponseFunction(void);
static void AdminSubnetMaskFrameDisplayFunction(void);
static void AdminSubnetMaskFrameResponseFunction(void);
static void AdminGatewayAddrFrameDisplayFunction(void);
static void AdminGatewayAddrFrameResponseFunction(void);
static void RemoveCR(char *InBuffer, char *OutBuffer);
static void ControllerSNDisplayFunction(void);
static void ControllerSNResponseFunction(void);
static void CncIdDisplayFunction(void);
static void CncIdResponseFunction(void);
static void CnReplacePromptFunction(void);
static void CnReplaceResponseFunction(void);
static void ApplyPromptFunction(void);
static void ApplyResponseFunction(void);
static void ApplyUpdates(void);
static void AlreadyConfiguredPromptFunction(void);
static void AlreadyConfiguredResponseFunctionR(void);
static void AlreadyConfiguredResponseFunctionC(void);
static void AlreadyConfiguredResponseFunctionZ(void);
static void AlreadyConfiguredResponseFunctionP(void);
static void AlreadyConfiguredResponseFunctionA(void);
static void AlreadyConfiguredResponseFunctionF(void);
static void AlreadyConfiguredResetPromptFunction(void);
static void AlreadyConfiguredResetResponseFunction(void);
static void AlreadyConfiguredIPPromptFunction(void);
static void AlreadyConfiguredIPResponseFunction(void);
static void AlreadyConfiguredZEROPromptFunction(void);
static void AlreadyConfiguredZEROPromptFunctionPre(void);
static void AlreadyConfiguredZEROResponseFunctionPre(void);

/* void AlreadyConfiguredPREPPromptFunctionPre(void); -- same as AlreadyConfiguredZEROResponseFunction */
static void AlreadyConfiguredZEROResponseFunction(void);
static void AlreadyConfiguredPREPResponseFunctionPre(void);
static void AlreadyConfiguredPREPPromptFunction(void);
static void AlreadyConfiguredPREPResponseFunction(void);
static void PREPPromptFunction(void);
static void PREPResponseFunction(void);
static void PREPZeroDISK(void);
static void ZEROTieUpConsolePromptFunction(void);
static void ZEROTieUpConsoleResponseFunction(void);
static void GotoPostFCConfig(void);

static void GotoBEFCConfig(void);
static void GotoFEFCConfig(void);
static void FCConfigPromptDisplay(void);
static void FCConfigDisplay(void);
static void FCConfigInput(void);
static void ToggleDiagPortsDisplayFunction(void);
static void ToggleDiagPortsResponseFunction(void);

/*****************************************************************************
** Private variables
*****************************************************************************/
static unsigned char badInput = FALSE;
static UINT32 CurrentAddress;
static unsigned char needinput = FALSE;
static UINT32 gIpAddress;
static UINT32 gSubnetAddress;
static UINT32 gGatewayAddress;
static UINT32 gCncId;
static UINT32 gCnId;
static bool gReplacementFlag;
static bool gCleanControllerFlag;
static bool gConfiguredIPChangeFlag;
static PORT_CONFIG fc_config;
static ISP_CONFIG *fc_setup;
static const char *fc_name;

static CONSOLEFRAME FCConfigPrompt = {
    FCConfigPromptDisplay,      /* Start function */
    NULL,                       /* Finish function */
    5,                          /* # of choices */
    {
      {'B', GotoBEFCConfig},
      {'F', GotoFEFCConfig},
      {'Q', GotoPostFCConfig},
      {CRLF, GotoPostFCConfig},
      {'\0', BadInput},
    },
    /* Strings */
    {
      {'$', 0},
    }
};

static CONSOLEFRAME FCConfig = {
    FCConfigDisplay,            /* Start function */
    NULL,                       /* Finish function */
    2,                          /* # of choices */
    {
      {'Q', GotoFirstFrame},
      {'\0', FCConfigInput},
    },
    /* Strings */
    {
      {'$', 0},
    }
};

static CONSOLEFRAME AdminGatewayAddrFrame = {
    AdminGatewayAddrFrameDisplayFunction,       /* Start function   */
    NULL,                       /* Finish function  */
    2,                          /* # of choices     */
    /* Choice  */
    {
      {'Q', GotoFirstFrame},
      {'\0', AdminGatewayAddrFrameResponseFunction}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

static CONSOLEFRAME AdminSubnetMaskFrame = {
    AdminSubnetMaskFrameDisplayFunction,        /* Start function   */
    NULL,                       /* Finish function  */
    2,                          /* # of choices     */
    /* Choice */
    {
      {'Q', GotoFirstFrame},
      {'\0', AdminSubnetMaskFrameResponseFunction}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

static CONSOLEFRAME AdminIPAddrFrame = {
    AdminIPAddrFrameDisplayFunction,    /* Start function   */
    NULL,                       /* Finish function  */
    2,                          /* # of choices     */
    /* Choice */
    {
      {'Q', GotoFirstFrame},
      {'\0', AdminIPAddrFrameResponseFunction}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Controller S/N FRAME */
static CONSOLEFRAME EnterControllerSNFrame = {
    ControllerSNDisplayFunction,        /* Start function   */
    NULL,                       /* Finish function  */
    2,                          /* # of choices     */
    /* Choice */
    {
      {'Q', GotoFirstFrame},
      {'\0', ControllerSNResponseFunction}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Controller Node Cluster ID FRAME */
static CONSOLEFRAME EnterCncIdFrame = {
    CncIdDisplayFunction,       /* Start function   */
    NULL,                       /* Finish function  */
    2,                          /* # of choices     */
    /* Choice */
    {
      {'Q', GotoFirstFrame},
      {'\0', CncIdResponseFunction}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Controller Node Cluster replacement? Default No. */
static CONSOLEFRAME EnterCnReplaceFrame = {
    CnReplacePromptFunction,    /* Start function   */
    NULL,                       /* Finish function  */
    5,                          /* # of choices     */
    /* Choice */
    {
      {'Q', GotoFirstFrame},
      {'Y', CnReplaceResponseFunction},
      {'N', CnReplaceResponseFunction},
      {CRLF, CnReplaceResponseFunction},
      {'\0', BadInput}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Are you sure? Prompt Frame. Default No. */
static CONSOLEFRAME ApplyAreYouSureFrame = {
    ApplyPromptFunction,        /* Start function   */
    NULL,                       /* Finish function  */
    5,                          /* # of choices     */
    /* Choice */
    {
      {'N', GotoFirstFrame},
      {'Q', GotoFirstFrame},
      {'Y', ApplyResponseFunction},
      {CRLF, GotoFirstFrame},
      {'\0', BadInput}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* The controller is already configured, do what? */
static CONSOLEFRAME AlreadyConfiguredFrame = {
    AlreadyConfiguredPromptFunction,    /* Start function   */
    NULL,                       /* Finish function  */
    8,                          /* # of choices     */
    /* Choice */
    {
      {'Q', GotoFirstFrame},
      {'R', AlreadyConfiguredResponseFunctionR},
      {'C', AlreadyConfiguredResponseFunctionC},
      {'Z', AlreadyConfiguredResponseFunctionZ},
      {'P', AlreadyConfiguredResponseFunctionP},
      {'A', AlreadyConfiguredResponseFunctionA},
      {'F', AlreadyConfiguredResponseFunctionF},
      {'\0', BadInput}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Are you sure? Prompt Frame (default N) */
static CONSOLEFRAME AlreadyConfiguredResetFrame = {
    AlreadyConfiguredResetPromptFunction,       /* Start function   */
    NULL,                       /* Finish function  */
    5,                          /* # of choices     */
    /* Choice */
    {
      {'N', GotoFirstFrame},
      {'Q', GotoFirstFrame},
      {'Y', AlreadyConfiguredResetResponseFunction},
      {CRLF, GotoFirstFrame},
      {'\0', BadInput}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Are you sure? Prompt Frame (default N) */
static CONSOLEFRAME AlreadyConfiguredIPFrame = {
    AlreadyConfiguredIPPromptFunction,  /* Start function   */
    NULL,                       /* Finish function  */
    5,                          /* # of choices     */
    /* Choice */
    {
      {'N', GotoFirstFrame},
      {'Q', GotoFirstFrame},
      {'Y', AlreadyConfiguredIPResponseFunction},
      {CRLF, GotoFirstFrame},
      {'\0', BadInput}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Command can cause data loss if misused, abort?. Default Y (zero) */
static CONSOLEFRAME AlreadyConfiguredZEROFramePre = {
    AlreadyConfiguredZEROPromptFunctionPre,     /* Start function   */
    NULL,                       /* Finish function  */
    5,                          /* # of choices     */
    /* Choice */
    {
      {'N', AlreadyConfiguredZEROResponseFunctionPre},
      {'Q', GotoFirstFrame},
      {'Y', GotoFirstFrame},
      {CRLF, GotoFirstFrame},
      {'\0', BadInput}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Are you sure? Prompt Frame.  Default N. */
static CONSOLEFRAME AlreadyConfiguredZEROFrame = {
    AlreadyConfiguredZEROPromptFunction,        /* Start function   */
    NULL,                       /* Finish function  */
    5,                          /* # of choices     */
    /* Choice */
    {
      {'N', GotoFirstFrame},
      {'Q', GotoFirstFrame},
      {'Y', AlreadyConfiguredZEROResponseFunction},
      {CRLF, GotoFirstFrame},
      {'\0', BadInput}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Command can cause data loss if misused, abort? Default Y. (prepare) */
static CONSOLEFRAME AlreadyConfigurePREPFramePre = {
    AlreadyConfiguredZEROPromptFunctionPre,     /* Start function   */
    NULL,                       /* Finish function  */
    5,                          /* # of choices     */
    /* Choice */
    {
      {CRLF, GotoFirstFrame},
      {'N', AlreadyConfiguredPREPResponseFunctionPre},
      {'Q', GotoFirstFrame},
      {'Y', GotoFirstFrame},
      {'\0', BadInput}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Which disk do you wish to destroy? Default N. */
static CONSOLEFRAME AlreadyConfiguredPREPFrame = {
    AlreadyConfiguredPREPPromptFunction,        /* Start function   */
    NULL,                       /* Finish function  */
    3,                          /* # of choices     */
    /* Choice */
    {
      {'Q', GotoFirstFrame},
      {'N', GotoFirstFrame},
      {'\0', AlreadyConfiguredPREPResponseFunction}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Do you really wish to destroy the customers disk? Default N. */
static CONSOLEFRAME PREPFrame = {
    PREPPromptFunction,         /* Start function   */
    NULL,                       /* Finish function  */
    5,                          /* # of choices     */
    /* Choice */
    {
      {'Q', GotoFirstFrame},
      {'N', GotoFirstFrame},
      {'Y', PREPResponseFunction},
      {CRLF, GotoFirstFrame},
      {'\0', BadInput}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Do not allow anything Frame. */
static CONSOLEFRAME ZEROTieUpConsoleFrame = {
    ZEROTieUpConsolePromptFunction,     /* Start function   */
    NULL,                       /* Finish function  */
    2,                          /* # of choices     */
    /* Choice */
    {
      {'Q', GotoFirstFrame},
      {'\0', ZEROTieUpConsoleResponseFunction}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Turn on ccbe port. */
static CONSOLEFRAME ToggleDiagPortFrame = {
    ToggleDiagPortsDisplayFunction,     /* Start function   */
    NULL,                       /* Finish function  */
    1,                          /* # of choices     */
    /* Choice  */
    {
      {'\0', ToggleDiagPortsResponseFunction}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/*****************************************************************************
** Code Start
*****************************************************************************/

void SetConfigEthernet(void)
{
    UpdateIfcfgScript(gIpAddress, gSubnetAddress, gGatewayAddress);
    SetEthernetConfigured(1);
}

/* ------------------------------------------------------------------------ */

/*
 *  Validate the format of the address.
 */
unsigned char TestAddress(char *StringAddress, unsigned char length)
{
    char        str[20] = { 0 };
    UINT8       rc = TRUE;

    strncpy(str, StringAddress, length > 16 ? 16 : length);

    if (inet_addr(str) == (UINT32)PI_SOCKET_ERROR)
    {
        rc = FALSE;
    }

    return rc;
}

/* ------------------------------------------------------------------------ */
unsigned char ShowEthernet(unsigned char LineNumber)
{
    UINT32      Address;
    unsigned long ErrCode = 0;
    char        Buffer[18];

    ETHERNET_MAC_ADDRESS ethernetMACAddress;

    /*
     * Display MAC address
     */
    ethernetMACAddress = GetMacAddrFromInterface(ethernetDriver.interfaceHandle);

    sprintf(currentFramePtr->line[LineNumber++],
            "\r\nMAC:              %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
            ethernetMACAddress.macByte[0], ethernetMACAddress.macByte[1],
            ethernetMACAddress.macByte[2], ethernetMACAddress.macByte[3],
            ethernetMACAddress.macByte[4], ethernetMACAddress.macByte[5]);

    if (ethernetDriver.interfaceHandle != NULL)
    {
        /* Get and format IP */
        Address = GetIPAddress();
        InetToAscii(Address, Buffer);
        sprintf(currentFramePtr->line[LineNumber++], "\r\nIP:               %s", Buffer);

        /* Get and format the netmask */
        Address = GetSubnetMask();
        InetToAscii(Address, Buffer);
        sprintf(currentFramePtr->line[LineNumber++], "\r\nSubnet/mask:      %s", Buffer);

        /* Get and format the Default Gateway */
        Address = GetGatewayAddress();

        if (Address == 0)
        {
            ErrCode = 1;
            InetToAscii(GetGatewayAddress(), Buffer);
        }
        else
        {
            InetToAscii(Address, Buffer);
        }
        sprintf(currentFramePtr->line[LineNumber++], "\r\nGateway:          %s", Buffer);

        if (ErrCode && GetGatewayAddress())
        {
            sprintf(currentFramePtr->line[LineNumber++], " (unreachable)");
        }
    }
    else
    {
        sprintf(currentFramePtr->line[LineNumber++], "\r\nEthernet not yet available");
        sprintf(currentFramePtr->line[LineNumber++], " ");
        sprintf(currentFramePtr->line[LineNumber++], " ");
        sprintf(currentFramePtr->line[LineNumber++], " ");
    }

    return (LineNumber);
}

/* ------------------------------------------------------------------------ */
void RemoveCR(char *InBuffer, char *OutBuffer)
{
    unsigned char count = 0;

    while ((*(InBuffer + count) != CRLF) && (count < 15))
    {
        *(OutBuffer + count) = *(InBuffer + count);
        count++;
    }

    *(OutBuffer + count) = '\0';
}


/*----------------------------------------------------------------------------
** Function:    Controller Setup Frame Choice
**
** Description: Called when the user selects to configure a controller. If
**              the controller was previously configured,
**                   Next frane = Already Configured Controller prompt.
**              otherwise,
**                   Next frane = IP Address prompt.
**
**
**--------------------------------------------------------------------------*/
void ControllerSetupFrameChoiceFunction(void)
{
    /*
     * Initialize global flags
     */
    gCleanControllerFlag = false;
    gConfiguredIPChangeFlag = false;
    fc_config = cntlSetup.config;

    /*
     * If the controller is not already set up, proceed to the IP admin
     * frame, otherwise see if the user wants to continue.
     */
    if (!IsControllerSetup())
    {
        currentFramePtr = &FCConfigPrompt;
    }
    else
    {
        currentFramePtr = &AlreadyConfiguredFrame;
    }
}


/*----------------------------------------------------------------------------
** Function:    ProcessBadInputAddress
**
** Description: Warns the user that input address was invalid, try again.
**--------------------------------------------------------------------------*/

static void ProcessBadInputAddress(unsigned char *LineNumber)
{
    if (badInput == TRUE)
    {
        sprintf(currentFramePtr->line[(*LineNumber)++], "\r\n\nInvalid address; try again.");
    }
    badInput = FALSE;
}


/*----------------------------------------------------------------------------
** Function:    ProcessBadInput
**
** Description: Warns the user that last input was unrecognized, try again.
**--------------------------------------------------------------------------*/

static void ProcessBadInput(unsigned char *LineNumber)
{
    if (badInput == TRUE)
    {
        sprintf(currentFramePtr->line[(*LineNumber)++], "\r\n\nInvalid Entry; try again.");
    }
    badInput = FALSE;
}


/*----------------------------------------------------------------------------
** Function:    Already Configured Controller Prompt and Response
**
** Description: Warns the user that this controller was previously configured
**              prompts them to determine if they want to reset the
**              controller to default values or to change the IP address.
**              [Z]     Remove procdata/shared_memory_NVRAM_{B,F}E,
**                      ccbdata/{CCB_FLASH,CCB_NVRAM,RAIDMON}.mmf
**                      and restart controller.
**
**              [P]     Prepare (destroy) 1 pdisk, then zero controller (above).
**
**              [R]     set global flag to clean controller.
**                      Next Frame = IP Address Prompt
**
**              [C]     Next Frame = Controller Configured Address change
**
**              [F]     Next Frame = FC Port configuration
**
**              [Q]     Next Frame = First Frame
**
**--------------------------------------------------------------------------*/
void AlreadyConfiguredPromptFunction(void)
{
    unsigned char LineNumber = 0;

    ProcessBadInput(&LineNumber);

    /*
     * Display the already configured menu options
     */
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nWARNING: THIS CONTROLLER IS ALREADY SETUP.");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n(Z)ero Controller Completely to it's Initial State");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n(P)repare [DESTROY] a single Disk and Controller to it's Initial State");
#ifdef EXTRAOPTIONSDISPLAY
    sprintf(currentFramePtr->line[LineNumber++], "\r\n(A)utoconfigure Customer Disks");
#endif /* EXTRAOPTIONSDISPLAY */
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n(R)eset Controller To Defaults");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n(C)hange IP Address");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n(F)C Port configure");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n(Q)uit");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nSelect?: ");
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/* ------------------------------------------------------------------------ */
void AlreadyConfiguredResponseFunctionR(void)
{
    currentFramePtr = &AlreadyConfiguredResetFrame;
}

/* ------------------------------------------------------------------------ */
void AlreadyConfiguredResponseFunctionC(void)
{
    currentFramePtr = &AlreadyConfiguredIPFrame;
}

/* ------------------------------------------------------------------------ */
void AlreadyConfiguredResponseFunctionZ(void)
{
    currentFramePtr = &AlreadyConfiguredZEROFramePre;
}

/* ------------------------------------------------------------------------ */
void AlreadyConfiguredResponseFunctionP(void)
{
    currentFramePtr = &AlreadyConfigurePREPFramePre;
}

/* ------------------------------------------------------------------------ */
void AlreadyConfiguredResponseFunctionA(void)
{
/* if not present, re-display current frame. */
}

/* ------------------------------------------------------------------------ */
void AlreadyConfiguredResponseFunctionF(void)
{
    currentFramePtr = &FCConfigPrompt;
}

/*----------------------------------------------------------------------------
** Function:    Reset Controller Defaults Prompt and Response
**
** Description: Warns the user that this controller was previously configured
**              prompts them to determine if they want to reset the defaults.
**              [Y]     set global flag to change controller map addresses.
**                      Next Frame = IP Address Prompt
**              [N]     no action taken.
**                      Next Frame = Main User Prompt
**
**              default [N]
**
**--------------------------------------------------------------------------*/
void AlreadyConfiguredResetPromptFunction(void)
{
    unsigned char LineNumber = 0;

    ProcessBadInput(&LineNumber);

    /*
     * Display the controller node cluster
     */
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nWARNING: RESETTING CONTROLLER TO DEFAULTS");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         CAN CAUSE LOSS OF DATA.");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nDO YOU WANT TO RESET CONTROLLER TO DEFAULTS?");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nContinue Y/N ?: [N] ");
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/* ------------------------------------------------------------------------ */
void AlreadyConfiguredResetResponseFunction(void)
{
    /*
     * This controller was already configured, but we want to
     * continue anyway. Set a flag to ensure the controller is
     * cleaned before setting up.
     */
    gCleanControllerFlag = true;
    currentFramePtr = &FCConfigPrompt;
}


/*----------------------------------------------------------------------------
** Function:    Change Configured Controller Addresses Prompt and Response
**
** Description: Warns the user that this controller was previously configured
**              prompts them to determine if they want to change IP addresses.
**              [Y]     set global flag to change controller map addresses.
**                      Next Frame = IP Address Prompt
**              [N]     no action taken.
**                      Next Frame = Main User Prompt
**
**              default [N]
**
**--------------------------------------------------------------------------*/
void AlreadyConfiguredIPPromptFunction(void)
{
    unsigned char LineNumber = 0;

    ProcessBadInput(&LineNumber);

    /*
     * Display the controller node cluster
     */
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nWARNING: CHANGING THE IP ADDRESS OF A");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         CONTROLLER CAN CAUSE THE LOSS");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         OF CONNECTIVITY TO THE ICON OR");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         OTHER CONTROLLERS.");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         BEFORE CHANGING THE IP ADDRESS");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         ALL OTHER CONTROLLERS IN THE");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         DSC SHOULD BE POWERED OFF.");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         CONTROLLER SHOULD HAVE VALID");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         CONNECTION TO LABELED DRIVES");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nDO YOU WANT TO CHANGE IP ADDRESSES?");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nContinue Y/N ?: [N] ");
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/*----------------------------------------------------------------------------
** Function:    AlreadyConfiguredZEROPromptFunctionPre
**
** Description: This may cause data lass.
**              [Y]     Abort this function.
**              [N]     Continue to next warning.
**              default [Y]
**
**--------------------------------------------------------------------------*/

void AlreadyConfiguredZEROPromptFunctionPre(void)
{
    unsigned char LineNumber = 0;

    ProcessBadInput(&LineNumber);

    /*
     * Display the controller node cluster
     */
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nWarning: This command can cause data loss if misused.");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nAbort this function?  Y/N ?: [Y] ");
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/*----------------------------------------------------------------------------
** Function:    AlreadyConfiguredZEROPromptFunction
**
** Description: Warns the user that this controller was previously configured.
**              Prompts them to determine if they want to ZERO it out.
**              [Y]     set global flag to zero controller information.
**                      Next Frame = Last Chance before doing it.
**              [N]     no action taken.
**                      Next Frame = Main User Prompt
**
**              default [N]
**
**--------------------------------------------------------------------------*/
void AlreadyConfiguredZEROPromptFunction(void)
{
    unsigned char LineNumber = 0;

    ProcessBadInput(&LineNumber);

    /*
     * Display the controller node cluster
     */
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nWARNING: THIS WILL COMPLETELY CLEAR THE CONTROLLER");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         AND WILL CAUSE THE LOSS OF CONNECTIVITY");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         TO THE ICON AND OTHER CONTROLLERS.");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         THIS IS YOUR LAST CHANCE.");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nDO YOU WANT TO ZERO THE CONTROLLER INFORMATION?");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nContinue Y/N ?: [N] ");
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/*----------------------------------------------------------------------------
** Function:    PREPPromptFunction
**
** Description: Warns the user that this is dangerous.
**              [Y]     Zero the disk.
**                      Next Frame = Main User Prompt
**              [N]     No action taken.
**                      Next Frame = Main User Prompt
**
**              default [N]
**
**--------------------------------------------------------------------------*/
static unsigned int gpdisk;

void PREPPromptFunction(void)
{
    unsigned char LineNumber = 0;

    ProcessBadInput(&LineNumber);

    /*
     * Display the controller node cluster
     */
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nWARNING: PREPARING CUSTOMER DISK #%d WILL LOSE INFORMATION.", gpdisk);
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         THIS WILL COMPLETELY CLEAR THE CONTROLLER");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         AND WILL CAUSE THE LOSS OF CONNECTIVITY");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         TO THE ICON AND OTHER CONTROLLERS.");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         THIS IS YOUR LAST CHANCE.");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nDO YOU REALLY WANT TO DESTROY CUSTOMER'S DISK #%u AND CLEAR THE CONTROLLER?",
            gpdisk);
    sprintf(currentFramePtr->line[LineNumber++], "\r\nContinue Y/N ?: [N] ");
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/*--------------------------------------------------------------------------*/
static char string_pd[4];

static char *string_pdname(int bay, int slot)
{
    if (bay < 26)
    {
        string_pd[0] = 'A' + bay;
    }
    else if (bay < (26 + 26))
    {
        string_pd[0] = 'a' + bay - 26;
    }
    else
    {
        string_pd[0] = '?';
    }
    if (slot < 100)
    {
        string_pd[1] = '0' + (slot / 10);
        string_pd[2] = '0' + (slot % 10);
    }
    else
    {
        string_pd[1] = '*';
        string_pd[2] = '*';
    }
    string_pd[3] = 0;
    return (string_pd);
}


/*----------------------------------------------------------------------------
** Function:    AlreadyConfiguredPREPPromptFunction
**
** Description: Warns the user that this controller was previously configured.
**              Asks which disk they wish to zero.
**                      set global flag to zero disk drive.
**              [N]     no action taken.
**                      Next Frame = Main User Prompt
**
**              default [N]
**
**--------------------------------------------------------------------------*/
void AlreadyConfiguredPREPPromptFunction(void)
{
    unsigned char LineNumber = 0;
    PI_PDISKS_RSP *pPDisks = NULL;
    UINT16      count = 0;
    int         lth;
    int         slot;
    int         bay;

    ProcessBadInput(&LineNumber);

    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nHere are your disks:");

    /* Printout physical disk list. */

    pPDisks = PhysicalDisks();
    if (pPDisks)
    {
        lth = snprintf(currentFramePtr->line[LineNumber], CONSOLE_COLUMNS - 1, "\r\nUNLABELED:");
        for (count = 0; count < pPDisks->count; count++)
        {
            if (pPDisks->pdiskInfo[count].pdd.devClass == PD_UNLAB &&
                (pPDisks->pdiskInfo[count].pdd.devStat == PD_OP ||
                 (pPDisks->pdiskInfo[count].pdd.devStat == PD_INOP &&
                  pPDisks->pdiskInfo[count].pdd.postStat == PD_FDIR)))
            {
                bay = pPDisks->pdiskInfo[count].pdd.devName[PD_DNAME_CSES];
                slot = pPDisks->pdiskInfo[count].pdd.devName[PD_DNAME_CSLOT];
                /* Not too many characters in line. */
                if (lth > (CONSOLE_COLUMNS - (1 + 5)))
                {
                    if (LineNumber > 5)
                    {
                        snprintf(currentFramePtr->line[LineNumber] + lth,
                                 CONSOLE_COLUMNS - 1, " ...");
                        break;
                    }
                    LineNumber++;
                    lth = snprintf(currentFramePtr->line[LineNumber], CONSOLE_COLUMNS - 1, "\r\n          ");
                }
                lth += snprintf(currentFramePtr->line[LineNumber] + lth, CONSOLE_COLUMNS - 1, " %u", count);
            }
        }
        LineNumber++;
        lth = snprintf(currentFramePtr->line[LineNumber], CONSOLE_COLUMNS - 1, "\r\nHOTSPARE:");
        for (count = 0; count < pPDisks->count; count++)
        {
            if (pPDisks->pdiskInfo[count].pdd.devClass == PD_HOTLAB &&
                pPDisks->pdiskInfo[count].pdd.devStat == PD_OP)
            {
                bay = pPDisks->pdiskInfo[count].pdd.devName[PD_DNAME_CSES];
                slot = pPDisks->pdiskInfo[count].pdd.devName[PD_DNAME_CSLOT];
                /* Not too many characters in line. */
                if (lth > (CONSOLE_COLUMNS - (1 + 7)))
                {
                    if (LineNumber > 5 + 3)
                    {
                        snprintf(currentFramePtr->line[LineNumber] + lth,
                                 CONSOLE_COLUMNS - 1, " ...");
                        break;
                    }
                    LineNumber++;
                    lth = snprintf(currentFramePtr->line[LineNumber], CONSOLE_COLUMNS - 1, "\r\n         ");
                }
                lth += snprintf(currentFramePtr->line[LineNumber] + lth, CONSOLE_COLUMNS - 1, " %u-%s", count, string_pdname(bay, slot));
            }
        }
        LineNumber++;
        lth = snprintf(currentFramePtr->line[LineNumber], CONSOLE_COLUMNS - 1, "\r\nUNSAFE:");
        for (count = 0; count < pPDisks->count; count++)
        {
            if (pPDisks->pdiskInfo[count].pdd.devClass == PD_NDATALAB &&
                pPDisks->pdiskInfo[count].pdd.devStat == PD_OP)
            {
                bay = pPDisks->pdiskInfo[count].pdd.devName[PD_DNAME_CSES];
                slot = pPDisks->pdiskInfo[count].pdd.devName[PD_DNAME_CSLOT];
                /* Not too many characters in line. */
                if (lth > (CONSOLE_COLUMNS - (1 + 7)))
                {
                    if (LineNumber > 5 + 3 + 3)
                    {
                        snprintf(currentFramePtr->line[LineNumber] + lth, CONSOLE_COLUMNS - 1, " ...");
                        break;
                    }
                    LineNumber++;
                    lth = snprintf(currentFramePtr->line[LineNumber], CONSOLE_COLUMNS - 1, "\r\n       ");
                }
                lth += snprintf(currentFramePtr->line[LineNumber] + lth, CONSOLE_COLUMNS - 1, " %u-%s", count, string_pdname(bay, slot));
            }
        }
        LineNumber++;
        lth = snprintf(currentFramePtr->line[LineNumber], CONSOLE_COLUMNS - 1, "\r\nDATA:");
        for (count = 0; count < pPDisks->count; count++)
        {
            if (pPDisks->pdiskInfo[count].pdd.devClass == PD_DATALAB &&
                pPDisks->pdiskInfo[count].pdd.devStat == PD_OP)
            {
                bay = pPDisks->pdiskInfo[count].pdd.devName[PD_DNAME_CSES];
                slot = pPDisks->pdiskInfo[count].pdd.devName[PD_DNAME_CSLOT];
                /* Not too many characters in line. */
                if (lth > (CONSOLE_COLUMNS - (1 + 7)))
                {
                    if (LineNumber > 5 + 3 + 3 + 3)
                    {
                        snprintf(currentFramePtr->line[LineNumber] + lth, CONSOLE_COLUMNS - 1, " ...");
                        break;
                    }
                    LineNumber++;
                    lth = snprintf(currentFramePtr->line[LineNumber], CONSOLE_COLUMNS - 1, "\r\n     ");
                }
                lth += snprintf(currentFramePtr->line[LineNumber] + lth, CONSOLE_COLUMNS - 1, " %u-%s", count, string_pdname(bay, slot));
            }
        }
        LineNumber++;
        Free(pPDisks);
    }
    else
    {
        sprintf(currentFramePtr->line[LineNumber++], "\r\nWARNING: NO PHYSICAL DISKS FOUND.");
    }

    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nWARNING: DESTROYING A CUSTOMER'S DISK COULD RESULT");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n         IN DATA LOSS!");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nENTER THE DISK NUMBER TO DESTROY: [N] ");
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/*--------------------------------------------------------------------------*/
void AlreadyConfiguredIPResponseFunction(void)
{
    /*
     * This controller was already configured, but we want to
     * continue anyway. Set a flag to ensure the controller is
     * cleaned before setting up.
     */
    gConfiguredIPChangeFlag = true;
    currentFramePtr = &AdminIPAddrFrame;
}


/*--------------------------------------------------------------------------*/
void AlreadyConfiguredZEROResponseFunctionPre(void)
{
    currentFramePtr = &AlreadyConfiguredZEROFrame;
}


/*--------------------------------------------------------------------------*/
void AlreadyConfiguredZEROResponseFunction(void)
{
    /*
     * This controller was already configured, but we want to
     * continue anyway. Set flag for input and Zero Controller.
     */
    currentFramePtr = &ZEROTieUpConsoleFrame;
    /* 1 - Lock and shutdown FE. */
    Clean_Controller(1, MFG_CTRL_CLEAN_OPT_SERIAL_MESSAGES |
                     MFG_CTRL_CLEAN_OPT_NO_LOG_CLEAR | MFG_CTRL_CLEAN_OPT_POWER_DOWN);
}


/*--------------------------------------------------------------------------*/
void ZEROTieUpConsolePromptFunction(void)
{
    unsigned char LineNumber = 0;

    ProcessBadInput(&LineNumber);

    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
}


/*--------------------------------------------------------------------------*/
void ZEROTieUpConsoleResponseFunction(void)
{
    badInput = TRUE;
}


/*--------------------------------------------------------------------------*/
void GotoFirstFrame(void)
{
    currentFramePtr = &FirstFrame;
}


/*****************************************************************************
**
**  @brief  Go to BE FC config frame
**
**  @param  none
**
**  @return none
**
*****************************************************************************
*/
static void GotoBEFCConfig(void)
{
    fc_setup = &fc_config.be;
    fc_name = "BE";
    currentFramePtr = &FCConfig;
}


/*****************************************************************************
**
**  @brief  Go to FE FC config frame
**
**  @param  none
**
**  @return none
**
*****************************************************************************
*/
static void GotoFEFCConfig(void)
{
    fc_setup = &fc_config.fe;
    fc_name = "FE";
    currentFramePtr = &FCConfig;
}


/*****************************************************************************
**
**  @brief  Reset QLogic
**
**  @param  proc - Processor to send command to
**
**  @return none
**
*****************************************************************************
*/
static void ResetQLogic(UINT8 proc)
{
    UINT32      cmd;
    UINT32      rc;
    UINT32      timeout = MRP_STD_TIMEOUT;
    MRRESETPORT_REQ *req;
    MRRESETPORT_RSP *rsp;
    const char *pname;

    switch (proc)
    {
        case PROCESS_BE:
            timeout = MAX(timeout, TMO_PI_PROC_RESETQLOGIC_CMD);
            cmd = MRRESETBEPORT;
            pname = "BE";
            break;

        case PROCESS_FE:
            cmd = MRRESETFEPORT;
            pname = "FE";
            break;

        default:
            fprintf(stderr, "%s: Unknown processor - %d\n", __func__, proc);
            return;
    }

    req = MallocWC(sizeof(*req));
    req->port = RESET_PORT_ALL;
    req->option = RESET_PORT_INIT;

    rsp = MallocSharedWC(sizeof(*rsp));

    fprintf(stderr, "%s: Resetting %s Ports\n", __func__, pname);
    rc = PI_ExecMRP(req, sizeof(*req), cmd, rsp, sizeof(*rsp), timeout);

    Free(req);

    if (rc != PI_TIMEOUT)
    {
        Free(rsp);
    }
}


/*****************************************************************************
**
**  @brief  Go to next frame after FC Port Config
**
**  @param  none
**
**  @return none
**
*****************************************************************************
*/
static void GotoPostFCConfig(void)
{
    bool        bechange;
    bool        fechange;
    bool        setup;

    bechange = memcmp(&fc_config.be, &cntlSetup.config.be, sizeof(fc_config.be)) != 0;
    fechange = memcmp(&fc_config.fe, &cntlSetup.config.fe, sizeof(fc_config.fe)) != 0;
    setup = IsControllerSetup();

    if (bechange || fechange)
    {
        cntlSetup.config = fc_config;
    }

    if (!setup)
    {
        currentFramePtr = &AdminIPAddrFrame;
    }
    else
    {
        currentFramePtr = &FirstFrame;
        if (bechange || fechange)
        {
            SaveControllerSetup();
            if (bechange)
            {
                SendPortConfig(PROCESS_BE);
                ResetQLogic(PROCESS_BE);
            }
            if (fechange)
            {
                SendPortConfig(PROCESS_FE);
                ResetQLogic(PROCESS_FE);
            }
        }
    }
}


/*****************************************************************************
**
**  @brief  Get FC Port Config
**
**  @param  port - Port number to get config for
**
**  @return none
**
*****************************************************************************
*/
static const char *GetFCPortConfig(UINT8 port)
{
    static const char *speed_str[] = {
        [ISP_CONFIG_AUTO] = " A",
        [ISP_CONFIG_1] = " 1",
        [ISP_CONFIG_2] = " 2",
        [ISP_CONFIG_4] = " 4",
        [ISP_CONFIG_8] = " 8",
    };
    UINT8       cfg;

    if (port >= ISP_MAX_CONFIG_PORTS)
    {
        port = 0;
    }

    cfg = fc_setup->config[port / 2];
    if (cfg >= dimension_of(speed_str) || !speed_str[cfg])
    {
        cfg = ISP_CONFIG_AUTO;
    }

    return speed_str[cfg];
}


/*****************************************************************************
**
**  @brief  FC config prompt display
**
**  @param  none
**
**  @return none
**
*****************************************************************************
*/
static void FCConfigPromptDisplay(void)
{
    unsigned char LineNumber = 0;

    ProcessBadInput(&LineNumber);

    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "Configure FC ports\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "Configure:\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "(B)ack End\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "(F)ront End\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "(Q)uit\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nSelect?: ");
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/*****************************************************************************
**
**  @brief  FC config display
**
**  @param  none
**
**  @return none
**
*****************************************************************************
*/
static void FCConfigDisplay(void)
{
    unsigned char LineNumber = 0;
    unsigned char label[] = "  0  1  2  3  4  5  6  7";
    unsigned char spd[80];
    CONSOLEFRAME *cfp = currentFramePtr;
    int         i;
    int         maxports;

    ProcessBadInput(&LineNumber);

    if (fc_setup->count == 0 || fc_setup->count > ISP_MAX_CONFIG_PORTS)
    {
        fc_setup->count = 4;
    }
    maxports = fc_setup->count;
    label[maxports * 3] = 0;

    sprintf(cfp->line[LineNumber++], "\r\n");
    sprintf(cfp->line[LineNumber++], "Configure %s FC ports\r\n", fc_name);
    sprintf(cfp->line[LineNumber++], "%s\r\n", label);

    spd[0] = 0;
    for (i = 0; i < maxports; ++i)
    {
        strcat(spd, " ");
        strcat(spd, GetFCPortConfig(i));
    }
    sprintf(cfp->line[LineNumber++], "%s\r\n", spd);
    sprintf(cfp->line[LineNumber++], "\r\n");
    sprintf(cfp->line[LineNumber++], "Enter Port number, speed\r\n");
    sprintf(cfp->line[LineNumber++], "As in \"2,A\" to set port 2 to auto\r\n");
    sprintf(cfp->line[LineNumber++], "Or \"1,4\" to set port 1 to 4Gb\r\n");
    sprintf(cfp->line[LineNumber++], "Or \"0,8\" to set port 0 to 8Gb\r\n");
    sprintf(cfp->line[LineNumber++], "\r\n");
    sprintf(cfp->line[LineNumber++], "\r\nEnter?: ");
    sprintf(cfp->line[LineNumber++], "$");
}


/*****************************************************************************
**
**  @brief  FC config input
**
**  @param  none
**
**  @return none
**
*****************************************************************************
*/
static void FCConfigInput(void)
{
    char       *p = &command.line[0];
    UINT8       pn;
    UINT8       ps;

    if (*p == CRLF)
    {
        currentFramePtr = &FCConfigPrompt;
        return;
    }
    if (*p < '0' || *p > '7')
    {
        goto out1;
    }
    if (fc_setup->count == 0 || fc_setup->count > ISP_MAX_CONFIG_PORTS)
    {
        fc_setup->count = 4;    /* Set normal port count */
    }

    pn = *p - '0';              /* Port number */
    if (pn >= fc_setup->count)
    {
        goto out1;
    }

    ++p;
    if (*p != ',')
    {
        goto out1;
    }

    ++p;
    switch (toupper(*p))
    {
        case 'A':
            ps = ISP_CONFIG_AUTO;
            break;

        case '1':
            ps = ISP_CONFIG_1;
            break;

        case '2':
            ps = ISP_CONFIG_2;
            break;

        case '4':
            ps = ISP_CONFIG_4;
            break;

        case '8':
            ps = ISP_CONFIG_8;
            break;

        default:
            goto out1;
    }

    fc_setup->config[pn / 2] = ps;
    return;

  out1:
    badInput = TRUE;
}


/*--------------------------------------------------------------------------*/
void BadInput(void)
{
    badInput = TRUE;
}


/*--------------------------------------------------------------------------*/
void AlreadyConfiguredPREPResponseFunctionPre(void)
{
    currentFramePtr = &AlreadyConfiguredPREPFrame;
}


/*--------------------------------------------------------------------------*/
void AlreadyConfiguredPREPResponseFunction(void)
{
    INT32       pdisk = -1;
    PI_PDISKS_RSP *pPDisks = NULL;

    if (command.line[0] == CRLF)
    {
        currentFramePtr = &FirstFrame;
        return;
    }

    /* Get the value entered */
    sscanf(command.line, "%u", &pdisk);

    pPDisks = PhysicalDisks();

    if (!pPDisks || pdisk < 0 || pdisk >= pPDisks->count)
    {
        /* bad input, don't change the frame */
        badInput = TRUE;
    }
    else
    {
        gpdisk = pdisk;
        currentFramePtr = &PREPFrame;
    }
    if (pPDisks)
    {
        Free(pPDisks);
    }
}


/*--------------------------------------------------------------------------*/
void PREPResponseFunction(void)
{
    currentFramePtr = &ZEROTieUpConsoleFrame;
    PREPZeroDISK();
}


/*----------------------------------------------------------------------------
** Function:    IP Address Display Prompt and Response
**
** Description: Prompts user to enter new IP Address
**              [xx.xx.xx.xx] save new IP Address in a global location.
**                            Next frane = Subnet Mask Address prompt.
**
**              default [Current IP Address]
**
**--------------------------------------------------------------------------*/
void AdminIPAddrFrameDisplayFunction(void)
{
    unsigned char LineNumber = 0;
    char        Buffer[18];

    if (ethernetDriver.interfaceHandle != NULL)
    {
        CurrentAddress = GetIPAddress();
        InetToAscii(CurrentAddress, Buffer);
        sprintf(currentFramePtr->line[LineNumber++], "\r\nIP: %s", Buffer);
    }

    if (needinput == TRUE)
    {
        sprintf(currentFramePtr->line[LineNumber++],
                "\r\n\nEthernet hasn't been configured from the default.");
        sprintf(currentFramePtr->line[LineNumber++],
                "\r\nAn IP address must be entered.");
    }

    ProcessBadInputAddress(&LineNumber);

    sprintf(currentFramePtr->line[LineNumber++], "\r\nIP address, (use dot notation)?: ");
    sprintf(currentFramePtr->line[LineNumber], "$");
    needinput = FALSE;
}


/*--------------------------------------------------------------------------*/
void AdminIPAddrFrameResponseFunction(void)
{
    char        Address[16];

    if (command.line[0] == CRLF)
    {
        if (GetEthernetConfigured() == TRUE)
        {
            gIpAddress = CurrentAddress;
            currentFramePtr = &AdminSubnetMaskFrame;
            return;
        }
        else
        {
            needinput = TRUE;
        }
    }
    else
    {
        if (TestAddress((char *)command.line, command.line[85] - 2) == TRUE)
        {
            RemoveCR(command.line, Address);
            gIpAddress = inet_addr(Address);
            currentFramePtr = &AdminSubnetMaskFrame;
        }
        else
        {
            badInput = TRUE;
        }
    }
}


/*----------------------------------------------------------------------------
** Function:    Subnet Mask Address Display Prompt and Response
**
** Description: Prompts user to enter new Subnet Mask Address
**              [xx.xx.xx.xx] save new Subnet Mask Address in a global location.
**                            Next frane = Gateway Address prompt.
**
**              default [Current Subnet Mask Address]
**
**--------------------------------------------------------------------------*/
void AdminSubnetMaskFrameDisplayFunction(void)
{
    unsigned char LineNumber = 0;
    char        Buffer[18];

    if (ethernetDriver.interfaceHandle != NULL)
    {
        CurrentAddress = GetSubnetMask();
        InetToAscii(CurrentAddress, Buffer);
        sprintf(currentFramePtr->line[LineNumber++], "\r\nSubnet/mask: %s", Buffer);
    }

    if (needinput == TRUE)
    {
        sprintf(currentFramePtr->line[LineNumber++], "\r\n\nEthernet hasn't been configured from the default.");
        sprintf(currentFramePtr->line[LineNumber++], "\r\nA subnet mask must be entered.");
    }

    ProcessBadInputAddress(&LineNumber);

    sprintf(currentFramePtr->line[LineNumber++], "\r\nSubnet mask, (use dot notation)?: ");
    sprintf(currentFramePtr->line[LineNumber], "$");
    needinput = FALSE;
}


/* ------------------------------------------------------------------------ */
void AdminSubnetMaskFrameResponseFunction(void)
{
    char        Address[16];

    if (command.line[0] == CRLF)
    {
        if (GetEthernetConfigured() == TRUE)
        {
            gSubnetAddress = CurrentAddress;
            currentFramePtr = &AdminGatewayAddrFrame;
            return;
        }
        else
        {
            needinput = TRUE;
        }
    }
    else
    {
        if (TestAddress((char *)command.line, command.line[85] - 2) == TRUE)
        {
            RemoveCR(command.line, Address);
            gSubnetAddress = inet_addr(Address);
            currentFramePtr = &AdminGatewayAddrFrame;
        }
        else
        {
            badInput = TRUE;
        }
    }
}


/*----------------------------------------------------------------------------
** Function:    Gateway Address Display Prompt and Response
**
** Description: Prompts user to enter new Gateway Address
**              [xx.xx.xx.xx] save new Gateway Address in a global location.
**                            Next frane = Controller Node Cluster (CNC)ID prompt.
**
**              default [Current Gateway Address]
**
**--------------------------------------------------------------------------*/
void AdminGatewayAddrFrameDisplayFunction(void)
{
    UINT32      Address;
    unsigned char LineNumber = 0;
    unsigned char ErrCode = 0;
    char        Buffer[18];

    if (ethernetDriver.interfaceHandle != NULL)
    {
        Address = GetGatewayAddress();

        if (Address == 0)
        {
            ErrCode = 1;
            InetToAscii(GetGatewayAddress(), Buffer);
            CurrentAddress = GetGatewayAddress();
        }
        else
        {
            InetToAscii(Address, Buffer);
            CurrentAddress = Address;
        }
        sprintf(currentFramePtr->line[LineNumber++], "\r\nGateway: %s", Buffer);

        if (ErrCode && GetGatewayAddress())
        {
            sprintf(currentFramePtr->line[LineNumber++], " (unreachable)");
        }
    }

    if (needinput == TRUE)
    {
        sprintf(currentFramePtr->line[LineNumber++], "\r\n\nEthernet hasn't been configured from the default.");
        sprintf(currentFramePtr->line[LineNumber++], "\r\nA gateway must be entered.");
    }

    ProcessBadInputAddress(&LineNumber);

    sprintf(currentFramePtr->line[LineNumber++], "\r\nGateway address, (use dot notation)?: ");
    sprintf(currentFramePtr->line[LineNumber], "$");
    needinput = FALSE;
}


/* ------------------------------------------------------------------------ */

/*
** Make sure that the IP address, Subnet mask and Gateway address
** are all compatible with each other.
*/
static INT32 AddressQuickCheck(void)
{
    if (gGatewayAddress && ((gGatewayAddress & gSubnetAddress) != (gIpAddress & gSubnetAddress)))
    {
        char        msg[] = "\r\n\nWARNING: SOMETHING IS WRONG WITH YOUR IP/SUBNET MASK/"
                            "\r\nGATEWAY CONFIGURATION. PLEASE TRY ENTERING THEM AGAIN.\r\n";

        SerialBufferedWriteString(msg, strlen(msg));
        SerialBufferedWriteFlush(TRUE);

        dprintf(DPRINTF_DEFAULT, "gI = %08X, gS = %08X, gG = %08X, G&S = %08X, I&S = %08X\n",
                gIpAddress, gSubnetAddress, gGatewayAddress,
                (gGatewayAddress & gSubnetAddress),
                (gIpAddress & gSubnetAddress));

        return FAIL;
    }

    return GOOD;
}

/* ------------------------------------------------------------------------ */
void AdminGatewayAddrFrameResponseFunction(void)
{
    char        Address[16];

    if (command.line[0] == CRLF)
    {
        if (GetEthernetConfigured() == TRUE)
        {
            gGatewayAddress = CurrentAddress;

            /*
             *  If the not only changing the IP of a configured controller,
             *  then go on to enter the CNC ID. Otherwise proceed to the Are
             *  You Sure for the IP address change.
             */
            if (AddressQuickCheck() == FAIL)
            {
                /*
                 * Start over.
                 */
                currentFramePtr = &AdminIPAddrFrame;
            }
            else if (gConfiguredIPChangeFlag == false)
            {
                currentFramePtr = &EnterCncIdFrame;
            }
            else
            {
                currentFramePtr = &ApplyAreYouSureFrame;
            }
        }
        else
        {
            needinput = TRUE;
        }
    }
    else
    {
        if (TestAddress((char *)command.line, command.line[85] - 2) == TRUE)
        {
            RemoveCR(command.line, Address);
            gGatewayAddress = (inet_addr(Address));

            /*
             *  If the not only changing the IP of a configured controller,
             *  then go on to enter the CNC ID. Otherwise proceed to the Are
             *  You Sure for the IP address change.
             */
            if (AddressQuickCheck() == FAIL)
            {
                /*
                 * Start over.
                 */
                currentFramePtr = &AdminIPAddrFrame;
            }
            else if (gConfiguredIPChangeFlag == false)
            {
                currentFramePtr = &EnterCncIdFrame;
            }
            else
            {
                currentFramePtr = &ApplyAreYouSureFrame;
            }
        }
        else
        {
            badInput = TRUE;
        }
    }
}


/*----------------------------------------------------------------------------
** Function:    CNC ID Display Prompt and Response
**
** Description: Prompts user to enter CNC ID
**              [nnnnn] save new  CNC ID in global location.
**                      Next frane = Controller Node ID prompt.
**
**              default [Current CNC ID]
**
**--------------------------------------------------------------------------*/
void CncIdDisplayFunction(void)
{
    unsigned char LineNumber = 0;

    ProcessBadInput(&LineNumber);

    /*
     * Display the controller node cluster
     */
    sprintf(currentFramePtr->line[LineNumber++], "\r\nDimensional Storage Cluster ID ?: [%u] ",
            CntlSetup_GetSystemSN());
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/* ------------------------------------------------------------------------ */
void CncIdResponseFunction(void)
{
    UINT32      cncID = 0;

    if (command.line[0] == CRLF)
    {
        cncID = CntlSetup_GetSystemSN();
    }
    else
    {
        /*
         * Get the value entered
         */
        sscanf(command.line, "%u", &cncID);
    }

    if (cncID == 0)
    {
        /*
         * bad input, don't change the frame
         */
        badInput = TRUE;
    }
    else
    {
        /*
         * Update the CNC ID
         */
        gCncId = cncID;
        currentFramePtr = &EnterControllerSNFrame;
    }
}


/*----------------------------------------------------------------------------
** Function:    Controller Node ID Display Prompt and Response
**
** Description: Prompts user to enter CN ID
**              [nnnnn] save new  CN ID in global location.
**                      Next frane = Replacement Controller prompt.
**
**              default [Current CN ID]
**
**--------------------------------------------------------------------------*/
void ControllerSNDisplayFunction(void)
{
    unsigned char LineNumber = 0;

    ProcessBadInput(&LineNumber);

    /*
     * Display controller node ID
     */
    sprintf(currentFramePtr->line[LineNumber++], "\r\nDimensional Controller Node ID ?: [%u] ",
            Qm_SlotFromSerial(CntlSetup_GetControllerSN()));
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/* ------------------------------------------------------------------------ */
void ControllerSNResponseFunction(void)
{
    UINT32      cnId = 0;

    if (command.line[0] == CRLF)
    {
        /*
         * Get the current controller node ID value.
         */
        cnId = Qm_SlotFromSerial(CntlSetup_GetControllerSN());

    }
    else
    {
        sscanf(command.line, "%u", &cnId);
    }

    if (cnId > MAX_CONFIGURABLE_NODE_ID)
    {
        badInput = TRUE;
    }
    else
    {
        /*
         * Calculate the controller serial number given the slot and
         * the VCGID.
         */
        gCnId = cnId;

        currentFramePtr = &EnterCnReplaceFrame;
    }
}


/*----------------------------------------------------------------------------
** Function:    CNC Replacement Prompt and Response
**
** Description: Prompts user to determine if this is a replacement controller
**              [Y]     set global controller replacement flag.
**                      Next frane = Apply Changes prompt.
**              [N]     clear global replacement controller.
**                      Next frane = Apply Changes prompt.
**
**
**              default [N]
**
**--------------------------------------------------------------------------*/
void CnReplacePromptFunction(void)
{
    unsigned char LineNumber = 0;

    ProcessBadInput(&LineNumber);

    /*
     * Display the controller node cluster
     */
    sprintf(currentFramePtr->line[LineNumber++], "\r\nReplacement Controller Y/N ?: [N] ");
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/* ------------------------------------------------------------------------ */
void CnReplaceResponseFunction(void)
{
    if ((upper_case(command.line[0]) == 'Y'))
    {
        /* This is a replacement controller and is already licensed to this
         * group, so set the licensed flag. */
        gReplacementFlag = true;
    }
    else
    {
        /* Default is No - nothing to do */
        gReplacementFlag = false;
    }
    currentFramePtr = &ApplyAreYouSureFrame;
}


/*----------------------------------------------------------------------------
** Function:    ApplyUpdates Prompt and Response
**
** Description: Prompts user to determine if they want to apply the changes
**              that were enter.
**              [Y]     the changes are applied.
**                      Next Frame = Main User Prompt
**              [N]     no action taken.
**                      Next Frame = Main User Prompt
**
**              default [N]
**
**--------------------------------------------------------------------------*/
void ApplyPromptFunction(void)
{
    unsigned char LineNumber = 0;

    ProcessBadInput(&LineNumber);

    /*
     * Display the controller node cluster
     */
    sprintf(currentFramePtr->line[LineNumber++], "\r\nTHIS WILL CHANGE YOUR CONTROLLER'S SETUP INFORMATION ");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nApply changes Y/N ?: [N] ");
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/* ------------------------------------------------------------------------ */
void ApplyResponseFunction(void)
{
    /*
     * This is a replacement controller and is already licensed to this
     * group, so set the licensed flag.
     */
    ApplyUpdates();

    currentFramePtr = &FirstFrame;
}

/*----------------------------------------------------------------------------
** Function:    ConfigCtrl
**
** Description: Apply controller setup changes (invoked by the PI Interface)
**
** Inputs:      IP Address
**              Subnet Mask
**              Default Gateway
**              Replacement Flag
**
** Outputs:     PI_GOOD on success, Error Code on failure
**
**--------------------------------------------------------------------------*/
void        SerSetupConfigInfo(PI_VCG_CONFIGURE_REQ *pReq);
void SerSetupConfigInfo(PI_VCG_CONFIGURE_REQ *pReq)
{
    gIpAddress = pReq->IPAddr;
    gSubnetAddress = pReq->subnetMask;
    gGatewayAddress = pReq->gateway;
    gReplacementFlag = pReq->replacementFlag;
    gCncId = pReq->dscId;
    gCnId = pReq->nodeId;
    gCleanControllerFlag = false;
    return;
}

void ConfigCtrl(TASK_PARMS *parms)
{
    TASK_PARMS  parms1;
    PI_VCG_CONFIGURE_REQ *pReq = (PI_VCG_CONFIGURE_REQ *)(parms->p1);
    UINT32      controllerSN;
    char        msg1[] = { "\n****\n** CONTROLLER IS BEING RESET - PLEASE WAIT\n****\n" };

    if ((AddressQuickCheck() == GOOD) && (pReq->IPAddr != gSysIP))
    {
        /*
         * do the configuration
         */
        gSysIP = pReq->IPAddr;
        dprintf(DPRINTF_DEFAULT, "PI_ConfigCtrl: Pausing for 5 seconds...\n");
        TaskSleepMS(5000);

        /*
         * Set up network information
         */
        SetIPAddress(gIpAddress);
        SetSubnetMask(gSubnetAddress);
        SetGatewayAddress(gGatewayAddress);
        SetConfigEthernet();

        /*
         * Set the CNC ID
         */
        CntlSetup_SetSystemSN(gCncId);

        /*
         * Update the controller SN
         * Calculate the controller serial number given the slot and
         * the VCGID.
         */
        controllerSN = Qm_SerialFromVCGID(CntlSetup_GetSystemSN(), gCnId);

        CntlSetup_SetControllerSN(controllerSN);
        UpdateProcSerialNumber(CONTROLLER_SN, controllerSN);

        /*
         * Update the CNC name if it is not set.
         */
        if (!ControllerNodeCluster_IsNameSet())
        {
            /*
             * Update the controller node cluster name with the
             * default value of CNCnnnnnnnnnn.
             */
            LogMessage(LOG_TYPE_DEBUG, "SERCON-Set CNC Name");
            ControllerNodeCluster_SetDefaultName();
        }

        /*
         * If this is a relpacement controller, then the CNC must be licensed,
         * so set the Licensed flag.
         */
        if (gReplacementFlag == true)
        {
            LogMessage(LOG_TYPE_DEBUG, "SERCON-Replacement Controller option");
            SetReplacementController();
            SetLicenseApplied();

            /*
             * Delete the "clean shutdown" flag so that when the controller
             * comes back up, it assumes a dirty (possible outstanding Raid5
             * resync ops) shutdown.
             */
            GetCleanShutdown(); /* A "Get" clears the flag */
        }
        else
        {
            ClearReplacementController();
        }

        /*
         * If the controller was cleaned, reset the controller and start
         * over. TO DO: Do we need to do this????
         */
        if (gCleanControllerFlag == true)
        {
            /* Send the string to the serial console to be displayed. */

            SerialBufferedWriteString(msg1, strlen(msg1));

            /* Flush the string to the console. */

            SerialBufferedWriteFlush(TRUE);

            /* Reset the system. */

            LogMessage(LOG_TYPE_DEBUG, "SERCON-Resetting all processors");
            ProcessReset(PROCESS_ALL);
        }

        /*
         * Now Start the PI Server
         * Start the EWOK Port Server (3000)
         */
        parms1.p1 = EWOK_PORT_NUMBER;
        TaskCreate(DebugServer, &parms1);

        /* Start the TEST Port Server (3100) */
        parms1.p1 = TEST_PORT_NUMBER;
        TaskCreate(DebugServer, &parms1);

        /* Start the DEBUG Port Server (3200) */
        parms1.p1 = DEBUG_PORT_NUMBER;
        TaskCreate(DebugServer, &parms1);
    }
    else if (pReq->IPAddr == gSysIP)
    {
        dprintf(DPRINTF_DEFAULT, "PI_ConfigCtrl: System has the given IP\n");
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "PI_ConfigCtrl: Invalid IP/Sm/GW\n");
    }
    return;
}

/*----------------------------------------------------------------------------
** Function:    ApplyUpdates
**
** Description: Apply controller setup changes entered via serial port
**
** Inputs:      bool    - TRUE = replacement controller
**                      - FALSE = not a replacement controller
**
**--------------------------------------------------------------------------*/

void ApplyUpdates(void)
{
    UINT8       restartPIServer = FALSE;
    UINT32      controllerSN;
    TASK_PARMS  parms1;
    char        msg1[] = { "\n****\n** CONTROLLER IS BEING RESET - PLEASE WAIT\n****\n" };
    char        msg2[] = { "\n****\n** UPDATES APPLIED, POWER-OFF CONTROLLER\n****\n" };
    char        msg3[] = { "\n****\n** PLEASE WAIT\n****\n" };
    char        msgErr[] = { "\n****\n** UPDATES FAILED, CHECK DRIVE CONNECTIONS\n"
                             "** POWER-OFF CONTROLLER TO TRY AGAIN\n****\n"
    };

    LogMessage(LOG_TYPE_INFO, "SERCON-Serial console configuration");
    if (gSysIP != 0 && gSysIP != gIpAddress)
    {
        gSysIP = gIpAddress;
        restartPIServer = TRUE;
        SerialBufferedWriteString(msg3, strlen(msg3));
        dprintf(DPRINTF_DEFAULT, "Apply Updates: Pausing for 3 seconds...\n");
        TaskSleepMS(3000);
    }

    if (gConfiguredIPChangeFlag == false)
    {
        LogMessage(LOG_TYPE_DEBUG, "SERCON-Not a configured IP change");
        /*
         * If the system was previously configured and the user has requested to
         * continue with the set-up, the clean previous configuration information
         * from the controller before continuing.
         */
        if (gCleanControllerFlag == true)
        {
            LogMessage(LOG_TYPE_DEBUG, "SERCON-Cleaned Controller Setup");
            SerSetupCtrlClean();
        }

        /*
         * Set up network information
         */
        SetIPAddress(gIpAddress);
        SetSubnetMask(gSubnetAddress);
        SetGatewayAddress(gGatewayAddress);
        SetConfigEthernet();

        /*
         * Set the CNC ID
         */
        CntlSetup_SetSystemSN(gCncId);

        /*
         * Update the controller SN
         * Calculate the controller serial number given the slot and
         * the VCGID.
         */
        controllerSN = Qm_SerialFromVCGID(CntlSetup_GetSystemSN(), gCnId);

        CntlSetup_SetControllerSN(controllerSN);
        UpdateProcSerialNumber(CONTROLLER_SN, controllerSN);

        /*
         * Update the CNC name if it is not set.
         */
        if (!ControllerNodeCluster_IsNameSet())
        {
            /*
             * Update the controller node cluster name with the
             * default value of CNCnnnnnnnnnn.
             */
            LogMessage(LOG_TYPE_DEBUG, "SERCON-Set CNC Name");
            ControllerNodeCluster_SetDefaultName();
        }

        /*
         * If this is a relpacement controller, then the CNC must be licensed,
         * so set the Licensed flag.
         */
        if (gReplacementFlag == true)
        {
            LogMessage(LOG_TYPE_DEBUG, "SERCON-Replacement Controller option");
            SetReplacementController();
            SetLicenseApplied();

            /*
             * Delete the "clean shutdown" flag so that when the controller
             * comes back up, it assumes a dirty (possible outstanding Raid5
             * resync ops) shutdown.
             */
            GetCleanShutdown(); /* A "Get" clears the flag */
        }
        else
        {
            ClearReplacementController();
        }

        /*
         * If the controller was cleaned, reset the controller and start
         * over.
         */
        if (gCleanControllerFlag == true)
        {
            /* Send the string to the serial console to be displayed. */

            SerialBufferedWriteString(msg1, strlen(msg1));

            /* Flush the string to the console. */

            SerialBufferedWriteFlush(TRUE);

            /* Reset the system. */

            LogMessage(LOG_TYPE_DEBUG, "SERCON-Resetting all processors");
            ProcessReset(PROCESS_ALL);
        }
        else if (restartPIServer == TRUE)
        {
            /*
             * Now Start the PI Server
             * Start the EWOK Port Server (3000)
             */
            parms1.p1 = EWOK_PORT_NUMBER;
            TaskCreate(DebugServer, &parms1);

            /* Start the TEST Port Server (3100) */
            parms1.p1 = TEST_PORT_NUMBER;
            TaskCreate(DebugServer, &parms1);

            /* Start the DEBUG Port Server (3200) */
            parms1.p1 = DEBUG_PORT_NUMBER;
            TaskCreate(DebugServer, &parms1);
        }
    }
    else
    {
        LogMessage(LOG_TYPE_DEBUG, "SERCON-Configured IP change");
        /*
         * Ensure we are master to allow the controller map to be written.
         * This is ok since only one controller should be powered on at this
         * time and we are going to reset after this operation.
         */
        Qm_SetMasterControllerSN(GetMyControllerSN());

        /*
         * Since this is a configured controller, write the IP address
         * information to the controller map in the quorum area.
         *
         * The true signals that the "new" and "current" network settings
         * should be updated.
         */
        if (SetControllerMapAddresses(GetMyControllerSN(), gIpAddress,
                                      gSubnetAddress, gGatewayAddress, true) == GOOD)
        {

            LogMessage(LOG_TYPE_DEBUG, "SERCON-Controller Map update OK");

            /* Set up network information */
            SetIPAddress(gIpAddress);
            SetSubnetMask(gSubnetAddress);
            SetGatewayAddress(gGatewayAddress);
            SetConfigEthernet();

            /* Send the string to the serial console to be displayed. */

            SerialBufferedWriteString(msg2, strlen(msg2));
        }
        else
        {
            LogMessage(LOG_TYPE_DEBUG, "SERCON-Controller Map update FAILED");
            SerialBufferedWriteString(msgErr, strlen(msgErr));
        }

        /* Flush the string to the console. */
        SerialBufferedWriteFlush(TRUE);

        /* Wait in a tight loop for the user to power-off the controller. */

        LogMessage(LOG_TYPE_DEBUG, "SERCON-Controller shut down after IP change");
        errExit(ERR_EXIT_SHUTDOWN);
    }
}


/*----------------------------------------------------------------------------
** Function:    PREPZeroDISK
**
** Description: Zero the first 128k of a CUSTOMERS disk.
**
** Returns:     NONE
**--------------------------------------------------------------------------*/

void PREPZeroDISK(void)
{
    char        msg1[CONSOLE_COLUMNS];
    UINT8       rc;
    PI_PDISKS_RSP *pPDisks;

    Lock_Shutdown_FE();

    snprintf(msg1, CONSOLE_COLUMNS, "SERCON-Serial console PREPING DISK %u", gpdisk);
    LogMessage(LOG_TYPE_INFO, "%s", msg1);

    snprintf(msg1, CONSOLE_COLUMNS, "\n****\n** DISK %u IS BEING ZEROED - PLEASE WAIT\n****\n",
             gpdisk);
    /* Send the string to the serial console to be displayed. */
    SerialBufferedWriteString(msg1, strlen(msg1));
    /* Flush the string to the console. */
    SerialBufferedWriteFlush(TRUE);

    pPDisks = PhysicalDisks();
    if (pPDisks)
    {
        rc = MfgCtrlClean_WriteSame(pPDisks->pdiskInfo[gpdisk].pdd.wwn,
                                    pPDisks->pdiskInfo[gpdisk].pdd.lun);
        if (rc != PI_GOOD)
        {
            snprintf(msg1, CONSOLE_COLUMNS, "Failed to clear physical disk %u (%8.8x%8.8x)\n",
                     gpdisk,
                     bswap_32((UINT32)pPDisks->pdiskInfo[gpdisk].pdd.wwn),
                     bswap_32((UINT32)(pPDisks->pdiskInfo[gpdisk].pdd.wwn >> 32)));
        }
        else
        {
            snprintf(msg1, CONSOLE_COLUMNS, "Cleared physical disk %u (%8.8x%8.8x)\n",
                     gpdisk,
                     bswap_32((UINT32)pPDisks->pdiskInfo[gpdisk].pdd.wwn),
                     bswap_32((UINT32)(pPDisks->pdiskInfo[gpdisk].pdd.wwn >> 32)));
        }
        Free(pPDisks);
    }
    else
    {
        snprintf(msg1, CONSOLE_COLUMNS, "No physical disks currently available, failed to clear physical disk %u\n",
                 gpdisk);
    }
    LogMessage(LOG_TYPE_INFO, "%s", msg1);
    SerialBufferedWriteString(msg1, strlen(msg1));
    SerialBufferedWriteFlush(TRUE);

    /* 0 - Do not need to lock and shutdown FE, done above. */
    Clean_Controller(0, MFG_CTRL_CLEAN_OPT_SERIAL_MESSAGES |
                     MFG_CTRL_CLEAN_OPT_NO_LOG_CLEAR | MFG_CTRL_CLEAN_OPT_POWER_DOWN);
    /* Does not return. */
}


/*----------------------------------------------------------------------------
** Function:    NetstatDisplayFunction
**
** Description: Display network connections
**                      Next Frame = Main User Prompt
**
** Outputs:     To serial port
**--------------------------------------------------------------------------*/
void NetstatDisplayFunction(void)
{
    /*
     * Initialize a pointer to store buffer malloc'd by callee in
     */
    char       *str = NULL;
    UINT32      len = DisplaySocketStats(&str, 0);

    if (len && str)
    {
        /* Send the string to the serial console to be displayed. */
        SerialBufferedWriteString(str, len);

        /* Flush the string to the console. */
        SerialBufferedWriteFlush(TRUE);
    }

    /* Free the buffer passed to us */
    if (str)
    {
        Free(str);
    }

    currentFramePtr = &FirstFrame;
}

/*----------------------------------------------------------------------------
** Function:    ToggleDiagPorts...Function(void)
**
** Description: Manage the Toggling of the diag ports
**
** Inputs:      None
**
**--------------------------------------------------------------------------*/
void ToggleDiagPortsChoiceFunction(void)
{
    currentFramePtr = &ToggleDiagPortFrame;
}

/* ------------------------------------------------------------------------ */
static void ToggleDiagPortsDisplayFunction(void)
{
    SerialBufferedWriteString("\r\n", 2);
    SerialBufferedWriteFlush(TRUE);
}

/* ------------------------------------------------------------------------ */
static void ToggleDiagPortsResponseFunction(void)
{
    char        buf[256];

    if (TestModeBit(MD_DIAG_PORTS_ENABLE) == 0)
    {
        /*
         * Toggling ON requires a password
         */
        if (sscanf(command.line, "%s", buf) != 1)
        {
            currentFramePtr = &FirstFrame;
        }
        else
        {
            if (memcmp(buf, "xiotech2003", 11) == 0)
            {
                SetModeBit(MD_DIAG_PORTS_ENABLE);
                SerialBufferedWriteString("ok          \r\n", 14);

                /*
                 * K_timel will not roll for 17 years so I am not going to
                 * worry about that.  I don't think we have the reliability of
                 * an AS400, so I think this is a safe assumption :-).
                 */
                diagPortTimeout = K_timel + DIAG_PORT_ENABLE_TIMEOUT;
            }
            else
            {
                /* Blank out any bad attempts */
                SerialBufferedWriteString("            \r\n", 14);
            }
        }
    }
    else
    {
        /* Toggling OFF doesn't require a password */
        ClrModeBit(MD_DIAG_PORTS_ENABLE);
        SerialBufferedWriteString("off         \r\n", 14);
        diagPortTimeout = 0;
    }

    /* Flush the string to the console. */
    SerialBufferedWriteFlush(TRUE);

    /* Return to the main frame */
    currentFramePtr = &FirstFrame;
}

/* ------------------------------------------------------------------------ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
