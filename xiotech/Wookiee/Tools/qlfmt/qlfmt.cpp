/* $Id: qlfmt.cpp 93725 2009-07-27 16:11:33Z mdr $ */
/**
******************************************************************************
**
**  @file       qlfmt.cpp
**
**  @version    $Revision: 93725 $
**
**  @brief      QLogic dump format utility.
**
**  @author     Chris Nigbur
**
**  modified    $Author: mdr $
**
**  @date       $Date: 2009-07-27 16:11:33 +0000 (Mon, 27 Jul 2009) $
**
**  Utility to build correctly formatted QLogic dump output from the memory dump.
**
**  Copyright (c) 2007-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#include <cstdio>
#include <string>
#include <fstream>
#include <iostream>
#include <ostream>
#include <iomanip>
#include <stdint.h>

using namespace std;

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/


/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define QLOGIC_ID   0x1077
#define QL2312      0x2312
#define QL2422      0x2422
#define QL2432      0x2432


/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/
typedef uint32_t UINT32;
typedef uint16_t UINT16;
typedef uint64_t UINT64;

typedef struct
{
    UINT32 vendID;                  /* Vendor ID                            */
    UINT32 model;                   /* Vendor model                         */
    UINT16 revLvl;                  /* Revision Level of ISP                */
    UINT16 rscLvl;                  /* RISC revision Level                  */
    UINT16 fpmLvl;                  /* FB & FPM revision levels             */
    UINT16 romLvl;                  /* RISC ROM revision level              */
    UINT64 type;                    /* Firmware type (ef/efm)               */
    UINT16 fwMajor;                 /* ISP firmware major revision          */
    UINT16 fwMinor;                 /* ISP firmware minor revision          */
    UINT16 fwSub;                   /* ISP firmware subminor revision       */
    UINT16 fwAttrib;                /* ISP firmware attribute               */
    UINT16 dataRate;                /* Data Rate (1G/2G)                    */
    UINT32 endMemAddr;              /* Ending memory Addr(Used in Debug     */
} __attribute__((__packed__)) ISP_REV;


/*
******************************************************************************
** Private variables
******************************************************************************
*/

ISP_REV isprev;     /* Holds revision header from file */


/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/


/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
bool sanitycheck_16(ifstream& in, unsigned int addr, unsigned short sanity_val);
bool sanitycheck_32(ifstream& in, unsigned int addr, unsigned int sanity_val);
void process_16(ifstream& in, ostream& out, unsigned short count);
void process_16(ifstream& in, ostream& out, unsigned short count, unsigned int addr);
void process_32(ifstream& in, ostream& out, unsigned short count);
void process_32(ifstream& in, ostream& out, unsigned short count, unsigned int addr);
void process2300(ifstream& in, ostream& out);
void process2400(ifstream& in, ostream& out);


/*
******************************************************************************
** Code Start
******************************************************************************
*/
/**
******************************************************************************
**
**  @brief      Validate the value at an offset in the file vs. an expected
**              value.
**
**  @param      in - Input stream from which to read the value.
**
**  @param      addr - Offset/address within the file to read the value.
**
**  @param      sanity_val - Expected value for the data in the file.
**
**  @return     true - if the value read from the file matches the sanity value.
**  @return     false - if the value read from the file does not match the
**                      sanity value.
**
******************************************************************************
**/
bool sanitycheck_16(ifstream& in, unsigned int addr, unsigned short sanity_val)
{
    unsigned short  val;
    streampos       pos = in.tellg();

    in.seekg(addr);
    in.read((char*)&val, 2);
    in.seekg(pos);

    return (val == sanity_val);
}


/**
******************************************************************************
**
**  @brief      Validate the value at an offset in the file vs. an expected
**              value.
**
**  @param      in - Input stream from which to read the value.
**
**  @param      addr - Offset/address within the file to read the value.
**
**  @param      sanity_val - Expected value for the data in the file.
**
**  @return     true - if the value read from the file matches the sanity value.
**  @return     false - if the value read from the file does not match the
**                      sanity value.
**
******************************************************************************
**/
bool sanitycheck_32(ifstream& in, unsigned int addr, unsigned int sanity_val)
{
    unsigned int    val;
    streampos       pos = in.tellg();

    in.seekg(addr);
    in.read((char*)&val, 4);
    in.seekg(pos);

    return (val == sanity_val);
}


