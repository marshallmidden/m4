/*
 * Intel 5400(A/B) class Memory Controllers kernel module
 *
 * This file may be distributed under the terms of the
 * GNU General Public License.
 *
 * Based on the Intel 5000 EDAC driver written by:
 * Douglas Thompson Linux Networx (http://lnxi.com)
 *	norsk5@xmission.com
 *
 * Modified for the I5400 by:
 * Mark Rustad, Xiotech Corporation (http://www.xiotech.com/)
 *	mark_rustad@xiotech.com or MRustad@gmail.com
 *
 * This module is based on the following document:
 *
 * Intel 5400 Chipset Memory Controller Hub (MCH) - Datasheet
 * 	http://developer.intel.com/design/chipsets/datashts/318610.htm
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/slab.h>
#include <linux/edac.h>
#include <asm/mmzone.h>

#include "edac_core.h"

/*
 * Define the following to turn on stop and scream mode.
 */
#define	I5400_STOP_AND_SCREAM 1

/*
 * Alter this version for the I5400 module when modifications are made
 */
#define I5400_REVISION    " Ver: 1.0.0 " __DATE__
#define EDAC_MOD_STR      "i5400_edac"

#define i5400_printk(level, fmt, arg...) \
        edac_printk(level, "i5400", fmt, ##arg)

#define i5400_mc_printk(mci, level, fmt, arg...) \
        edac_mc_chipset_printk(mci, level, "i5400", fmt, ##arg)

#define	PCI_DEVICE_ID_INTEL_I5400A_DEV0	0x4001
#define	PCI_DEVICE_ID_INTEL_I5400B_DEV0	0x4003

/* Device 0,
 * Function 0: PCI Express PCI space
 */

#define		SSCTRL			0x47	/* Also in devices 1 - 9 */

#define	PCI_DEVICE_ID_INTEL_I5400_DEV1	0x4021

/* Device 16,
 * Function 0: System Address
 * Function 1: Memory Branch Map, Control, Errors Register
 * Function 2: FSB Error Registers
 *
 * All 3 functions of Device 16 (0,1,2) share the SAME DID
 */
#define	PCI_DEVICE_ID_INTEL_I5400_DEV16	0x4030

/* Offsets for Function 0 */
#define		AMBASE			0x48
#define		MAXCH			0x56
#define		MAXDIMMPERCH		0x57

/* Offsets for Function 1 */
#define		TOLM			0x6C
#define		MIR0			0x80
#define		MIR1			0x84
#define		AMIR0			0x8C
#define		AMIR1			0x90

#define		FERR_FAT_FBD		0x98
#define		NERR_FAT_FBD		0x9C
#define			EXTRACT_FBDCHAN_INDX(x)	(((x)>>28) & 0x3)
#define			FERR_FAT_FBDCHAN 0x30000000
#define			FERR_FAT_M3ERR	0x00000004
#define			FERR_FAT_M2ERR	0x00000002
#define			FERR_FAT_M1ERR	0x00000001
#define			FERR_FAT_MASK	(FERR_FAT_M1ERR | \
						FERR_FAT_M2ERR | \
						FERR_FAT_M3ERR)

#define		FERR_NF_FBD		0xA0

/* Thermal and SPD or BFD errors */
#define			FERR_NF_M28ERR	0x01000000
#define			FERR_NF_M27ERR	0x00800000
#define			FERR_NF_M26ERR	0x00400000
#define			FERR_NF_M25ERR	0x00200000
#define			FERR_NF_M24ERR	0x00100000
#define			FERR_NF_M23ERR	0x00080000
#define			FERR_NF_M22ERR	0x00040000
#define			FERR_NF_M21ERR	0x00020000

/* Correctable errors */
#define			FERR_NF_M20ERR	0x00010000
#define			FERR_NF_M19ERR	0x00008000
#define			FERR_NF_M18ERR	0x00004000
#define			FERR_NF_M17ERR	0x00002000

/* Non-Retry or redundant Retry errors */
#define			FERR_NF_M16ERR	0x00001000
#define			FERR_NF_M15ERR	0x00000800
#define			FERR_NF_M14ERR	0x00000400
#define			FERR_NF_M13ERR	0x00000200

/* Uncorrectable errors */
#define			FERR_NF_M12ERR	0x00000100
#define			FERR_NF_M11ERR	0x00000080
#define			FERR_NF_M10ERR	0x00000040
#define			FERR_NF_M9ERR	0x00000020
#define			FERR_NF_M8ERR	0x00000010
#define			FERR_NF_M7ERR	0x00000008
#define			FERR_NF_M6ERR	0x00000004
#define			FERR_NF_M5ERR	0x00000002
#define			FERR_NF_M4ERR	0x00000001

#define			FERR_NF_UNCORRECTABLE	(FERR_NF_M12ERR | \
							FERR_NF_M11ERR | \
							FERR_NF_M10ERR | \
							FERR_NF_M8ERR | \
							FERR_NF_M7ERR | \
							FERR_NF_M6ERR | \
							FERR_NF_M5ERR | \
							FERR_NF_M4ERR)
#define			FERR_NF_CORRECTABLE	(FERR_NF_M20ERR | \
							FERR_NF_M19ERR | \
							FERR_NF_M18ERR | \
							FERR_NF_M17ERR)
#define			FERR_NF_DIMM_SPARE	(FERR_NF_M27ERR | \
							FERR_NF_M28ERR)
#define			FERR_NF_THERMAL		(FERR_NF_M26ERR | \
							FERR_NF_M25ERR | \
							FERR_NF_M24ERR | \
							FERR_NF_M23ERR)
#define			FERR_NF_SPD_PROTOCOL	(FERR_NF_M22ERR)
#define			FERR_NF_NORTH_CRC	(FERR_NF_M21ERR)
#define			FERR_NF_NON_RETRY	(FERR_NF_M13ERR | \
							FERR_NF_M14ERR | \
							FERR_NF_M15ERR)

#define		NERR_NF_FBD		0xA4
#define			FERR_NF_MASK		(FERR_NF_UNCORRECTABLE | \
							FERR_NF_CORRECTABLE | \
							FERR_NF_DIMM_SPARE | \
							FERR_NF_THERMAL | \
							FERR_NF_SPD_PROTOCOL | \
							FERR_NF_NORTH_CRC | \
							FERR_NF_NON_RETRY)

#define		EMASK_FBD		0xA8
#define			EMASK_FBD_M28ERR	0x08000000
#define			EMASK_FBD_M27ERR	0x04000000
#define			EMASK_FBD_M26ERR	0x02000000
#define			EMASK_FBD_M25ERR	0x01000000
#define			EMASK_FBD_M24ERR	0x00800000
#define			EMASK_FBD_M23ERR	0x00400000
#define			EMASK_FBD_M22ERR	0x00200000
#define			EMASK_FBD_M21ERR	0x00100000
#define			EMASK_FBD_M20ERR	0x00080000
#define			EMASK_FBD_M19ERR	0x00040000
#define			EMASK_FBD_M18ERR	0x00020000
#define			EMASK_FBD_M17ERR	0x00010000

#define			EMASK_FBD_M15ERR	0x00004000
#define			EMASK_FBD_M14ERR	0x00002000
#define			EMASK_FBD_M13ERR	0x00001000
#define			EMASK_FBD_M12ERR	0x00000800
#define			EMASK_FBD_M11ERR	0x00000400
#define			EMASK_FBD_M10ERR	0x00000200
#define			EMASK_FBD_M9ERR		0x00000100
#define			EMASK_FBD_M8ERR		0x00000080
#define			EMASK_FBD_M7ERR		0x00000040
#define			EMASK_FBD_M6ERR		0x00000020
#define			EMASK_FBD_M5ERR		0x00000010
#define			EMASK_FBD_M4ERR		0x00000008
#define			EMASK_FBD_M3ERR		0x00000004
#define			EMASK_FBD_M2ERR		0x00000002
#define			EMASK_FBD_M1ERR		0x00000001

