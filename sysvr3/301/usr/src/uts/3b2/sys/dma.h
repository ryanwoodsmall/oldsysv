/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/dma.h	10.1"
/*
 *	9517 DMA Controller
 */

struct	dma	{
	unsigned char	C0CA;		/* channel 0 current address reg */
	unsigned char	C0WC;		/* channel 0 word current count reg */
	unsigned char	C1CA;		/* channel 1 current address reg */
	unsigned char	C1WC;		/* channel 1 word current count reg */
	unsigned char	C2CA;		/* channel 2 current address reg */
	unsigned char	C2WC;		/* channel 2 word current count reg */
	unsigned char	C3CA;		/* channel 3 current address reg */
	unsigned char	C3WC;		/* channel 3 word current count reg */
	unsigned char	RSR_CMD;	/* read status - write CMD reg */
	unsigned char	WRR;		/* write request reg */
	unsigned char	WMKR;		/* write channel mask reg */
	unsigned char	WMODR;		/* write mode reg */
	unsigned char	CBPFF;		/* clear byte pointer flip/flop */
	unsigned char	RTR_WMC;	/* read temp reg */
	unsigned char	pad;		/* N/A */
	unsigned char	WAMKR;		/* write all mask reg */
	unsigned char	CLR_INT;	/* location read to clear DMAC intr */
};

extern struct dma	dmac;		/* The real thing.	*/

#define	OIDMAC		0x00048000L
#define	IDMAC		((struct dma *)OIDMAC)

#define DMNDMOD	0x00	/* demand transfer mode used by idisk */
#define SNGLMOD	0x40	/* single transfer mode used by iflop, iuartA, iuartB */
#define	RDMA	0x8	/* read DMA command */
#define	WDMA	0x4	/* write DMA command */
#define	RSTDMA	0x0	/* reset DMA command */

#define CH0IHD	0x00	/* channel 0 integral hard disk */
#define CH1IFL	0x01	/* channel 1 integral floppy disk */
#define CH2IUA	0x02	/* channel 2 integral uart A */
#define CH3IUB	0x03	/* channel 3 integral uart B */

/*
 *	DMA Page Registers
 */

/*	The external symbols are defined in the vuifile.
*/

#define	OIDMAPHD	0x00045000L	/* hard disk */
#define	IDMAPHD		(((struct r8 *)OIDMAPHD)->data)
extern int		dmaid;

#define	OIDMAPUA	0x00046000L	/* uart A */
#define	IDMAPUA		(((struct r8 *)OIDMAPUA)->data)
extern int		dmaiuA;

#define	OIDMAPUB	0x00047000L	/* uart B */
#define	IDMAPUB		(((struct r8 *)OIDMAPUB)->data)
extern int		dmaiuB;

#define	OIDMAPFD	0x0004E000L	/* floppy disk */
#define	IDMAPFD		(((struct r8 *)OIDMAPFD)->data)
extern int		dmaif;

#define RPAGE		0x80
#define WPAGE		0x00

#define MSK64K 0xffff	/* 64K byte address mask - used by disk drivers */
#define BND64K 0x10000