/**
******************************************************************************
**
**  @brief      Processes a set of bytes from the input stream, formats them
**              and submits them to the output stream.
**
**              This function processes a number of 16 bit chunks of data from
**              the input stream and converts the data to a HEX string sent to
**              the output stream.
**
**  @return     none
**
******************************************************************************
**/
void process_16(ifstream& in, ostream& out, unsigned short count)
{
    unsigned short  i;
    unsigned short  val;

    out << hex;

    for (i = 0; i < count; ++i)
    {
        if (i > 0)
        {
            out << ' ';
        }

        in.read((char *)&val, 2);

        out.fill('0');
        out.width(4);
        out << val;
    }

    out << dec;
    out << endl;
}


/**
******************************************************************************
**
**  @brief      Sends the address label to the output stream and then continues
**              to the processsing done via process_16.
**
**  @return     none
**
******************************************************************************
**/
void process_16(ifstream& in, ostream& out, unsigned short count, unsigned int addr)
{
    out.fill('0');
    out.width(4);
    out << hex << addr << dec << ": ";

    process_16(in, out, count);
}


/**
******************************************************************************
**
**  @brief      Processes a set of bytes from the input stream, formats them
**              and submits them to the output stream.
**
**              This function processes a number of 32 bit chunks of data from
**              the input stream and converts the data to a HEX string sent to
**              the output stream.
**
**  @return     none
**
******************************************************************************
**/
void process_32(ifstream& in, ostream& out, unsigned short count)
{
    unsigned short  i;
    unsigned int    val;

    out << hex;

    for (i = 0; i < count; ++i)
    {
        if (i > 0)
        {
            out << ' ';
        }

        in.read((char *)&val, 4);

        out.fill('0');
        out.width(8);
        out << val;
    }

    out << dec;
    out << endl;
}


/**
******************************************************************************
**
**  @brief      Sends the address label to the output stream and then continues
**              to the processsing done via process_32.
**
**  @return     none
**
******************************************************************************
**/
void process_32(ifstream& in, ostream& out, unsigned short count, unsigned int addr)
{
    out.fill('0');
    out.width(8);
    out << hex << addr << dec << ": ";

    process_32(in, out, count);
}


/**
******************************************************************************
**
**  @brief      Process an input stream containing the dump data from a
**              QLogic 23xx card.
**
**  @return     none
**
******************************************************************************
**/
void process2300(ifstream& in, ostream& out)
{
    unsigned int    j;

    cerr << "Processing dump from QLogic 23xx.\n";

    out << "PBIU registers\n";
    process_16(in, out, 8);

    out << "\nReqQ-RspQ-Risc2Host Status registers\n";
    process_16(in, out, 8);

    out << "\nMailbox registers\n";
    for (j = 0; j < 4; ++j)
    {
        process_16(in, out, 8);
    }

    out << "\nAuto Request Responce DMA registers\n";
    for (j = 0; j < 4; ++j)
    {
        process_16(in, out, 8);
    }

    out << "\nDMA registers\n";
    for (j = 0; j < 6; ++j)
    {
        process_16(in, out, 8);
    }

    out << "\nRISC hardware registers\n";
    process_16(in, out, 8);
    process_16(in, out, 8);

    out << "\nRISC GP0 registers\n";
    process_16(in, out, 8);
    process_16(in, out, 8);

    out << "\nRISC GP1 registers\n";
    process_16(in, out, 8);
    process_16(in, out, 8);

    out << "\nRISC GP2 registers\n";
    process_16(in, out, 8);
    process_16(in, out, 8);

    out << "\nRISC GP3 registers\n";
    process_16(in, out, 8);
    process_16(in, out, 8);

    out << "\nRISC GP4 registers\n";
    process_16(in, out, 8);
    process_16(in, out, 8);

    out << "\nRISC GP5 registers\n";
    process_16(in, out, 8);
    process_16(in, out, 8);

    out << "\nRISC GP6 registers\n";
    process_16(in, out, 8);
    process_16(in, out, 8);

    out << "\nRISC GP7 registers\n";
    process_16(in, out, 8);
    process_16(in, out, 8);

    out << "\nframe buffer hardware registers\n";
    for (j = 0; j < 8; ++j)
    {
        process_16(in, out, 8);
    }

    out << "\nFPM B0 hardware registers\n";
    for (j = 0; j < 8; ++j)
    {
        process_16(in, out, 8);
    }

    out << "\nFPM B1 hardware registers\n";
    for (j = 0; j < 8; ++j)
    {
        process_16(in, out, 8);
    }

    if (!sanitycheck_16(in, 0x03A0, 0x0470))
    {
        cerr << "ERROR: Invalid code start != 0x0470\n";
    }

    out << "\nCode RAM Dump\n";

    process_16(in, out, 16, 0x800);

    for (j = 0x810; j < 0xFFFF; j += 16)
    {
        process_16(in, out, 16, j);

        if (in.eof())
        {
            break;
        }
    }

    out << "\nStack RAM Dump\n";

    for (j = 0x10000; j < 0x10FFF; j += 16)
    {
        process_16(in, out, 16, j);

        if (in.eof())
        {
            break;
        }
    }

    if (!sanitycheck_16(in, 0x3F220, 0xFF80))
    {
        cerr << "ERROR: Address 0x3F220 does not contain 0x80FF.\n";
    }

    out << "\nData RAM Dump\n";

    for (j = 0x11000; j < 0x01FFFF; j += 16)
    {
        process_16(in, out, 16, j);

        if (in.eof())
        {
            break;
        }
    }

    out << "\n[<==END]ISP Debug Dump\n";
}