#define			ENABLE_EMASK_FBD_FATAL_ERRORS	(EMASK_FBD_M1ERR | \
							EMASK_FBD_M2ERR | \
							EMASK_FBD_M3ERR)

#define 		ENABLE_EMASK_FBD_UNCORRECTABLE	(EMASK_FBD_M4ERR | \
							EMASK_FBD_M5ERR | \
							EMASK_FBD_M6ERR | \
							EMASK_FBD_M7ERR | \
							EMASK_FBD_M8ERR | \
							EMASK_FBD_M9ERR | \
							EMASK_FBD_M10ERR | \
							EMASK_FBD_M11ERR | \
							EMASK_FBD_M12ERR)
#define 		ENABLE_EMASK_FBD_CORRECTABLE	(EMASK_FBD_M17ERR | \
							EMASK_FBD_M18ERR | \
							EMASK_FBD_M19ERR | \
							EMASK_FBD_M20ERR)
#define			ENABLE_EMASK_FBD_DIMM_SPARE	(EMASK_FBD_M27ERR | \
							EMASK_FBD_M28ERR)
#define			ENABLE_EMASK_FBD_THERMALS	(EMASK_FBD_M26ERR | \
							EMASK_FBD_M25ERR | \
							EMASK_FBD_M24ERR | \
							EMASK_FBD_M23ERR)
#define			ENABLE_EMASK_FBD_SPD_PROTOCOL	(EMASK_FBD_M22ERR)
#define			ENABLE_EMASK_FBD_NORTH_CRC	(EMASK_FBD_M21ERR)
#define			ENABLE_EMASK_FBD_NON_RETRY	(EMASK_FBD_M15ERR | \
							EMASK_FBD_M14ERR | \
							EMASK_FBD_M13ERR)

#define		ENABLE_EMASK_ALL	(ENABLE_EMASK_FBD_NON_RETRY | \
					ENABLE_EMASK_FBD_NORTH_CRC | \
					ENABLE_EMASK_FBD_SPD_PROTOCOL | \
					ENABLE_EMASK_FBD_THERMALS | \
					ENABLE_EMASK_FBD_DIMM_SPARE | \
					ENABLE_EMASK_FBD_FATAL_ERRORS | \
					ENABLE_EMASK_FBD_CORRECTABLE | \
					ENABLE_EMASK_FBD_UNCORRECTABLE)

#define		ERR0_FBD		0xAC
#define		ERR1_FBD		0xB0
#define		ERR2_FBD		0xB4
#define		MCERR_FBD		0xB8


/* Offsets for Function 2 */

/*
 * Device 21,
 * Function 0: Memory Map Branch 0
 * Function 1: Error info
 *
 * Device 22,
 * Function 0: Memory Map Branch 1
 * Function 1: Error info
 */
#define PCI_DEVICE_ID_I5400_BRANCH_0	0x4035
#define PCI_DEVICE_ID_I5400_BRANCH_1	0x4036

/* Function 0 */
#define AMB_PRESENT_0	0x64
#define AMB_PRESENT_1	0x66
#define MTR0		0x80
#define MTR1		0x82
#define MTR2		0x84
#define MTR3		0x86

/* Function 1 */
#define	NRECFGLOG	0x74
#define	REDMEMB		0x7C
#define		RED_ECC_LOCATOR(x)	((x) & 0x3FFFF)
#define		REC_ECC_LOCATOR_EVEN(x)	((x) & 0x001FF)
#define		REC_ECC_LOCATOR_ODD(x)	((x) & 0x3FE00)

#define	NRECMEMA		0xBE
#define		NREC_BANK(x)		(((x) >> 12) & 0x7)
#define		NREC_RANK(x)		(((x) >> 8) & 0xF)
#define	NRECMEMB		0xC0
#define		NREC_RDWR(x)		(((x) >> 31) & 1)
#define		NREC_MERR(x)		(((x) >> 29) & 3)
#define		NREC_CAS(x)		(((x) >> 16) & 0x1FFF)
#define		NREC_RAS(x)		((x) & 0xFFFF)
#define	NREEECFBDA		0xC4
#define	NREEECFBDB		0xC8
#define	NREEECFBDC		0xCC
#define	NREEECFBDD		0xD0
#define	NREEECFBDE		0xD4
#define	NREEECFBDF		0xD8
#define	REDMEMA			0xDC
#define	RECMEMA			0xE0
#define		REC_MERR(x)		(((x) >> 15) & 0x1F)
#define		REC_BANK(x)		(((x) >> 12) & 0x7)
#define		REC_RANK(x)		(((x) >> 8) & 0xF)
#define	RECMEMB			0xE4
#define		REC_RDWR(x)		(((x) >> 31) & 1)
#define		REC_CAS(x)		(((x) >> 16) & 0x1FFF)
#define		REC_RAS(x)		((x) & 0xFFFF)
#define	RECFGLOG		0x78
#define	RECFBDA			0xE8
#define	RECFBDB			0xEC
#define	RECFBDC			0xF0
#define	RECFBDD			0xF4
#define	RECFBDE			0xF8
#define	RECFBDF			0xFC

#define NUM_MTRS		4
#define CHANNELS_PER_BRANCH	(2)

/* Defines to extract the various fields from the
 *	MTRx - Memory Technology Registers
 */
#define MTR_DIMMS_PRESENT(mtr)		((mtr) & (0x1 << 10))
#define MTR_DRAM_WIDTH(mtr)		((((mtr) >> 8) & 0x1) ? 8 : 4)
#define MTR_DRAM_BANKS(mtr)		((((mtr) >> 6) & 0x1) ? 8 : 4)
#define MTR_DRAM_BANKS_ADDR_BITS(mtr)	((MTR_DRAM_BANKS(mtr) == 8) ? 3 : 2)
#define MTR_DIMM_RANK(mtr)		(((mtr) >> 5) & 0x1)
#define MTR_DIMM_RANK_ADDR_BITS(mtr)	(MTR_DIMM_RANK(mtr) ? 2 : 1)
#define MTR_DIMM_ROWS(mtr)		(((mtr) >> 2) & 0x3)
#define MTR_DIMM_ROWS_ADDR_BITS(mtr)	(MTR_DIMM_ROWS(mtr) + 13)
#define MTR_DIMM_COLS(mtr)		((mtr) & 0x3)
#define MTR_DIMM_COLS_ADDR_BITS(mtr)	(MTR_DIMM_COLS(mtr) + 10)

#ifdef CONFIG_EDAC_DEBUG
static char *numrow_toString[] = {
	"8,192 - 13 rows",
	"16,384 - 14 rows",
	"32,768 - 15 rows",
	"reserved"
};

static char *numcol_toString[] = {
	"1,024 - 10 columns",
	"2,048 - 11 columns",
	"4,096 - 12 columns",
	"reserved"
};
#endif

/* Enumeration of supported devices */
enum i5400_chips {
	I5400A = 0,
	I5400B = 1
};

/* Device name and register DID (Device ID) */
struct i5400_dev_info {
	const char *ctl_name;	/* name for this device */
	u16 fsb_mapping_errors;	/* DID for the branchmap,control */
};

