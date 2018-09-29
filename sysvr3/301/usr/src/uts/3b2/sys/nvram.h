/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/nvram.h	10.4"
/*
 *	Non-Volatile RAM
 */
#define	ONVRAM		0x00043000L
#define	NVRSIZ		0x400L

#define PASS 1		/* read or write of nvram had no checksum error */
#define FAIL 0

/* firmware section (256 nibbles) */

#define NVRBNAM 45

struct fw_nvr {
	char passwd[9];			/* interactive mcp password */
	unsigned char cons_def;		/* console slot and port #'s */
	unsigned short link;		/* download link baud rate */
	unsigned char b_dev;		/* default boot device */
	char b_name[NVRBNAM];		/* default boot path name */
	char dsk_chk;			/* flag to check for second disk */
};

#define FW_OFSET 0
#define FW_NVR ((struct fw_nvr *)(ONVRAM + FW_OFSET))

/* UN*X section (256 nibbles) */

struct unx_nvr {
	unsigned short consflg;
	unsigned char nv_month ;
	unsigned char nv_year ;
	int spmem;
	char sys_name[9];
	char rootdev;
	unsigned long ioslotinfo[12] ;
};

#define UNX_OFSET 0x80
#define UNX_NVR ((struct unx_nvr *)(ONVRAM + UNX_OFSET))

/* checksum section (4 nibbles) */

#define CHKS_OFSET 0x100
#define CHKS_NVR (ONVRAM + CHKS_OFSET)

/* extra section (508 nibbles, no chksum calc */

# define NERRLOG	4
# define NVSANITY	0x372e245f

#ifndef fw3b2			/* since types.h is not included in firmware */
struct xtra_nvr
{
	int	nvsanity ;
	struct	systate
	{
		short	csr ;
		psw_t	psw ;
		int	r3 ;
		int	r4 ;
		int	r5 ;
		int	r6 ;
		int	r7 ;
		int	r8 ;
		int	oap ;
		int	opc ;
		int	osp ;
		int	ofp ;
		int	isp ;
		int	pcbp ;
		FLTCR	mmufltcr ;
		int	mmufltar ;
		SRAMA	mmusrama[4] ;
		SRAMB	mmusramb[4] ;
		int	lfp ;
	} systate ;
	struct	errlog
	{
		char	*message ;
		int	param1 ;
		int	param2 ;
		int	param3 ;
		int	param4 ;
		int	param5 ;
		time_t	time ;
	} errlog[NERRLOG] ;
};
#endif

#define XTRA_OFSET (CHKS_OFSET + 2)
#define XTRA_NVR ((struct xtra_nvr *)(ONVRAM + XTRA_OFSET))

/* for fw chnvram routine only! */
#define NVR_RD 0	/* only read checksum, don't recalculate it */
#define NVR_WR 1	/* recalculate checksum and update it */

/* extended firmware error message information */

struct fwerr_nvr {
	unsigned long gooderror;	/* flag that info is good */
	unsigned long errno;		/* errno accumulated till printed */
	unsigned long psw;		/* pswstore when flt/int occurred */
	unsigned long pc;		/* pcstore when flt/int occurred */
	unsigned long misc;		/* info specific to error type */
};

#define FWERR_OFSET ((NVRSIZ / 2) - sizeof(struct fwerr_nvr))
#define FWERR_NVR ((struct fwerr_nvr *)(ONVRAM + FWERR_OFSET))


