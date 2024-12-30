/* $Id: loop.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       loop.h
**
**  @brief      Loop WWN definitions
**
**  To provide definitions for generic FC-AL equates, structures and
**  bit definitions.
**
**  Copyright (c) 2002-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/

#ifndef _LOOP_H_
#define _LOOP_H_

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
/*
** OUIs (IEEE address)
*/
#define XIO_OUI             0x00D0B2

#define ARIO_OUI            0x00054A

#define CURTIS_OUI          0x0050C2

#define SL_OUI              0x000a33    /**< OUI for the Sierra Logic       */
                                        /**< Chipset in the Xyratex Elwood  */

#define SHIFT_IEEE_48BIT(x) (bswap_64((UINT64)(x) << 24))
#define MASK_IEEE_48BIT(x)  ((x) & 0x0000000FFFFFF0000LL)

#define SHIFT_IEEE_REG(x)   (bswap_32((x) << 4))
#define MASK_IEEE_REG(x)    ((x) & 0x000000000F0FFFF0FLL)

/*
** World wide names constants
**
** The world wide names follow this format...
**
**     Node names:  21e0oooo:oossssss  where e = 0 for front end
**                                               1 for back end
**                                          oooooo = OUI
**                                          ssssss = controller serial number
**
**     Front end Port names:  22c0oooo:oossssss  where c = channel number
**
**     Back end Port names:  23c0oooo:oossssss  where c = channel number
*/
#define WWN_F_NODE  0x202000D0      /* MSW of front end node name           */
#define WWN_C_NODE  0x206000D0      /* MSW of front end control port name   */
#define WWN_B_NODE  0x20A000D0      /* MSW of back end node name            */
#define WWN_E_NODE  0x20E000D0      /* MSW of disk enclosure node name      */
#define WWN_F_PORT  0x212000D0      /* MSW of front end port name           */
#define WWN_C_PORT  0x216000D0      /* MSW of front end control port name   */
#define WWN_B_PORT  0x21A000D0      /* MSW of back end port name            */

#endif /* _LOOP_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