/* Table of devices attributes supported by this driver */
static const struct i5400_dev_info i5400_devs[] = {
	[I5400A] = {
		.ctl_name = "I5400A",
		.fsb_mapping_errors = PCI_DEVICE_ID_INTEL_I5400_DEV16,
	},
	[I5400B] = {
		.ctl_name = "I5400B",
		.fsb_mapping_errors = PCI_DEVICE_ID_INTEL_I5400_DEV16,
	},
};

struct i5400_dimm_info {
	int megabytes;		/* size, 0 means not present  */
	int dual_rank;
};

#define	MAX_CHANNELS	6	/* max possible channels */
#define MAX_CSROWS	(8 * 2)	/* max possible csrows per channel */

/* driver private data structure */
struct i5400_pvt {
	struct pci_dev *dev5400;	/* 0.0 */
	struct pci_dev *system_address;	/* 16.0 */
	struct pci_dev *branchmap_werrors;	/* 16.1 */
	struct pci_dev *fsb_error_regs;	/* 16.2 */
	struct pci_dev *branch_0f0;	/* 21.0 */
	struct pci_dev *branch_0f1;	/* 21.1 */
	struct pci_dev *branch_1f0;	/* 22.0 */
	struct pci_dev *branch_1f1;	/* 22.1 */

	u16 tolm;		/* top of low memory */
	u64 ambase;		/* AMB BAR */

	u16 mir0, mir1 /* , mir2 */ ;

	u16 b0_mtr[NUM_MTRS];	/* Memory Technlogy Reg */
	u16 b0_ambpresent0;	/* Branch 0, Channel 0 */
	u16 b0_ambpresent1;	/* Brnach 0, Channel 1 */

	u16 b1_mtr[NUM_MTRS];	/* Memory Technlogy Reg */
	u16 b1_ambpresent0;	/* Branch 1, Channel 8 */
	u16 b1_ambpresent1;	/* Branch 1, Channel 1 */

	/* DIMM infomation matrix, allocating architecture maximums */
	struct i5400_dimm_info dimm_info[MAX_CSROWS][MAX_CHANNELS];

	/* Actual values for this controller */
	int maxch;		/* Max channels */
	int maxdimmperch;	/* Max DIMMs per channel */
};

/* I5400 MCH error information retrieved from Hardware */
struct i5400_error_info {
	/* These registers are always read from the MC */
	u32 ferr_fat_fbd;	/* First Errors Fatal */
	u32 nerr_fat_fbd;	/* Next Errors Fatal */
	u32 ferr_nf_fbd;	/* First Errors Non-Fatal */
	u32 nerr_nf_fbd;	/* Next Errors Non-Fatal */

	/* These registers are input ONLY if there was a Recoverable Error */
	u32 redmemb;		/* Recoverable Mem Data Error log B */
	u16 recmema;		/* Recoverable Mem Error log A */
	u32 recmemb;		/* Recoverable Mem Error log B */

	/* These registers are input ONLY if there was a
	 * Non-Recoverable Error */
	u16 nrecmema;		/* Non-Recoverable Mem log A */
	u16 nrecmemb;		/* Non-Recoverable Mem log B */
};

static struct edac_pci_ctl_info *i5400_pci;

struct memctrl_dev_attribute {
        struct attribute attr;
        int *value;
        ssize_t (*show)(void *,char *);
        ssize_t (*store)(void *, const char *, size_t);
};


/*
 *	i5400_get_error_info	Retrieve the hardware error information from
 *				the hardware and cache it in the 'info'
 *				structure
 */
static void i5400_get_error_info(struct mem_ctl_info *mci,
				 struct i5400_error_info *info)
{
	struct i5400_pvt *pvt;
	u32 value;

	pvt = mci->pvt_info;

	/* read in the 1st FATAL error register */
	pci_read_config_dword(pvt->branchmap_werrors, FERR_FAT_FBD, &value);

	/* Mask only the bits that the doc says are valid
	 */
	value &= (FERR_FAT_FBDCHAN | FERR_FAT_MASK);

	/* If there is an error, then read in the */
	/* NEXT FATAL error register and the Memory Error Log Register A */
	if (value & FERR_FAT_MASK) {
		info->ferr_fat_fbd = value;

		/* harvest the various error data we need */
		pci_read_config_dword(pvt->branchmap_werrors,
				NERR_FAT_FBD, &info->nerr_fat_fbd);
		pci_read_config_word(pvt->branchmap_werrors,
				NRECMEMA, &info->nrecmema);
		pci_read_config_word(pvt->branchmap_werrors,
				NRECMEMB, &info->nrecmemb);

		/* Clear the error bits, by writing them back */
		pci_write_config_dword(pvt->branchmap_werrors,
				FERR_FAT_FBD, value);
	} else {
		info->ferr_fat_fbd = 0;
		info->nerr_fat_fbd = 0;
		info->nrecmema = 0;
		info->nrecmemb = 0;
	}

	/* read in the 1st NON-FATAL error register */
	pci_read_config_dword(pvt->branchmap_werrors, FERR_NF_FBD, &value);

	/* If there is an error, then read in the 1st NON-FATAL error
	 * register as well */
	if (value & FERR_NF_MASK) {
		info->ferr_nf_fbd = value;

		/* harvest the various error data we need */
		pci_read_config_dword(pvt->branchmap_werrors,
				NERR_NF_FBD, &info->nerr_nf_fbd);
		pci_read_config_word(pvt->branchmap_werrors,
				RECMEMA, &info->recmema);
		pci_read_config_dword(pvt->branchmap_werrors,
				RECMEMB, &info->recmemb);
		pci_read_config_dword(pvt->branchmap_werrors,
				REDMEMB, &info->redmemb);

		/* Clear the error bits, by writing them back */
		pci_write_config_dword(pvt->branchmap_werrors,
				FERR_NF_FBD, value);
	} else {
		info->ferr_nf_fbd = 0;
		info->nerr_nf_fbd = 0;
		info->recmema = 0;
		info->recmemb = 0;
		info->redmemb = 0;
	}
}

/*
 * i5400_process_fatal_error_info(struct mem_ctl_info *mci,
 * 					struct i5400_error_info *info,
 * 					int handle_errors);
 *
 *	handle the Intel FATAL errors, if any
 */
static void i5400_process_fatal_error_info(struct mem_ctl_info *mci,
					struct i5400_error_info *info,
					int handle_errors)
{
	char msg[EDAC_MC_LABEL_LEN + 1 + 90];
	u32 allErrors;
	int branch;
	int channel;
	int bank;
	int rank;
	int rdwr;
	int ras, cas;

	/* mask off the Error bits that are possible */
	allErrors = (info->ferr_fat_fbd & FERR_FAT_MASK);
	if (!allErrors)
		return;		/* if no error, return now */

	/* ONLY ONE of the possible error bits will be set, as per the docs */
	i5400_mc_printk(mci, KERN_ERR,
			"FATAL ERRORS Found!!! 1st FATAL Err Reg= 0x%x\n",
			allErrors);

	branch = EXTRACT_FBDCHAN_INDX(info->ferr_fat_fbd);
	channel = branch;

	/* Use the NON-Recoverable macros to extract data */
	bank = NREC_BANK(info->nrecmema);
	rank = NREC_RANK(info->nrecmema);
	rdwr = NREC_RDWR(info->nrecmema);
	ras = NREC_RAS(info->nrecmemb);
	cas = NREC_CAS(info->nrecmemb);

	debugf0("\t\tCSROW= %d  Channels= %d,%d  (Branch= %d "
		"DRAM Bank= %d rdwr= %s ras= %d cas= %d)\n",
		rank, channel, channel + 1, branch >> 1, bank,
		rdwr ? "Write" : "Read", ras, cas);

	/* Only 1 bit will be on */
	if (allErrors & FERR_FAT_M1ERR) {
		i5400_mc_printk(mci, KERN_ERR,
				"Alert on non-redundant retry or fast "
				"reset timeout\n");

	} else if (allErrors & FERR_FAT_M2ERR) {
		i5400_mc_printk(mci, KERN_ERR,
				"Northbound CRC error on non-redundant "
				"retry\n");

	} else if (allErrors & FERR_FAT_M3ERR) {
		i5400_mc_printk(mci, KERN_ERR,
				">Tmid Thermal event with intelligent "
				"throttling disabled\n");
	}

	/* Form out message */
	snprintf(msg, sizeof(msg),
		 "(Branch=%d DRAM-Bank=%d RDWR=%s RAS=%d CAS=%d "
		 "FATAL Err=0x%x)",
		 branch >> 1, bank, rdwr ? "Write" : "Read", ras, cas,
		 allErrors);

	/* Call the helper to output message */
	edac_mc_handle_fbd_ue(mci, rank, channel, channel + 1, msg);
}