/**
******************************************************************************
**
**  @brief      Process an input stream containing the dump data from a
**              QLogic 24xx card.
**
**  @return     none
**
******************************************************************************
**/
void process2400(ifstream& in, ostream& out)
{
    unsigned int    j;

    cerr << "Processing dump from QLogic 24xx.\n";

    out.fill('0');
    out << "ISP FW Version " << dec
        << isprev.fwMajor << '.'
        << setw(2) << isprev.fwMinor << '.' << isprev.fwSub
        << " Attributes " << hex << setw(4) << isprev.fwAttrib << endl;

    out << "\nR2H Status Register\n";
    process_32(in, out, 1);

    out << "\nHost Interface Registers\n";
    process_32(in, out, 8);
    process_32(in, out, 8);
    process_32(in, out, 8);
    process_32(in, out, 8);

    out << "\nShadow Registers\n";
    process_32(in, out, 7);

    out << "\nMailbox Registers\n";
    process_16(in, out, 8);
    process_16(in, out, 8);
    process_16(in, out, 8);
    process_16(in, out, 8);

    out << "\nXSEQ GP Registers\n";
    /* Transfer Sequence Registers */
    for (j = 0; j < 16; ++j)
    {
        process_32(in, out, 8);
    }

    for (j = 0; j < 2; ++j)
    {
        out << "\nXSEQ-" << j << " Registers\n";
        process_32(in, out, 8);
        process_32(in, out, 8);
    }

    out << "\nRSEQ GP Registers\n";
    /* Receive Sequence Registers */
    for (j = 0; j < 16; ++j)
    {
        process_32(in, out, 8);
    }

    for (j = 0; j < 3; ++j)
    {
        out << "\nRSEQ-" << j << " Registers\n";
        process_32(in, out, 8);
        process_32(in, out, 8);
    }

    out << "\nCommand DMA Registers\n";
    /* Command DMA Registers */
    process_32(in, out, 8);
    process_32(in, out, 8);

    out << "\nRequest0 Queue DMA Channel Registers\n";
    process_32(in, out, 8);
    process_32(in, out, 7);

    out << "\nResponse0 Queue DMA Channel Registers\n";
    process_32(in, out, 8);
    process_32(in, out, 7);

    out << "\nRequest1 Queue DMA Channel Registers\n";
    process_32(in, out, 8);
    process_32(in, out, 7);

    /* Transmit DMA Registers */
    for (j = 0; j < 5; ++j)
    {
        out << "\nXMT" << j << " Data DMA Registers\n";
        process_32(in, out, 8);
        process_32(in, out, 8);
        process_32(in, out, 8);
        process_32(in, out, 8);
    }

    out << "\nXMT Data DMA Common Registers\n";
    process_32(in, out, 8);
    process_32(in, out, 8);

    /* Receive DMA Registers */
    for (j = 0; j < 2; ++j)
    {
        out << "\nRCV Thread " << j << " Data DMA Registers\n";
        process_32(in, out, 8);
        process_32(in, out, 8);
        process_32(in, out, 8);
        process_32(in, out, 8);
    }

    /* RISC Registers */
    out << "\nRISC GP Registers\n";
    for (j = 0; j < 16; ++j)
    {
        process_32(in, out, 8);
    }

    /* Local Memory Controller Registers */
    out << "\nLMC Registers\n";
    for (j = 0; j < 14; ++j)
    {
        process_32(in, out, 8);
    }

    /* Fibre Protocol Module Registers */
    out << "\nFPM Hardware Registers\n";
    for (j = 0; j < 24; ++j)
    {
        process_32(in, out, 8);
    }

    /* Frame Buffer Registers */
    out << "\nFB Hardware Registers\n";
    for (j = 0; j < 22; ++j)
    {
        process_32(in, out, 8);
    }

    out << "\nCode RAM\n";
    for (j = 0; j < 0x2000; j += 8)
    {
        process_32(in, out, 8, 0x20000 + j);

        if (in.eof())
        {
            break;
        }
    }

    out << "\nExternal Memory\n";
    for (j = 0; j < 0x40000; j += 8)
    {
        process_32(in, out, 8, 0x100000 + j);

        if (in.eof())
        {
            break;
        }
    }

    out << "\n[<==END] ISP Debug Dump\n";
}


