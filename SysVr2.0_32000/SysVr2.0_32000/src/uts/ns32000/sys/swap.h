/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* @(#)swap.h	1.1 */
/*	The following structure contains the data describing a
 *	swap file.
 */

typedef struct swaptab {
	dev_t	st_dev;		/* The swap device.		*/
	short	st_flags;	/* Flags defined below.		*/
	use_t	*st_ucnt;	/* Ptr to use count array for	*/
				/* pages on swap.		*/
	use_t	*st_last;
	int	st_swplo;	/* First block number on device	*/
				/* to be used for swapping.	*/
	int	st_npgs;	/* Number of pages of swap	*/
				/* space on device.		*/
	int	st_nfpgs;	/* Nbr of free pages on device.	*/
} swpt_t;

#define	ST_INDEL	0x01	/* This file is in the process 	*/
				/* of being deleted.  Don't	*/
				/* allocate from it.		*/


extern swpt_t	swaptab[];	/* The table of swap files.	*/
extern int	nextswap;;	/* Index into swptab to the	*/
				/* next file to  allocate from.	*/
extern int	swapwant;	/* Set non-zero if someone is	*/
				/* waiting for swap space.	*/

#define	MSFILES	16		/* The maximum number of swap	*/
				/* files which can be allocated.*/
				/* It is limited by the size of	*/
				/* the dbd_swpi field in the	*/
				/* dbd_t structure.		*/


/*	The following struct is used by the sys3b system call.
 *	If the first argument to the sys3b system call is 3,
 *	then the call pertains to the swap file.  In this case,
 *	the second argument is a pointer to a structure of the
 *	following format which contains the parameters for the
 *	operation to be performed.
 */

typedef struct swapint {
	char	si_cmd;		/* One of the command codes	*/
				/* listed below.		*/
	char	*si_buf;	/* For an SI_LIST function, this*/
				/* is a pointer to a buffer of	*/
				/* sizeof(swpt_t)*MSFILES bytes.*/
				/* For the other cases, it is a	*/
				/* pointer to a pathname of a	*/
				/* swap file.			*/
	int	si_swplo;	/* The first block number of the*/
				/* swap file.  Used only for	*/
				/* SI_ADD and SI_DEL.		*/
	int	si_nblks;	/* The size of the swap file in	*/
				/* blocks.  Used only for an	*/
				/* SI_ADD request.		*/
} swpi_t;

/*	The following are the possible values for si_cmd.
 */

#define	SI_LIST		0	/* List the currently active	*/
				/* swap files.			*/
#define	SI_ADD		1	/* Add a new swap file.		*/
#define	SI_DEL		2	/* Delete one of the currently	*/
				/* active swap files.		*/