/*
 * i5400_process_fatal_error_info(struct mem_ctl_info *mci,
 * 				struct i5400_error_info *info,
 * 				int handle_errors);
 *
 *	handle the Intel NON-FATAL errors, if any
 */
static void i5400_process_nonfatal_error_info(struct mem_ctl_info *mci,
					struct i5400_error_info *info,
					int handle_errors)
{
	char msg[EDAC_MC_LABEL_LEN + 1 + 90];
	u32 allErrors;
	u32 ue_errors;
	u32 ce_errors;
	u32 misc_errors;
	int branch;
	int channel;
	int bank;
	int rank;
	int rdwr;
	int ras, cas;

	/* mask off the Error bits that are possible */
	allErrors = (info->ferr_nf_fbd & FERR_NF_MASK);
	if (!allErrors)
		return;		/* if no error, return now */

	/* ONLY ONE of the possible error bits will be set, as per the docs */
	i5400_mc_printk(mci, KERN_WARNING,
			"NON-FATAL ERRORS Found!!! 1st NON-FATAL Err "
			"Reg= 0x%x\n", allErrors);

	ue_errors = allErrors & FERR_NF_UNCORRECTABLE;
	if (ue_errors) {
		debugf0("\tUncorrected bits= 0x%x\n", ue_errors);

		branch = EXTRACT_FBDCHAN_INDX(info->ferr_nf_fbd);
		channel = branch;
		bank = NREC_BANK(info->nrecmema);
		rank = NREC_RANK(info->nrecmema);
		rdwr = NREC_RDWR(info->nrecmema);
		ras = NREC_RAS(info->nrecmemb);
		cas = NREC_CAS(info->nrecmemb);

		debugf0
			("\t\tCSROW= %d  Channels= %d,%d  (Branch= %d "
			"DRAM Bank= %d rdwr= %s ras= %d cas= %d)\n",
			rank, channel, channel + 1, branch >> 1, bank,
			rdwr ? "Write" : "Read", ras, cas);

		/* Form out message */
		snprintf(msg, sizeof(msg),
			 "(Branch=%d DRAM-Bank=%d RDWR=%s RAS=%d "
			 "CAS=%d, UE Err=0x%x)",
			 branch >> 1, bank, rdwr ? "Write" : "Read", ras, cas,
			 ue_errors);

		/* Call the helper to output message */
		edac_mc_handle_fbd_ue(mci, rank, channel, channel + 1, msg);
	}

	/* Check correctable errors */
	ce_errors = allErrors & FERR_NF_CORRECTABLE;
	if (ce_errors) {
		debugf0("\tCorrected bits= 0x%x\n", ce_errors);

		branch = EXTRACT_FBDCHAN_INDX(info->ferr_nf_fbd);

		channel = 0;
		if (REC_ECC_LOCATOR_ODD(info->redmemb))
			channel = 1;

		/* Convert channel to be based from zero, instead of
		 * from branch base of 0 */
		channel += branch;

		bank = REC_BANK(info->recmema);
		rank = REC_RANK(info->recmema);
		rdwr = REC_RDWR(info->recmema);
		ras = REC_RAS(info->recmemb);
		cas = REC_CAS(info->recmemb);

		debugf0("\t\tCSROW= %d Channel= %d  (Branch %d "
			"DRAM Bank= %d rdwr= %s ras= %d cas= %d)\n",
			rank, channel, branch >> 1, bank,
			rdwr ? "Write" : "Read", ras, cas);

		/* Form out message */
		snprintf(msg, sizeof(msg),
			 "(Branch=%d DRAM-Bank=%d RDWR=%s RAS=%d "
			 "CAS=%d, CE Err=0x%x)", branch >> 1, bank,
			 rdwr ? "Write" : "Read", ras, cas, ce_errors);

		/* Call the helper to output message */
		edac_mc_handle_fbd_ce(mci, rank, channel, msg);
	}

	/* See if any of the thermal errors have fired */
	misc_errors = allErrors & FERR_NF_THERMAL;
	if (misc_errors) {
		i5400_printk(KERN_WARNING, "\tTHERMAL Error, bits= 0x%x\n",
			misc_errors);
	}

	/* See if any of the thermal errors have fired */
	misc_errors = allErrors & FERR_NF_NON_RETRY;
	if (misc_errors) {
		i5400_printk(KERN_WARNING, "\tNON-Retry  Errors, bits= 0x%x\n",
			misc_errors);
	}

	/* See if any of the thermal errors have fired */
	misc_errors = allErrors & FERR_NF_NORTH_CRC;
	if (misc_errors) {
		i5400_printk(KERN_WARNING,
			"\tNORTHBOUND CRC  Error, bits= 0x%x\n",
			misc_errors);
	}

	/* See if any of the thermal errors have fired */
	misc_errors = allErrors & FERR_NF_SPD_PROTOCOL;
	if (misc_errors) {
		i5400_printk(KERN_WARNING,
			"\tSPD Protocol  Error, bits= 0x%x\n",
			misc_errors);
	}

	/* See if any of the thermal errors have fired */
	misc_errors = allErrors & FERR_NF_DIMM_SPARE;
	if (misc_errors) {
		i5400_printk(KERN_WARNING, "\tDIMM-Spare  Error, bits= 0x%x\n",
			misc_errors);
	}
}

/*
 *	i5400_process_error_info	Process the error info that is
 *	in the 'info' structure, previously retrieved from hardware
 */
static void i5400_process_error_info(struct mem_ctl_info *mci,
				struct i5400_error_info *info,
				int handle_errors)
{
	/* First handle any fatal errors that occurred */
	i5400_process_fatal_error_info(mci, info, handle_errors);

	/* now handle any non-fatal errors that occurred */
	i5400_process_nonfatal_error_info(mci, info, handle_errors);
}

/*
 *	i5400_clear_error	Retrieve any error from the hardware
 *				but do NOT process that error.
 *				Used for 'clearing' out of previous errors
 *				Called by the Core module.
 */
static void i5400_clear_error(struct mem_ctl_info *mci)
{
	struct i5400_error_info info;

	i5400_get_error_info(mci, &info);
}

/*
 *	i5400_check_error	Retrieve and process errors reported by the
 *				hardware. Called by the Core module.
 */