/**
******************************************************************************
**
**  @brief      Get revision information.
**
**  @return     none
**
******************************************************************************
**/
static bool getrev(ifstream &in)
{
    in.read((char *)(&isprev), sizeof(isprev));
    return in.bad() || in.gcount() != sizeof(isprev);
}


/**
******************************************************************************
**
**  @brief      Set revision information.
**
**  @return     none
**
******************************************************************************
**/
static void setisprev(ifstream &in, uint32_t model)
{
    memset(&isprev, 0, sizeof(isprev));
    isprev.vendID = QLOGIC_ID;
    isprev.model = model;
    in.seekg(0);            /* Back to the beginning */
}


/**
******************************************************************************
**
**  @brief  Print usage.
**
**  @return none
**
******************************************************************************
**/
static void usage(char *name)
{
    cerr
        << "Usage: " << name << " in req res atio asyqa\n"
        << "   in          qldmp file name\n"
        << "   req         request queue dump file\n"
        << "   res         response queue dump file\n"
        << "   atio        atio queue dump file\n"
        << "   asyqa       asynchronous event queue dump file\n"
        ;
}


/**
******************************************************************************
**
**  @brief  Dump queues
**
**  @param  argc - argument count
**  @param  argv - argument vector
**  @param  first - first argument to process
**
**  @return none
**
******************************************************************************
**/
static void dump_queues(int argc, char *argv[], int first)
{
    int i;
    static const char    *names[] =
        {
            "Request Queue",
            "Response Queue",
            "Async Event Queue",
            "ATIO Queue",
            0
        };

    for (i = first; names[i - first]; ++i)
    {
        if (i >= argc || argv[i][0] == 0)
        {
            cerr << "No " << names[i - first] << " dump file provided\n";
            continue;
        }

        ifstream    fin(argv[i], ios::in | ios::binary);
        if (fin.bad())
        {
            cerr << "Failed to open " << names[i - first]
                << " dump file " << argv[i] << endl;
            fin.close();
            continue;
        }
        cerr << "Processing " << names[i - first] << " dump file " << argv[i] << endl;
        cout << names[i - first] << endl;

        int j;
        for (j = 0; !fin.eof(); j += 8)
        {
            process_16(fin, cout, 8, j);
        }

        cout << "[<==END]" << names[i - first] << endl << endl;

        fin.close();
    }
}


/**
******************************************************************************
**
**  @brief  Application entry point.
**
**  @return none
**
******************************************************************************
**/
int main(int argc, char *argv[])
{
    ifstream    in;

//     cerr
//         << "===============================================================\n"
//         << " QLogic Dump Format Utility\n"
//         << " Copyright (c) 2007 Xiotech Corporation.\n"
//         <<< endl
//         < " All rights reserved.\n"
//         << endl
//         << " For Internal Use Only\n"
//         << "===============================================================\n"
//         << endl;

    if (argc < 2 || argc > 6)
    {
        usage(argv[0]);
        return 1;
    }

    in.open(argv[1], ios::in | ios::binary);
    if (in.bad())
    {
        cerr << "Unable to open input file " << argv[1] << endl;
        usage(argv[0]);
        return 2;
    }

    //cerr << "Formatting dump file: " << argv[1] << endl;

    if (getrev(in))
    {
    //    cerr << "Unable to get revision information - \n";
        setisprev(in, QL2312);
        return 3;
    }

    cerr << hex;
    if (isprev.vendID != QLOGIC_ID)
    {
        cerr
            << "Unknown vendor ID: "
            << setw(4) << "setfildanalysis\n";
        return 4;
    }

    switch (isprev.model)
    {
    case QL2422:
    case QL2432:
        process2400(in, cout);
        break;

    case QL2312: 
        process2300(in, cout);
        break;

    default:
        cerr
            << "Unknown model ID: "
            << setw(4) << setfill('0') << isprev.model << endl;
        return 5;
    }

    in.close();
    cout << endl;

    dump_queues(argc, argv, 2);

    return 0;
}


/***
** Modelines
** vi:sw=4 ts=4 expandtab
***/
