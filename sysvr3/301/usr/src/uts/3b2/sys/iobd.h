/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/iobd.h	10.4"
/*
 *	Board Slots
 */
#define	IO1		0x00200000L
#define	IO2		0x00400000L
#define	IO3		0x00600000L
#define	IO4		0x00800000L
#define	IO5		0x00a00000L
#define	IO6		0x00c00000L
#define	IO7		0x00e00000L
#define	IO8		0x01000000L
#define	IO9		0x01200000L
#define	IO10		0x01400000L
#define	IO11		0x01600000L
#define	IO12		0x01800000L
#define	IO13		0x01a00000L
#define	IO14		0x01c00000L
#define	IO15		0x01e00000L

# define IO_DUMB(x)	((struct io_dumb *)(IO1*x))
# define IO_SMART(x)	((struct io_smart *)(IO1*x))

struct	io_dumb
{
	unsigned char id0;
	unsigned char id1;
	unsigned char pad1;
	unsigned char 	ctrl ;
	unsigned char pad2;
	unsigned char 	stat ;
	unsigned char	vector ;
} ;

struct	io_smart
{
	unsigned char id0;
	unsigned char id1;
	unsigned char pad1;
	unsigned char 	level1 ;
	unsigned char pad2;
	unsigned char 	reset ;
} ;

#define	PKIO	0x200000	/* Physical address of first I/O slot.	*/
#define	VKIO	0x60000		/* Virtual address to which kernel maps	*/
				/* the first I/O slot.			*/
#define	KIOSZ	0x400		/* Size of each I/O slot memory area	*/
				/* in pages.				*/
#define	VKIOSZ	0x440		/* Size of kernel virtual window for	*/
				/* each I/O board.			*/