static void i5400_check_error(struct mem_ctl_info *mci)
{
	struct i5400_error_info info;
	debugf4("MC%d: " __FILE__ ": %s()\n", mci->mc_idx, __func__);
	i5400_get_error_info(mci, &info);
	i5400_process_error_info(mci, &info, 1);
}

/*
 * i5400_get_dev16 - Get a 5400 device 16 function
 * @func: function of device 16 to find
 */

static struct pci_dev *
i5400_get_dev16(int func)
{
	struct pci_dev *pdev;

	pdev = NULL;
	for (;;) {
		pdev = pci_get_device(PCI_VENDOR_ID_INTEL,
			PCI_DEVICE_ID_INTEL_I5400_DEV16, pdev);

		if (PCI_FUNC(pdev->devfn) == func)
			break;

		pci_dev_put(pdev);
	}

	if (!pdev) {
		i5400_printk(KERN_ERR, "dev 16 func not found "
			"vendor: 0x%x device 0x%x func %d (broken BIOS?)\n",
			PCI_VENDOR_ID_INTEL,
			PCI_DEVICE_ID_INTEL_I5400_DEV16, func);
	}

	return pdev;
}

/*
 *	i5400_get_devices	Find and perform 'get' operation on the MCH's
 *			device/functions we want to reference for this driver
 *
 *			Need to 'get' device 16 func 1 and func 2
 */
static int i5400_get_devices(struct mem_ctl_info *mci, int dev_idx)
{
	struct i5400_pvt *pvt;
	struct pci_dev *pdev;

	pvt = mci->pvt_info;

	/* Attempt to 'get' the MCH register we want */
	pdev = i5400_get_dev16(1);
	if (!pdev)
		return 1;

	pvt->branchmap_werrors = pdev;

	/* Attempt to 'get' the MCH register we want */
	pdev = i5400_get_dev16(2);
	if (!pdev)
		goto out1;

	pvt->fsb_error_regs = pdev;

	debugf1("System Address, processor bus- PCI Bus ID: %s  %x:%x\n",
		pci_name(pvt->system_address),
		pvt->system_address->vendor, pvt->system_address->device);
	debugf1("Branchmap, control and errors - PCI Bus ID: %s  %x:%x\n",
		pci_name(pvt->branchmap_werrors),
		pvt->branchmap_werrors->vendor, pvt->branchmap_werrors->device);
	debugf1("FSB Error Regs - PCI Bus ID: %s  %x:%x\n",
		pci_name(pvt->fsb_error_regs),
		pvt->fsb_error_regs->vendor, pvt->fsb_error_regs->device);

	pdev = NULL;
	pdev = pci_get_device(PCI_VENDOR_ID_INTEL,
			PCI_DEVICE_ID_I5400_BRANCH_0, pdev);

	if (pdev == NULL) {
		i5400_printk(KERN_ERR,
			"MC: 'BRANCH 0' device not found:"
			"vendor 0x%x device 0x%x Func 0 (broken BIOS?)\n",
			PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_I5400_BRANCH_0);
		goto out2;
	}

	if (PCI_FUNC(pdev->devfn) != 0) {
		i5400_printk(KERN_ERR, "MC: Wrong branch device = %d\n",
			PCI_FUNC(pdev->devfn));
		pci_dev_put(pdev);
		goto out2;
	}

	pvt->branch_0f0 = pdev;
	pdev = pci_get_device(PCI_VENDOR_ID_INTEL,
			PCI_DEVICE_ID_I5400_BRANCH_0, pdev);

	if (pdev == NULL) {
		i5400_printk(KERN_ERR,
			"MC: 'BRANCH 0' device f1 not found:"
			"vendor 0x%x device 0x%x Func 1 (broken BIOS?)\n",
			PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_I5400_BRANCH_0);
		goto out3;
	}

	if (PCI_FUNC(pdev->devfn) != 1) {
		i5400_printk(KERN_ERR, "MC: Wrong branch device = %d\n",
			PCI_FUNC(pdev->devfn));
		pci_dev_put(pdev);
		goto out3;
	}
	pvt->branch_0f1 = pdev;

	/* If this device claims to have more than 2 channels then
	 * fetch Branch 1's information
	 */
	if (pvt->maxch < CHANNELS_PER_BRANCH)
		return 0;	/* If no more channels */

	pdev = NULL;
	pdev = pci_get_device(PCI_VENDOR_ID_INTEL,
			PCI_DEVICE_ID_I5400_BRANCH_1, pdev);

	if (pdev == NULL) {
		i5400_printk(KERN_ERR,
			"MC: 'BRANCH 1' device not found:"
			"vendor 0x%x device 0x%x Func 0 "
			"(broken BIOS?)\n",
			PCI_VENDOR_ID_INTEL,
			PCI_DEVICE_ID_I5400_BRANCH_1);
		goto out4;
	}

	pvt->branch_1f0 = pdev;
	pdev = pci_get_device(PCI_VENDOR_ID_INTEL,
			PCI_DEVICE_ID_I5400_BRANCH_1, pdev);

	if (pdev == NULL) {
		i5400_printk(KERN_ERR,
			"MC: 'BRANCH 1' device f1 not found:"
			"vendor 0x%x device 0x%x Func 1 (broken BIOS?)\n",
			PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_I5400_BRANCH_1);
		goto out5;
	}

	if (PCI_FUNC(pdev->devfn) != 1) {
		i5400_printk(KERN_ERR, "MC: Wrong branch device = %d\n",
			PCI_FUNC(pdev->devfn));
		pci_dev_put(pdev);
		goto out5;
	}
	pvt->branch_1f1 = pdev;

	return 0;

out5:
	pci_dev_put(pvt->branch_1f0);
out4:
	pci_dev_put(pvt->branch_0f1);
out3:
	pci_dev_put(pvt->branch_0f0);
out2:
	pci_dev_put(pvt->fsb_error_regs);
out1:
	pci_dev_put(pvt->branchmap_werrors);
	return 1;
}

/*
 *	i5400_put_devices	'put' all the devices that we have
 *				reserved via 'get'
 */
static void i5400_put_devices(struct mem_ctl_info *mci)
{
	struct i5400_pvt *pvt;

	pvt = mci->pvt_info;

	pci_dev_put(pvt->system_address);	/* FUNC 0 */
	pci_dev_put(pvt->branchmap_werrors);	/* FUNC 1 */
	pci_dev_put(pvt->fsb_error_regs);	/* FUNC 2 */
	pci_dev_put(pvt->branch_0f0);	/* DEV 21 */
	pci_dev_put(pvt->branch_0f1);	/* DEV 21 */

	/* Only if more than 2 channels do we release the second branch */
	if (pvt->maxch >= CHANNELS_PER_BRANCH) {
		pci_dev_put(pvt->branch_1f0);	/* DEV 22 */
		pci_dev_put(pvt->branch_1f1);	/* DEV 22 */
	}
}

/*
 *	determine_amb_present
 *
 *		the information is contained in NUM_MTRS different registers
 *		determineing which of the NUM_MTRS requires knowing
 *		which channel is in question
 *
 *	2 branches, each with 2 channels
 *		b0_ambpresent0 for channel '0'
 *		b0_ambpresent1 for channel '1'
 *		b1_ambpresent0 for channel '2'
 *		b1_ambpresent1 for channel '3'
 */
static int determine_amb_present_reg(struct i5400_pvt *pvt, int channel)
{
	int amb_present;

	if (channel < CHANNELS_PER_BRANCH) {
		if (channel & 0x1)
			amb_present = pvt->b0_ambpresent1;
		else
			amb_present = pvt->b0_ambpresent0;
	} else {
		if (channel & 0x1)
			amb_present = pvt->b1_ambpresent1;
		else
			amb_present = pvt->b1_ambpresent0;
	}

	return amb_present;
}

/*
 * determine_mtr(pvt, csrow, channel)
 *
 *	return the proper MTR register as determine by the csrow and channel desired
 */
static int determine_mtr(struct i5400_pvt *pvt, int csrow, int channel)
{
	int mtr;

	if (channel < CHANNELS_PER_BRANCH)
		mtr = pvt->b0_mtr[csrow >> 1];
	else
		mtr = pvt->b1_mtr[csrow >> 1];

	return mtr;
}

/*
 */
static void decode_mtr(int slot_row, u16 mtr)
{
	int ans;

	ans = MTR_DIMMS_PRESENT(mtr);

	debugf2("\tMTR%d=0x%x:  DIMMs are %s\n", slot_row, mtr,
		ans ? "Present" : "NOT Present");
	if (!ans)
		return;

	debugf2("\t\tWIDTH: x%d\n", MTR_DRAM_WIDTH(mtr));
	debugf2("\t\tNUMBANK: %d bank(s)\n", MTR_DRAM_BANKS(mtr));
	debugf2("\t\tNUMRANK: %s\n", MTR_DIMM_RANK(mtr) ? "double" : "single");
	debugf2("\t\tNUMROW: %s\n", numrow_toString[MTR_DIMM_ROWS(mtr)]);
	debugf2("\t\tNUMCOL: %s\n", numcol_toString[MTR_DIMM_COLS(mtr)]);
}

static void handle_channel(struct i5400_pvt *pvt, int csrow, int channel,
			struct i5400_dimm_info *dinfo)
{
	int mtr;
	int amb_present_reg;
	int addrBits;

	mtr = determine_mtr(pvt, csrow, channel);
	if (MTR_DIMMS_PRESENT(mtr)) {
		amb_present_reg = determine_amb_present_reg(pvt, channel);

		/* Determine if there is  a  DIMM present in this DIMM slot */
		if (amb_present_reg & (1 << (csrow >> 1))) {
			dinfo->dual_rank = MTR_DIMM_RANK(mtr);

			if (!((dinfo->dual_rank == 0) &&
				((csrow & 0x1) == 0x1))) {
				/* Start with the number of bits for a Bank
				 * on the DRAM */
				addrBits = MTR_DRAM_BANKS_ADDR_BITS(mtr);
				/* Add thenumber of ROW bits */
				addrBits += MTR_DIMM_ROWS_ADDR_BITS(mtr);
				/* add the number of COLUMN bits */
				addrBits += MTR_DIMM_COLS_ADDR_BITS(mtr);

				addrBits += 6;	/* add 64 bits per DIMM */
				addrBits -= 20;	/* divide by 2^^20 */
				addrBits -= 3;	/* 8 bits per bytes */

				dinfo->megabytes = 1 << addrBits;
			}
		}
	}
}

/*
 *	calculate_dimm_size
 *
 *	also will output a DIMM matrix map, if debug is enabled, for viewing
 *	how the DIMMs are populated
 */
static void calculate_dimm_size(struct i5400_pvt *pvt)
{
	struct i5400_dimm_info *dinfo;
	int csrow, max_csrows;
	char *p, *mem_buffer;
	int space, n;
	int channel;

	/* ================= Generate some debug output ================= */
	space = PAGE_SIZE;
	mem_buffer = p = kmalloc(space, GFP_KERNEL);
	if (p == NULL) {
		i5400_printk(KERN_ERR, "MC: %s:%s() kmalloc() failed\n",
			__FILE__, __func__);
		return;
	}

	n = snprintf(p, space, "\n");
	p += n;
	space -= n;

	/* Scan all the actual CSROWS (which is # of DIMMS * 2)
	 * and calculate the information for each DIMM
	 * Start with the highest csrow first, to display it first
	 * and work toward the 0th csrow
	 */
	max_csrows = pvt->maxdimmperch * 2;
	for (csrow = max_csrows - 1; csrow >= 0; csrow--) {

		/* on an odd csrow, first output a 'boundary' marker,
		 * then reset the message buffer  */
		if (csrow & 0x1) {
			n = snprintf(p, space, "---------------------------"
				"--------------------------------");
			p += n;
			space -= n;
			debugf2("%s\n", mem_buffer);
			p = mem_buffer;
			space = PAGE_SIZE;
		}
		n = snprintf(p, space, "csrow %2d    ", csrow);
		p += n;
		space -= n;

		for (channel = 0; channel < pvt->maxch; channel++) {
			dinfo = &pvt->dimm_info[csrow][channel];
			handle_channel(pvt, csrow, channel, dinfo);
			n = snprintf(p, space, "%4d MB   | ", dinfo->megabytes);
			p += n;
			space -= n;
		}
		n = snprintf(p, space, "\n");
		p += n;
		space -= n;
	}

	/* Output the last bottom 'boundary' marker */
	n = snprintf(p, space, "---------------------------"
		"--------------------------------\n");
	p += n;
	space -= n;

	/* now output the 'channel' labels */
	n = snprintf(p, space, "            ");
	p += n;
	space -= n;
	for (channel = 0; channel < pvt->maxch; channel++) {
		n = snprintf(p, space, "channel %d | ", channel);
		p += n;
		space -= n;
	}
	n = snprintf(p, space, "\n");
	p += n;
	space -= n;

	/* output the last message and free buffer */
	debugf2("%s\n", mem_buffer);
	kfree(mem_buffer);
}

/*
 *	i5400_get_mc_regs	read in the necessary registers and
 *				cache locally
 *
 *			Fills in the private data members
 */
static void i5400_get_mc_regs(struct mem_ctl_info *mci)
{
	struct i5400_pvt *pvt;
	u32 actual_tolm;
	u16 limit;
	int slot_row;
	int maxch;
	int maxdimmperch;
	int way0, way1;

	pvt = mci->pvt_info;

	pci_read_config_dword(pvt->system_address, AMBASE,
			(u32 *) & pvt->ambase);
	pci_read_config_dword(pvt->system_address, AMBASE + sizeof(u32),
			((u32 *) & pvt->ambase) + sizeof(u32));

	maxdimmperch = pvt->maxdimmperch;
	maxch = pvt->maxch;

	debugf2("AMBASE= 0x%lx  MAXCH= %d  MAX-DIMM-Per-CH= %d\n",
		(long unsigned int)pvt->ambase, pvt->maxch, pvt->maxdimmperch);

	/* Get the Branch Map regs */
	pci_read_config_word(pvt->branchmap_werrors, TOLM, &pvt->tolm);
	pvt->tolm >>= 12;
	debugf2("\nTOLM (number of 256M regions) =%u (0x%x)\n", pvt->tolm,
		pvt->tolm);

	actual_tolm = pvt->tolm << 28;
	debugf2("Actual TOLM byte addr=%u (0x%x)\n", actual_tolm, actual_tolm);

	pci_read_config_word(pvt->branchmap_werrors, MIR0, &pvt->mir0);
	pci_read_config_word(pvt->branchmap_werrors, MIR1, &pvt->mir1);

	/* Get the MIR[0-1] regs */
	limit = (pvt->mir0 >> 4) & 0x0FFF;
	way0 = pvt->mir0 & 0x1;
	way1 = pvt->mir0 & 0x2;
	debugf2("MIR0: limit= 0x%x  WAY1= %u  WAY0= %x\n", limit, way1, way0);
	limit = (pvt->mir1 >> 4) & 0x0FFF;
	way0 = pvt->mir1 & 0x1;
	way1 = pvt->mir1 & 0x2;
	debugf2("MIR1: limit= 0x%x  WAY1= %u  WAY0= %x\n", limit, way1, way0);

	/* Get the MTR[0-3] regs */
	for (slot_row = 0; slot_row < NUM_MTRS; slot_row++) {
		int where = MTR0 + (slot_row * sizeof(u16));

		pci_read_config_word(pvt->branch_0f0, where,
				&pvt->b0_mtr[slot_row]);

		debugf2("MTR%d where=0x%x B0 value=0x%x\n", slot_row, where,
			pvt->b0_mtr[slot_row]);

		if (pvt->maxch >= CHANNELS_PER_BRANCH) {
			pci_read_config_word(pvt->branch_1f0, where,
					&pvt->b1_mtr[slot_row]);
			debugf2("MTR%d where=0x%x B1 value=0x%x\n", slot_row,
				where, pvt->b0_mtr[slot_row]);
		} else {
			pvt->b1_mtr[slot_row] = 0;
		}
	}

	/* Read and dump branch 0's MTRs */
	debugf2("\nMemory Technology Registers:\n");
	debugf2("   Branch 0:\n");
	for (slot_row = 0; slot_row < NUM_MTRS; slot_row++) {
		decode_mtr(slot_row, pvt->b0_mtr[slot_row]);
	}
	pci_read_config_word(pvt->branch_0f0, AMB_PRESENT_0,
			&pvt->b0_ambpresent0);
	debugf2("\t\tAMB-Branch 0-present0 0x%x:\n", pvt->b0_ambpresent0);
	pci_read_config_word(pvt->branch_0f0, AMB_PRESENT_1,
			&pvt->b0_ambpresent1);
	debugf2("\t\tAMB-Branch 0-present1 0x%x:\n", pvt->b0_ambpresent1);

	/* Only if we have 2 branchs (4 channels) */
	if (pvt->maxch < CHANNELS_PER_BRANCH) {
		pvt->b1_ambpresent0 = 0;
		pvt->b1_ambpresent1 = 0;
	} else {
		/* Read and dump  branch 1's MTRs */
		debugf2("   Branch 1:\n");
		for (slot_row = 0; slot_row < NUM_MTRS; slot_row++) {
			decode_mtr(slot_row, pvt->b1_mtr[slot_row]);
		}
		pci_read_config_word(pvt->branch_1f0, AMB_PRESENT_0,
				&pvt->b1_ambpresent0);
		debugf2("\t\tAMB-Branch 1-present0 0x%x:\n",
			pvt->b1_ambpresent0);
		pci_read_config_word(pvt->branch_1f0, AMB_PRESENT_1,
				&pvt->b1_ambpresent1);
		debugf2("\t\tAMB-Branch 1-present1 0x%x:\n",
			pvt->b1_ambpresent1);
	}

	/* Go and determine the size of each DIMM and place in an
	 * orderly matrix */
	calculate_dimm_size(pvt);
}

/*
 *	i5400_init_csrows	Initialize the 'csrows' table within
 *				the mci control	structure with the
 *				addressing of memory.
 *
 *	return:
 *		0	success
 *		1	no actual memory found on this MC
 */
static int i5400_init_csrows(struct mem_ctl_info *mci)
{
	struct i5400_pvt *pvt;
	struct csrow_info *p_csrow;
	int empty, channel_count;
	int max_csrows;
	int mtr;
	int csrow_megs;
	int channel;
	int csrow;

	pvt = mci->pvt_info;

	channel_count = pvt->maxch;
	max_csrows = pvt->maxdimmperch * 2;

	empty = 1;		/* Assume NO memory */

	for (csrow = 0; csrow < max_csrows; csrow++) {
		p_csrow = &mci->csrows[csrow];

		p_csrow->csrow_idx = csrow;

		/* use branch 0 for the basis */
		mtr = pvt->b0_mtr[csrow >> 1];

		/* if no DIMMS on this row, continue */
		if (!MTR_DIMMS_PRESENT(mtr))
			continue;

		/* FAKE OUT VALUES, FIXME */
		p_csrow->first_page = 0 + csrow * 20;
		p_csrow->last_page = 9 + csrow * 20;
		p_csrow->page_mask = 0xFFF;

		p_csrow->grain = 8;

		csrow_megs = 0;
		for (channel = 0; channel < pvt->maxch; channel++) {
			csrow_megs += pvt->dimm_info[csrow][channel].megabytes;
		}

		p_csrow->nr_pages = csrow_megs << 8;

		/* Assume DDR2 for now */
		p_csrow->mtype = MEM_RDDR;

		/* ask what device type on this row */
		if (MTR_DRAM_WIDTH(mtr))
			p_csrow->dtype = DEV_X8;
		else
			p_csrow->dtype = DEV_X4;

		p_csrow->edac_mode = EDAC_S8ECD8ED;

		empty = 0;
	}

	return empty;
}

/*
 *	i5400_enable_error_reporting
 *			Turn on the memory reporting features of the hardware
 */
static void i5400_enable_error_reporting(struct mem_ctl_info *mci)
{
	struct i5400_pvt *pvt;
	u32 fbd_error_mask;

	pvt = mci->pvt_info;

	/* Read the FBD Error Mask Register */
	pci_read_config_dword(pvt->branchmap_werrors, EMASK_FBD,
			&fbd_error_mask);

	/* Enable with a '0' */
	fbd_error_mask &= ~(ENABLE_EMASK_ALL);

	pci_write_config_dword(pvt->branchmap_werrors, EMASK_FBD,
			fbd_error_mask);
}

/*
 * i5400_get_dimm_and_channel_counts(pdev, &num_csrows, &num_channels)
 *
 *	ask the device how many channels are present and how many CSROWS
 *	 as well
 */
static void i5400_get_dimm_and_channel_counts(struct pci_dev *pdev,
					int *num_dimms_per_channel,
					int *num_channels)
{
	u8 value;

	/* Need to retrieve just how many channels and dimms per channel are
	 * supported on this memory controller
	 */
	pci_read_config_byte(pdev, MAXDIMMPERCH, &value);
	*num_dimms_per_channel = (int)value * 2;

	pci_read_config_byte(pdev, MAXCH, &value);
	*num_channels = (int)value;
}

#ifdef I5400_STOP_AND_SCREAM
/*
 * i5400_stop_and_scream
 * @pdev: device 0 of 5400
 */
static void i5400_stop_and_scream(struct pci_dev *pdev)
{
	int dev;
	u8 value;

	for (dev = 0; dev < 10; ++dev) {
		if (pdev) {
			pci_read_config_byte(pdev, SSCTRL, &value);
			pci_write_config_byte(pdev, SSCTRL, value | 1);
			i5400_printk(KERN_WARNING,
				"MC: Set stop and scream for device %d\n",
				dev);
		}

		if (dev)
			pci_dev_put(pdev);

		pdev = NULL;
		if (dev < 9)
			pdev = pci_get_device(PCI_VENDOR_ID_INTEL,
				PCI_DEVICE_ID_INTEL_I5400_DEV1 + dev, pdev);
	}
}
#endif /* I5400_STOP_AND_SCREAM */

/*
 *	i5400_probe1	Probe for ONE instance of device to see if it is
 *			present.
 *	return:
 *		0 for FOUND a device
 *		< 0 for error code
 */
static int i5400_probe1(struct pci_dev *pdev, int dev_idx)
{
	struct mem_ctl_info *mci;
	struct i5400_pvt *pvt;
	struct pci_dev *pdev16;
	int num_channels;
	int num_dimms_per_channel;
	int num_csrows;

	debugf0("MC: " __FILE__ ": %s(), pdev bus %u dev=0x%x fn=0x%x\n",
		__func__, pdev->bus->number,
		PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));

	/* We only are looking for func 0 of the set */
	if (PCI_FUNC(pdev->devfn) != 0)	{
		i5400_printk(KERN_ERR, "MC: exp func 0, got %d\n",
			PCI_FUNC(pdev->devfn));
		return -ENODEV;
	}

	pdev16 = i5400_get_dev16(0);
	if (!pdev16)
		return -ENODEV;

	/* Ask the devices for the number of CSROWS and CHANNELS so
	 * that we can calculate the memory resources, etc
	 *
	 * The Chipset will report what it can handle which will be greater
	 * or equal to what the motherboard manufacturer will implement.
	 *
	 * As we don't have a motherboard identification routine to determine
	 * actual number of slots/dimms per channel, we thus utilize the
	 * resource as specified by the chipset. Thus, we might have
	 * have more DIMMs per channel than actually on the mobo, but this
	 * allows the driver to support upto the chipset max, without
	 * some fancy mobo determination.
	 */
	i5400_get_dimm_and_channel_counts(pdev16, &num_dimms_per_channel,
					&num_channels);
	num_csrows = num_dimms_per_channel * 2;

	debugf0("MC: %s(): Number of - Channels= %d  DIMMS= %d  CSROWS= %d\n",
		__func__, num_channels, num_dimms_per_channel, num_csrows);

	/* allocate a new MC control structure */
	mci = edac_mc_alloc(sizeof(*pvt), num_csrows, num_channels, 0);

	if (mci == NULL) {
		pci_dev_put(pdev16);
		return -ENOMEM;
	}

	kobject_get(&mci->edac_mci_kobj);
	debugf0("MC: " __FILE__ ": %s(): mci = %p\n", __func__, mci);

	mci->dev = &pdev->dev;	/* record ptr to the generic device */

	pvt = mci->pvt_info;
	pvt->dev5400 = pdev;
	pdev = pdev16;
	pvt->system_address = pdev16;	/* Record this device in our private */
	pvt->maxch = num_channels;
	pvt->maxdimmperch = num_dimms_per_channel;

	/* 'get' the pci devices we want to reserve for our use */
	if (i5400_get_devices(mci, dev_idx))
		goto fail0;

#ifdef I5400_STOP_AND_SCREAM
	i5400_stop_and_scream(pvt->dev5400);
#endif /* I5400_STOP_AND_SCREAM */

	/* Time to get serious */
	i5400_get_mc_regs(mci);	/* retrieve the hardware registers */

	mci->mc_idx = 0;
	mci->mtype_cap = MEM_FLAG_RDDR;
	mci->edac_ctl_cap = EDAC_FLAG_SECDED;
	mci->edac_cap = EDAC_FLAG_SECDED;
	mci->mod_name = "i5400_edac.c";
	mci->mod_ver = I5400_REVISION;
	mci->ctl_name = i5400_devs[dev_idx].ctl_name;
	mci->dev_name = pci_name(pdev);
	mci->ctl_page_to_phys = NULL;

	/* Set the function pointer to an actual operation function */
	mci->edac_check = i5400_check_error;

	/* initialize the MC control structure 'csrows' table
	 * with the mapping and control information */
	if (i5400_init_csrows(mci)) {
		debugf0("MC: Setting mci->edac_cap to EDAC_FLAG_NONE\n"
			"    because i5400_init_csrows() returned nonzero "
			"value\n");
		mci->edac_cap = EDAC_FLAG_NONE;	/* no csrows found */
	} else {
		debugf1("MC: Enable error reporting now\n");
		i5400_enable_error_reporting(mci);
	}

	/* add this new MC control structure to EDAC's list of MCs */
	if (edac_mc_add_mc(mci)) {
		debugf0("MC: " __FILE__
			": %s(): failed edac_mc_add_mc()\n", __func__);
		/* FIXME: perhaps some code should go here that disables error
		 * reporting if we just enabled it
		 */
		goto fail1;
	}

	i5400_clear_error(mci);

	/* allocating generic PCI control info */
	i5400_pci = edac_pci_create_generic_ctl(&pdev->dev, EDAC_MOD_STR);
	if (!i5400_pci) {
		printk(KERN_WARNING
			"%s(): Unable to create PCI control\n",
			__func__);
		printk(KERN_WARNING
			"%s(): PCI error report via EDAC not setup\n",
			__func__);
	}

	return 0;

	/* Error exit unwinding stack */
fail1:
	i5400_put_devices(mci);

fail0:
	kobject_put(&mci->edac_mci_kobj);
	edac_mc_free(mci);
	return -ENODEV;
}

/*
 *	i5400_init_one	constructor for one instance of device
 *
 * 	returns:
 *		negative on error
 *		count (>= 0)
 */
static int __devinit i5400_init_one(struct pci_dev *pdev,
				const struct pci_device_id *id)
{
	int rc;

	debugf0("MC: " __FILE__ ": %s()\n", __func__);

	/* wake up device */
	rc = pci_enable_device(pdev);
	if (rc == -EIO)
		return rc;

	/* now probe and enable the device */
	return i5400_probe1(pdev, id->driver_data);
}

/*
 *	i5400_remove_one	destructor for one instance of device
 *
 */
static void __devexit i5400_remove_one(struct pci_dev *pdev)
{
	struct mem_ctl_info *mci;

	debugf0(__FILE__ ": %s()\n", __func__);

	if (i5400_pci)
		edac_pci_release_generic_ctl(i5400_pci);

	if ((mci = edac_mc_del_mc(&pdev->dev)) == NULL)
		return;

	/* retrieve references to resources, and free those resources */
	i5400_put_devices(mci);
	kobject_put(&mci->edac_mci_kobj);
	edac_mc_free(mci);
}

/*
 *	pci_device_id	table for which devices we are looking for
 *
 *	The "5400A" device is the first device supported.
 */
static const struct pci_device_id i5400_pci_tbl[] __devinitdata = {
	{PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I5400A_DEV0),
	 .driver_data = I5400A},
	{PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I5400B_DEV0),
	 .driver_data = I5400B},

	{0,}			/* 0 terminated list. */
};

MODULE_DEVICE_TABLE(pci, i5400_pci_tbl);

/*
 *	i5400_driver	pci_driver structure for this module
 *
 */
static struct pci_driver i5400_driver = {
	.name = KBUILD_BASENAME,
	.probe = i5400_init_one,
	.remove = __devexit_p(i5400_remove_one),
	.id_table = i5400_pci_tbl,
};

/*
 *	i5400_init		Module entry function
 *			Try to initialize this module for its devices
 */
static int __init i5400_init(void)
{
	int pci_rc;

	debugf2("MC: " __FILE__ ": %s()\n", __func__);

	pci_rc = pci_register_driver(&i5400_driver);

	return (pci_rc < 0) ? pci_rc : 0;
}

/*
 *	i5400_exit()	Module exit function
 *			Unregister the driver
 */
static void __exit i5400_exit(void)
{
	debugf2("MC: " __FILE__ ": %s()\n", __func__);
	pci_unregister_driver(&i5400_driver);
}

module_init(i5400_init);
module_exit(i5400_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR
    ("Linux Networx (http://lnxi.com) Doug Thompson <norsk5@xmission.com>");
MODULE_DESCRIPTION("MC Driver for Intel I5400 memory controllers - "
		I5400_REVISION);
